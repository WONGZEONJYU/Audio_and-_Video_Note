#include "AudioOutputStream.h"
#include <iostream>
#include "AVHelper.h"

extern "C"{
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
}

#if 0
AVFrame *get_audio_frame()
{
    auto frame{m_audio_ost.m_tmp_frame};

    auto q {reinterpret_cast<int16_t*>(frame->data[0])};

    /* check if we want to generate more frames */
    // 44100 * {1, 44100} = 1 --> 44100*5 * {1, 44100} = 5
    // 5 *{1,1} = 5

    if (av_compare_ts(m_audio_ost.m_next_pts, m_audio_ost.m_codecContext->time_base,
                      STREAM_DURATION, { 1, 1 }) >= 0) {
        return nullptr;
    }

    const auto nb_channels{m_audio_ost.m_codecContext->ch_layout.nb_channels};

    for (int j {}; j < frame->nb_samples; j++) {

        const auto v {static_cast<int16_t>(sin(m_audio_ost.m_t) * 10000)};

        for (int i {}; i < nb_channels; i++){
            *q++ = v;
        }

        m_audio_ost.m_t     += m_audio_ost.m_tincr;
        m_audio_ost.m_tincr += m_audio_ost.m_tincr2;
    }

    frame->pts = m_audio_ost.m_next_pts; // 使用samples作为计数 设置pts 0, nb_samples(1024) 2048
    m_audio_ost.m_next_pts += frame->nb_samples;    // 音频PTS使用采样点个数nb_samples叠加
    return frame;
}
#endif

bool AudioOutputStream::construct() noexcept
{
    /*顺序不能反*/
    return add_stream(m_fmt_ctx.oformat->audio_codec) && open_audio() && swr_init();
}

AudioOutputStream::AudioOutputStream(AVFormatContext& fmt_ctx) noexcept:
OutputStreamAbstract(fmt_ctx) {}

AVFrame *AudioOutputStream::alloc_audio_frame(const AVSampleFormat &sample_fmt) noexcept
{
    auto frame{av_frame_alloc()};

    if (!frame){
        std::cerr << "Error allocating an audio frame\n";
        return {};
    }

    frame->ch_layout = m_codec_ctx->ch_layout;
    frame->sample_rate = m_codec_ctx->sample_rate;
    frame->format = sample_fmt;
    frame->nb_samples = m_codec_ctx->frame_size;

    const auto ret{av_frame_get_buffer(frame,0)};
    if (ret < 0){
        std::cerr << "Error allocating an audio buffer: " << AVHelper::av_get_err(ret) << "\n";
        av_frame_free(&frame);
    }

    return frame;
}

void AudioOutputStream::get_one_audio_frame_data()
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

bool AudioOutputStream::get_one_audio_frame() noexcept
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

void AudioOutputStream::config_codec_params() {

    //m_codec_ctx->codec_id = m_fmt_ctx.oformat->audio_codec;
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

void AudioOutputStream::config_PCM_params() {

    /* init signal generator */
    // 初始化产生PCM的参数
    m_tincr = 2 * M_PI * 110.0 / m_codec_ctx->sample_rate;
    /* increment frequency by 110 Hz per second */
    m_tincr2 = 2 * M_PI * 110.0 / m_codec_ctx->sample_rate / m_codec_ctx->sample_rate;
}

bool AudioOutputStream::open_audio() noexcept{

    auto ret{avcodec_open2(m_codec_ctx,m_codec, nullptr)};
    if (ret < 0){
        std::cerr << "Could not open audio codec: " << AVHelper::av_get_err(ret) << "\n";
        return {};
    }

    config_PCM_params();

    // signal generator -> PCM -> m_tmp_frame -> swr_convert -> m_frame -> encode
    // 分配送给编码器的帧, 并申请对应的buffer
    m_frame = alloc_audio_frame(m_codec_ctx->sample_fmt);
    if (!m_frame){
        std::cerr << "m_frame alloc failed\n";
        return {};
    }
    // 分配送给信号生成PCM的帧, 并申请对应的buffer
    m_tmp_frame = alloc_audio_frame(STREAM_SAMPLE_FMT);
    if (!m_tmp_frame){
        std::cerr << "m_tmp_frame alloc failed\n";
        return {};
    }

    ret = avcodec_parameters_from_context(m_stream->codecpar, m_codec_ctx);
    if (ret < 0){
        std::cerr << "Could not copy the stream parameters :" <<
                AVHelper::av_get_err(ret) << "\n";
        return {};
    }

    return true;
}

bool AudioOutputStream::swr_init() noexcept
{
    try {
        m_swr = SwrContext_t::create();

        m_swr->opt_set_chlayout("in_chlayout",&m_codec_ctx->ch_layout,0);
        m_swr->opt_set_sample_fmt("in_sample_fmt",STREAM_SAMPLE_FMT,0);
        m_swr->opt_set_rate("in_sample_rate",m_codec_ctx->sample_rate,0);

        m_swr->opt_set_chlayout("out_chlayout",&m_codec_ctx->ch_layout,0);
        m_swr->opt_set_sample_fmt("out_sample_fmt",m_codec_ctx->sample_fmt,0);
        m_swr->opt_set_rate("out_sample_rate",m_codec_ctx->sample_rate,0);

        return m_swr->init();

    } catch (const std::runtime_error &e) {
        std::cerr << e.what() << "\n";
        return {};
    }
}

std::shared_ptr<OutputStreamAbstract> AudioOutputStream::create(AVFormatContext& oc) noexcept(false)
{
    try {
        AudioOutputStream_sp_type obj(new AudioOutputStream(oc));
        if (!obj->construct()){
            std::cerr << "AudioOutputStream failed\n";
            obj.reset();
        }
        return obj;
    } catch (const std::bad_alloc &e) {
        std::cerr << e.what() << "\n";
        throw std::runtime_error("new AudioOutputStream failed\n");
    }
}

void AudioOutputStream::convert() noexcept(false)
{
    const auto delay{m_swr->get_delay(m_codec_ctx->sample_rate)};
    const auto dst_nb_samples { av_rescale_rnd(delay + m_tmp_frame->nb_samples,
                                               m_codec_ctx->sample_rate, m_codec_ctx->sample_rate, AV_ROUND_UP)};

    if (dst_nb_samples != m_tmp_frame->nb_samples){
        throw std::runtime_error("dst_nb_samples != m_tmp_frame->nb_samples");
    }

    auto ret{av_frame_make_writable(m_frame)};
    if (ret < 0){
        throw std::runtime_error("av_frame_make_writable failed: " + AVHelper::av_get_err(ret)  + "\n");
    }

    ret = m_swr->convert(m_frame->data,static_cast<int>(dst_nb_samples),
                         const_cast<const uint8_t**>(m_tmp_frame->data),m_tmp_frame->nb_samples);

    if (ret < 0){
        throw std::runtime_error("Error while converting\n");
    }

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
        if (b){
            convert();
            AVHelper::encode(m_codec_ctx, m_frame,&pkt,std::move(write_media_file));
        }else{
            AVHelper::encode(m_codec_ctx, nullptr,&pkt,std::move(write_media_file));
        }
        return b;
    } catch (const std::runtime_error &e) {
        throw std::runtime_error(e.what());
    }
}
