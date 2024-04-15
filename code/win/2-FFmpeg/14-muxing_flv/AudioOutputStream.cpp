#include "AudioOutputStream.h"
#include <iostream>
#include "AVHelper.h"

extern "C"{
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
}

AVFrame *AudioOutputStream::alloc_audio_frame(const AVSampleFormat &sample_fmt) noexcept(false)
{
    auto frame{av_frame_alloc()};

    if (!frame){
        throw std::runtime_error( "Error allocating an audio frame\n");
    }

    frame->ch_layout = m_codec_ctx->ch_layout;
    frame->sample_rate = m_codec_ctx->sample_rate;
    frame->format = sample_fmt;
    frame->nb_samples = m_codec_ctx->frame_size;

    const auto ret{av_frame_get_buffer(frame,0)};
    if (ret < 0){
        av_frame_free(&frame);
        throw std::runtime_error( "Error allocating an audio buffer: " + AVHelper::av_get_err(ret) + "\n");
    }

    return frame;
}

void AudioOutputStream::get_one_audio_frame_data() noexcept(true)
{
    const auto nb_channel{m_tmp_frame->ch_layout.nb_channels},
                nb_sample{m_tmp_frame->nb_samples};

    auto data{reinterpret_cast<int16_t*>(m_tmp_frame->data[0])};

    for (int i {}; i < nb_sample; ++i) {
        const auto v{static_cast<int16_t>(sin(m_t) * 10000)};
        for (int j {}; j < nb_channel; ++j) {
            *data++ = v;
        }
        m_t += m_tincr;
        m_tincr += m_tincr2;
    }
}

bool AudioOutputStream::get_one_audio_frame() noexcept(true)
{
    if (av_compare_ts(m_next_pts,m_codec_ctx->time_base,
                      STREAM_DURATION,{1,1}) >= 0){
        return {};
    }

    get_one_audio_frame_data();

    m_tmp_frame->pts = m_next_pts; // 使用samples作为计数 设置pts 0, nb_samples(1024) 2048
    m_next_pts += m_tmp_frame->nb_samples;    // 音频PTS使用采样samples叠加

    return true;
}

void AudioOutputStream::config_codec_params() noexcept(true) {

    m_codec_ctx->codec_id = m_fmt_ctx.oformat->audio_codec;
    m_codec_ctx->sample_fmt  = m_codec->sample_fmts ?    // 采样格式
                               m_codec->sample_fmts[0] : AV_SAMPLE_FMT_FLTP;
    m_codec_ctx->bit_rate    = STREAM_BIT_RATE;        // 码率
    m_codec_ctx->sample_rate = STREAM_SAMPLE_RATE;     // 采样率

    if (m_codec->supported_samplerates){
        m_codec_ctx->sample_rate = m_codec->supported_samplerates[0];
        for (int i {}; m_codec->supported_samplerates[i]; i++){
            if (STREAM_BIT_RATE == m_codec->supported_samplerates[i]){
                m_codec_ctx->sample_rate = STREAM_BIT_RATE;
            }
        }
    }

    m_codec_ctx->ch_layout = AV_CHANNEL_LAYOUT_STEREO;
    if (m_codec->ch_layouts){
        m_codec_ctx->ch_layout = m_codec->ch_layouts[0];
        for (int i {};  m_codec->ch_layouts[i].u.mask; ++i) {
            if (AV_CH_LAYOUT_STEREO == m_codec->ch_layouts[i].u.mask){
                m_codec_ctx->ch_layout = m_codec->ch_layouts[i];
            }
        }
    }

    m_stream->time_base = { 1, m_codec_ctx->sample_rate };
}

void AudioOutputStream::config_PCM_params() noexcept(true) {

    /* init signal generator */
    // 初始化产生PCM的参数
    m_tincr = 2 * M_PI * 110.0 / m_codec_ctx->sample_rate;
    /* increment frequency by 110 Hz per second */
    m_tincr2 = 2 * M_PI * 110.0 / m_codec_ctx->sample_rate / m_codec_ctx->sample_rate;
}

void AudioOutputStream::open_audio() noexcept(false){

    auto ret{avcodec_open2(m_codec_ctx,m_codec, nullptr)};
    if (ret < 0){
        throw std::runtime_error("Could not open audio codec: " + AVHelper::av_get_err(ret) + "\n");
    }

    /*初始化编码器参数*/
    config_PCM_params();

    // signal generator -> PCM -> m_tmp_frame -> swr_convert -> m_frame -> encode
    // 分配送给编码器的帧,并申请对应的buffer
    try {
        m_frame = alloc_audio_frame(m_codec_ctx->sample_fmt);
    }catch (const std::runtime_error &e){
        throw std::runtime_error("m_frame alloc failed: " + std::string(e.what()) + "\n");
    }

    try {
        // 分配送给信号生成PCM的帧, 并申请对应的buffer
        m_tmp_frame = alloc_audio_frame(STREAM_SAMPLE_FMT);
    } catch (const std::runtime_error &e) {
        throw std::runtime_error("m_tmp_frame alloc failed: " + std::string(e.what()) + "\n");
    }

    ret = avcodec_parameters_from_context(m_stream->codecpar, m_codec_ctx);
    if (ret < 0){
        throw std::runtime_error("Could not copy the stream parameters :" +
                                    AVHelper::av_get_err(ret) + "\n");
    }
}

void AudioOutputStream::swr_init() noexcept(false)
{
    m_swr = SwrContext_t::create();
    m_swr->opt_set_ch_layout("in_chlayout",&m_codec_ctx->ch_layout,0);
    m_swr->opt_set_sample_fmt("in_sample_fmt",STREAM_SAMPLE_FMT,0);
    m_swr->opt_set_rate("in_sample_rate",m_codec_ctx->sample_rate,0);
    m_swr->opt_set_ch_layout("out_chlayout",&m_codec_ctx->ch_layout,0);
    m_swr->opt_set_sample_fmt("out_sample_fmt",m_codec_ctx->sample_fmt,0);
    m_swr->opt_set_rate("out_sample_rate",m_codec_ctx->sample_rate,0);
    m_swr->init();
}

void AudioOutputStream::convert() noexcept(false)
{
    const auto delay{m_swr->get_delay(m_codec_ctx->sample_rate)};
    const auto dst_nb_samples { av_rescale_rnd(delay + m_tmp_frame->nb_samples,
                                               m_codec_ctx->sample_rate, m_codec_ctx->sample_rate, AV_ROUND_UP)};

    if (dst_nb_samples != m_tmp_frame->nb_samples){
        throw std::runtime_error("dst_nb_samples != m_tmp_frame->nb_samples\n");
    }

    auto ret{av_frame_make_writable(m_frame)};
    if (ret < 0){
        throw std::runtime_error("av_frame_make_writable failed: " + AVHelper::av_get_err(ret)  + "\n");
    }

    m_swr->convert(m_frame->data,static_cast<int>(dst_nb_samples),
                         const_cast<const uint8_t**>(m_tmp_frame->data),m_tmp_frame->nb_samples);

    // 转换time_base
    m_frame->pts = av_rescale_q(m_samples_count, {1, m_codec_ctx->sample_rate},
                                m_codec_ctx->time_base);
    m_samples_count += dst_nb_samples;
}

bool AudioOutputStream::write_frame() noexcept(false)
{
    try {
        AVPacket pkt{};

        const auto write_media_file{[&]{
            OutputStreamAbstract::write_media_file(pkt);
        }};

        const auto b{get_one_audio_frame()};

        const auto frame{ b ? (convert(),m_frame) : nullptr};

        AVHelper::encode(m_codec_ctx, frame,&pkt,std::move(write_media_file));

        return b;
    } catch (const std::runtime_error &e) {
        throw std::runtime_error("audio write frame error: " + std::string(e.what()) + "\n");
    }
}

void AudioOutputStream::construct() noexcept(false)
{
    /*顺序不能反*/
    add_stream(m_fmt_ctx.oformat->audio_codec);
    open_audio();
    swr_init();
}

AudioOutputStream::AudioOutputStream(AVFormatContext& fmt_ctx) noexcept(true):
        OutputStreamAbstract(fmt_ctx) {}

OutputStreamAbstract::OutputStreamAbstract_sp_type AudioOutputStream::create(AVFormatContext& oc) noexcept(false)
{
    AudioOutputStream_sp_type obj;

    try {
        obj = std::move(AudioOutputStream_sp_type (new AudioOutputStream(oc)));
    } catch (const std::bad_alloc &e) {
        throw std::runtime_error("new AudioOutputStream failed: " + std::string(e.what()));
    }

    try {
        obj->construct();
        return obj;
    } catch (const std::runtime_error &e) {
        obj.reset();
        throw std::runtime_error("new AudioOutputStream failed: " + std::string(e.what()) + "\n");
    }
}
