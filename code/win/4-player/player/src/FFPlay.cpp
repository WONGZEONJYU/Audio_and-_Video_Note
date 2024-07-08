//
// Created by Administrator on 2024/6/27.
//

#include "FFPlay.hpp"
#include "ff_ffmsg.h"
#include <iostream>
#include <algorithm>

#include <SDL.h>
#include "ShareAVPacket.hpp"
#include "AVHelper.h"

using namespace std;
using namespace chrono;
using namespace this_thread;

int FFPlay::decode_interrupt_cb(void *_this)
{
    static int64_t s_pre_time {};
    const auto cur_time  {av_gettime_relative() / 1000};
    //cerr << "decode_interrupt_cb interval:\t" << (cur_time - s_pre_time) << "\n";
    s_pre_time = cur_time;
    auto this_{static_cast<FFPlay*>(_this)};
    return this_->m_abort_request;
}

void FFPlay::sdl_audio_callback(void *opaque, Uint8 *stream, int len)
{
    auto this_{static_cast<FFPlay*>(opaque)};

    while (len > 0){
        if (this_->m_audio_buf_index >= this_->m_audio_buf_size) { //此处代表上一次剩余数据已经被拷贝完毕,需重获取新数据
            const auto audio_size {this_->audio_decode_frame()};
            if (audio_size < 0){
                this_->m_audio_buf = nullptr;
                this_->m_audio_buf_size = SDL_AUDIO_MIN_BUFFER_SIZE / this_->m_audio_tgt.frame_size * this_->m_audio_tgt.frame_size;

            } else{
                this_->m_audio_buf_size = audio_size; /*读到多少字节数据*/
            }

            this_->m_audio_buf_index = 0;
        }

        auto len1{this_->m_audio_buf_size - this_->m_audio_buf_index};

        if (len1 > len){ //超出缓冲区给的长度,先拷贝缓冲区给出的长度
            len1 = len;
        }

        if (this_->m_audio_buf){
            std::copy_n(this_->m_audio_buf + this_->m_audio_buf_index,len1,stream);
        }

        len -= static_cast<int>(len1); //读取了多少数据,len要减去相应的长度
        stream += len1; //stream也需偏移
        /* 更新m_audio_buf_index,指向audio_buf中未被拷贝到stream的数据(剩余数据)的起始位置 */
        this_->m_audio_buf_index += static_cast<int>(len1);
    }
}

void FFPlay::prepare_async(const std::string &url) noexcept(false){
    if (url.empty()){
        throw std::runtime_error("FFPlay::prepare_async url empty\n");
    }
    m_url = url;
    //无需初始化消息队列,消息队列在构造的时候已经构造完成
    stream_open();
}

void FFPlay::construct() noexcept(false)
{

}

void FFPlay::stream_open() noexcept(false){

    //初始化SDL
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER) < 0) {
        const auto errmsg{"Could not initialize SDL - " + string (SDL_GetError())};
        const string errmsg1{"(Did you set the DISPLAY variable?)"};
        av_log(nullptr,AV_LOG_FATAL,"%s",errmsg.c_str());
        av_log(nullptr,AV_LOG_FATAL,"%s",errmsg1.c_str());
        throw std::runtime_error(errmsg + errmsg1 + "\n");
    }

    //初始化Frame队列
    if (frame_queue_init(&m_pictq,&m_videoq,VIDEO_PICTURE_QUEUE_SIZE,1) < 0){
        cerr << "video frame queue init failed\n";
        throw std::runtime_error("video frame queue init failed");
    }

    if (frame_queue_init(&m_sampq, &m_audioq, SAMPLE_QUEUE_SIZE, 1) < 0){
        cerr << "audio frame queue init failed\n";
        throw std::runtime_error("audio frame queue init failed");
    }

    //初始化packet队列
    if (packet_queue_init(&m_videoq) < 0){
        const string errmsg("packet_queue_init(&videoq)\n");
        cerr << errmsg;
        throw std::runtime_error(errmsg);
    }

    if (packet_queue_init(&m_audioq) < 0){
        const string errmsg("packet_queue_init(&audioq)\n");
        cerr << errmsg;
        throw std::runtime_error(errmsg);
    }

    //初始化时钟
    init_clock(&m_vidclk, &m_videoq.serial);
    init_clock(&m_audclk, &m_audioq.serial);
    init_clock(&m_extclk, &m_extclk.serial);

    //初始化音量
    m_startup_volume = av_clip_c(m_startup_volume,0,100);
    m_startup_volume = av_clip_c(SDL_MIX_MAXVOLUME * m_startup_volume / 100,0,SDL_MIX_MAXVOLUME);
    m_audio_volume = m_startup_volume;

    //创建解复用器读线程
    m_read_th = std::move(std::thread(&FFPlay::read_thread,this));

    //创建视频刷新线程
    m_video_refresh_th = std::move(std::thread(&FFPlay::video_refresh_thread, this));

    //处理失败调用stream_close(),此处通过抛异常处理
}

void FFPlay::stream_close() noexcept(true)
{
    m_abort_request = true;

    if (m_video_refresh_th.joinable()){ //等待视频刷新线程退出
        m_video_refresh_th.join();
    }

    if (m_read_th.joinable()){ //等待读线程退出
        m_read_th.join();
    }

    if (m_video_stream >= 0){ //关闭视频流
        stream_component_close(m_video_stream);
    }

    if (m_audio_stream >= 0){ //关闭音频流
        stream_component_close(m_audio_stream);
    }

    SDL_CloseAudio();

    //关闭解复用器avformat_close_input(...)
    avformat_close_input(&m_ic);

    //frame队列
    frame_queue_destroy(&m_pictq);
    frame_queue_destroy(&m_sampq);
    //释放packet队列
    packet_queue_destroy(&m_videoq);
    packet_queue_destroy(&m_audioq);
}

void FFPlay::stream_component_open(const int &stream_index) noexcept(false) {

    cerr << "begin\t" <<__FUNCTION__ << "\tstream_index:\t" << stream_index << "\n";

    AVCodecContext *av_codec_ctx{};
    const AVCodec *codec{};
    int sample_rate{},nb_channels{};
    AVChannelLayout channel_layout{};
    int err{};

    try {

        if (stream_index < 0 || stream_index >= m_ic->nb_streams){
            cerr << __FUNCTION__ << "\twaring stream_index error\n";
            return;
        }

        if (!(av_codec_ctx = avcodec_alloc_context3(nullptr))){
            //分配解码器上下文
            throw std::runtime_error("avcodec_alloc_context3 failed");
        }

        if ((err = avcodec_parameters_to_context(av_codec_ctx,m_ic->streams[stream_index]->codecpar)) < 0){
            //拷贝流的编解码器器信息参数到解码器里
            throw std::runtime_error("avcodec_parameters_to_context failed:\t" +
                                    string(AVHelper::av_get_err(err)));
        }

        //设置解码器时间基准,用流的基准作为时间基准
        av_codec_ctx->time_base = m_ic->streams[stream_index]->time_base;

        if(!(codec = avcodec_find_decoder(av_codec_ctx->codec_id))){
            //通过id查找解码器
            throw std::runtime_error("no decoder could be found for codec:\t" +
                                    string(avcodec_get_name(av_codec_ctx->codec_id)));
        }

        if ((err = avcodec_open2(av_codec_ctx,codec, nullptr)) < 0){
            throw std::runtime_error("open codec failed:\t" + string(AVHelper::av_get_err(err)));
        }

        switch (av_codec_ctx->codec_type){
            case AVMEDIA_TYPE_AUDIO:
                //从解码器上下文中获取音频格式参数
                sample_rate = av_codec_ctx->sample_rate;
                nb_channels = av_codec_ctx->ch_layout.nb_channels;
                channel_layout = av_codec_ctx->ch_layout;

                //prepare audio output 准备音频输出
                //调用audio_open打开sdl音频输出,实际打开的设备参数保存在audio_tgt,返回值表示输出设备的缓冲区大小
                m_audio_hw_size = static_cast<int>(audio_open(channel_layout,nb_channels,sample_rate,&m_audio_tgt));
                //暂且将数据源参数等同于目标输出参数
                m_audio_src = m_audio_tgt;

                m_audio_stream = stream_index; //获取音频索引
                m_audio_st = m_ic->streams[stream_index]; //获取音频的stream指针

                //初始化ffplay封装的音频解码器,并将解码器上下文与Decodec绑定
                //decoder_init(...)
                m_a_decoder = new_AudioDecoder(m_cv,m_audioq,m_sampq,*av_codec_ctx);
                //启动音频解码线程
                //decoder_start(...)
                m_a_decoder->av_decoder_start(this);

                //允许音频输出
                SDL_PauseAudio(0);
                break;
            case AVMEDIA_TYPE_VIDEO:
                m_video_stream = stream_index;
                m_video_st = m_ic->streams[stream_index];
                //初始化ffplay封装的视频解码器
                //decode_init(...)
                m_v_decoder = new_VideoDecoder(m_cv,m_videoq,m_pictq,*av_codec_ctx);

                //启动视频解码线程
                //decoder_start(...)
                m_v_decoder->av_decoder_start(this);
                break;
            default:
                break;
        }
    } catch (const std::exception &e) {
        cerr << __FUNCTION__ << "\t" << e.what() << "\n";
        avcodec_free_context(&av_codec_ctx);
        throw e;
    }
    //临时释放avcodec_free_context(&av_codec_ctx),此处用于测试,后续会交给封装的解码器销毁的时候销毁
    //avcodec_free_context(&av_codec_ctx);
    cerr << "end\t" <<__FUNCTION__ << "\tstream_index:\t" << stream_index << "\n";
}

void FFPlay::stream_component_close(const int &stream_index){

    cerr << "begin\t" <<__FUNCTION__ << "\tstream_index:\t" << stream_index << "\n";

    if (stream_index < 0 || stream_index >= m_ic->nb_streams){
        return;
    }

    const auto codec_par{m_ic->streams[stream_index]->codecpar};
    switch (codec_par->codec_type) {
        case AVMEDIA_TYPE_AUDIO:
            cerr << __FUNCTION__ << "\tAVMEDIA_TYPE_AUDIO\n";
            //请求终止解码器线程
            m_a_decoder->av_decoder_abort();
            //关闭音频设备
            //销毁解码器
            //释放从采样器
            //释放audio buf
            //decoder_abort(...)
            //SDL_CloseAudioDevice(...)
            //decoder_destroy(...)
            //swr_free(...)
            //av_freep(...)
            av_freep(m_audio_buf1);
            //audio_buf1_size = 0
            //audio_buf = nullptr
            m_audio_stream = -1;
            m_audio_st = {};

            break;
        case AVMEDIA_TYPE_VIDEO:
            cerr << __FUNCTION__ << "\tAVMEDIA_TYPE_VIDEO\n";
            //decoder_abort(...)
            m_v_decoder->av_decoder_abort();
            //decoder_destroy(...)
            //
            m_video_stream = -1;
            m_video_st = {};
            break;
        default:
            break;
    }

   // m_ic->streams[stream_index]->discard = AVDISCARD_ALL;
    cerr << "end\t" <<__FUNCTION__ << "\tstream_index:\t" << stream_index << "\n";
}

uint32_t FFPlay::audio_open(AVChannelLayout wanted_ch_layout,
                        int wanted_nb_channels,
                        int wanted_sample_rate,
                        AudioParams *audio_hw_params) noexcept(false) {

    SDL_AudioSpec wanted_spec{
        .freq = wanted_sample_rate,
        .format = AUDIO_S16,
        .channels = static_cast<uint8_t>(wanted_nb_channels),
        .silence = 0,
        .samples = 2048,
        .callback = sdl_audio_callback,
        .userdata = this,
    };

    if(SDL_OpenAudio(&wanted_spec, nullptr)) {
        const auto errmsg(string("Failed to open audio device , ") + SDL_GetError());
        cerr << errmsg << "\n";
        throw std::runtime_error(errmsg);
    }

    audio_hw_params->fmt  = AV_SAMPLE_FMT_S16;
    audio_hw_params->freq = wanted_sample_rate;
    audio_hw_params->ch_layout = wanted_ch_layout;
    audio_hw_params->channels = wanted_ch_layout.nb_channels;
    /*计算一个采样点占用多少个字节*/
    audio_hw_params->frame_size = av_samples_get_buffer_size(nullptr,
                                                             audio_hw_params->channels,
                                                             1,
                                                             audio_hw_params->fmt,
                                                             1);
    /*通过采样法计算每一秒需要多少个字节去填充*/
    audio_hw_params->bytes_per_sec = av_samples_get_buffer_size(nullptr,
                                                                audio_hw_params->channels,
                                                                audio_hw_params->freq,
                                                                audio_hw_params->fmt,
                                                                1);

    if (audio_hw_params->frame_size <= 0 || audio_hw_params->bytes_per_sec <= 0){
        const auto errmsg(string("av_samples_get_buffer_size failed"));
        throw std::runtime_error(errmsg);
    }

    return wanted_spec.size;
}

//void FFPlay::audio_close() {
//    SDL_CloseAudio();
//}

int FFPlay::audio_decode_frame() noexcept(true) {

    auto af{frame_queue_peek_readable(&m_sampq)};
    if (!af) {
        return -1;
    }

    const auto data_size{av_samples_get_buffer_size(nullptr, af->frame->ch_layout.nb_channels,
                                                    af->frame->nb_samples,
                                                    static_cast<AVSampleFormat>(af->frame->format), 1)};

    const auto wanted_nb_samples{af->frame->nb_samples};

    //通道布局 采样格式 采样率不同 则进行重采样
    //为什么是与m_audio_src而不是与m_audio_tgt比较,原因如下
    // m_audio_tgt是SDL可接受的音频帧数,是audio_open()中取得的参数
    // 在stream_component_open函数中有 "m_audio_src = m_audio_tgt"
    // 此处表示:如果frame中的音频参数 == m_audio_src = m_audio_tgt
    // 那音频重采样的过程就免了(因此时is->swr_ctr是NULL)
    // 否则使用frame(源)和is->audio_tgt(目标)中的音频参数来设置is->swr_ctx
    // 并使用frame中的音频参数来赋值is->audio_src

    const auto channel_not_equal{av_channel_layout_compare(&af->frame->ch_layout, &m_audio_src.ch_layout)};
    const auto fmt_not_equal{af->frame->format != m_audio_src.fmt};
    const auto sample_not_equal{af->frame->sample_rate != m_audio_src.freq};

    //第一次判断的是解码后的帧参数与SDL输出设备的参数是否有不同
    //第二次是判断是本次解码的帧参数和上一次解码的帧参数是否有不同

    if (channel_not_equal || fmt_not_equal || sample_not_equal) {

        m_swr_ctx.reset(); //先释放原有重采样器

        try {
            m_swr_ctx = new_SwrContext_t(&m_audio_tgt.ch_layout,
                                         m_audio_tgt.fmt,
                                         m_audio_tgt.freq,
                                         &af->frame->ch_layout,
                                         static_cast<AVSampleFormat>(af->frame->format),
                                         af->frame->sample_rate);

            m_audio_src.ch_layout = af->frame->ch_layout;
            m_audio_src.channels = af->frame->ch_layout.nb_channels;
            m_audio_src.fmt = static_cast<AVSampleFormat>(af->frame->format);
            m_audio_src.freq = af->frame->sample_rate;

        } catch (const exception &e) {
            cerr << e.what() << "\n";
            return -1;
        }
    }

    int resampled_data_size{};
    if (m_swr_ctx) {

        const auto *const *in{af->frame->extended_data};
        auto out{&m_audio_buf1};

        // in_count / in_sample_rate = out_count / out_sample_rate;
        //out_count是样本数量,并非是一帧的大小
        // + 256的目的是重采样内部是有一定的缓存,就存在上一次的重采样缓存数据和这一次重采样一起输出的情况,多出来的目的是为了分配大点的输出buffer
        const auto out_count{wanted_nb_samples * m_audio_tgt.freq / af->frame->sample_rate + 256};
        const auto out_size{av_samples_get_buffer_size(nullptr,
                                                       m_audio_tgt.ch_layout.nb_channels,
                                                       out_count,
                                                       m_audio_tgt.fmt,
                                                       1)};
        if (out_size < 0) {
            std::cerr << "av_samples_get_buffer_size() failed\n";
            return -1;
        }

        av_fast_mallocz(&m_audio_buf1, &m_audio_buf_size1, out_size);

        if (!m_audio_buf1) {
            cerr << "av_fast_mallocz failed: " << AVHelper::av_get_err(AVERROR(ENOMEM)) << "\n";
            return AVERROR(ENOMEM);
        }

        try {
            const auto len2 = m_swr_ctx->convert(out, out_count, in, af->frame->nb_samples);

            m_audio_buf = m_audio_buf1;
            resampled_data_size = av_samples_get_buffer_size(nullptr,
                                                             m_audio_tgt.ch_layout.nb_channels,
                                                             len2,
                                                             m_audio_tgt.fmt,
                                                             1);
            //resampled_data_size = len2 * m_audio_tgt.ch_layout.nb_channels * av_get_bytes_per_sample(m_audio_tgt.fmt);

        } catch (const exception &e) {
            cerr << e.what() << "\n";
            return -1;
        }

    } else { //此处是不需重采样的处理
        m_audio_buf = af->frame->data[0];
        resampled_data_size = data_size;
    }

    frame_queue_next(&m_sampq);

    return resampled_data_size;
}

void FFPlay::f_start() {
    cerr << __FUNCTION__ << "\n";
}

void FFPlay::f_stop(){
    cerr << __FUNCTION__ << "\t";
    m_abort_request = true;
    mq_abort();
}

FFPlay::~FFPlay(){
    cerr << __FUNCTION__ << "\n";
    stream_close();
}

void FFPlay::read_thread() {

    cerr << __FUNCTION__ << "\tbegin\n";

    ShareAVPacket_sp_type pkt;
    int err,st_index[AVMEDIA_TYPE_NB]{};
    std::fill_n(st_index,std::size(st_index),-1);

    try {
        pkt = new_ShareAVPacket(); //分配AVPacket

        if (!(m_ic = avformat_alloc_context())){ //创建avformat上下文
            throw std::runtime_error(string(__FUNCTION__) + "\tCould not allocate context.\n");
        }

        m_ic->interrupt_callback.callback = decode_interrupt_cb; //用于在读取媒体文件阻塞的时候,退出阻塞
        m_ic->interrupt_callback.opaque = this;

        if ((err = avformat_open_input(&m_ic,m_url.c_str(), nullptr, nullptr)) < 0){ //打开avformat
            const auto errmsg(string (__FUNCTION__)+ "\t" + AVHelper::av_get_err(err));
            throw std::runtime_error(errmsg);
        }

        mq_msg_put(FFP_MSG_OPEN_INPUT); //发送媒体文件打开消息
        std::cerr << __FUNCTION__ << "\tFFP_MSG_OPEN_INPUT\n";

        if ((err = avformat_find_stream_info(m_ic, nullptr)) < 0) { //有些流无法在avformat_open_input直接被识别,通过avformat_find_stream_info函数进行读包识别
            const auto errmsg(string(__FUNCTION__ ) + "\t" + AVHelper::av_get_err(err));
            throw std::runtime_error(errmsg);
        }

        mq_msg_put(FFP_MSG_FIND_STREAM_INFO); //发送寻媒体流消息
        std::cerr << __FUNCTION__ << "\tFFP_MSG_COMPONENT_OPEN\n";

        st_index[AVMEDIA_TYPE_AUDIO] = av_find_best_stream(m_ic, AVMEDIA_TYPE_AUDIO,
                                                           st_index[AVMEDIA_TYPE_AUDIO],-1,nullptr,0);
        //读取音频流index

        st_index[AVMEDIA_TYPE_VIDEO] = av_find_best_stream(m_ic, AVMEDIA_TYPE_VIDEO,
                                                           st_index[AVMEDIA_TYPE_VIDEO],-1,nullptr,0);
        //读取视频流index

        if (st_index[AVMEDIA_TYPE_VIDEO] >= 0){ //打开视频流
            stream_component_open(st_index[AVMEDIA_TYPE_VIDEO]);
        }

        if (st_index[AVMEDIA_TYPE_AUDIO] >= 0){ //打开音频流
            stream_component_open(st_index[AVMEDIA_TYPE_AUDIO]);
        }

        mq_msg_put(FFP_MSG_COMPONENT_OPEN); //发送打开媒体流消息

        if (m_video_stream < 0 && m_audio_stream < 0){ //音视频流都打开失败
            throw std::runtime_error("Failed to open file\n");
        }

        mq_msg_put(FFP_MSG_PREPARED);
        std::cerr << __FUNCTION__ << "\tFFP_MSG_PREPARED\n";

        int ret{};
        while (!m_abort_request){
            ret = av_read_frame(m_ic,*pkt); //不会释放pkt的数据,需要我们自己释放packet的数据
            if (ret < 0){ //出错或者读取完毕
                if (AVERROR_EOF == ret || avio_feof(m_ic->pb) && !m_eof){
                    m_eof = true;
                }
                if (m_ic->pb && m_ic->pb->error){ //io异常
                    break;
                }
                sleep_for(10ms); //休眠以下,主要
                continue;
            } else{
                m_eof = false;
            }

            if (m_audio_stream == (*pkt)->stream_index) {
                packet_queue_put(&m_audioq,*pkt); //音频插入队列
            } else{
                av_packet_unref(*pkt); //不入队则直接释放数据
            }

        }

    } catch (const std::exception &e) {
        pkt.reset();
        avformat_close_input(&m_ic);
        mq_msg_put(FFP_MSG_ERROR); //线程内出错,通知UI和IJKMediaPlay处理事件
        cerr << e.what() << "\n";
    }

    cerr << __FUNCTION__  << "\tend\n";
}

void FFPlay::video_refresh_thread() {
    cerr << __FUNCTION__ << "\tbegin\n";
    while (!m_abort_request){
        sleep_for(10ms);
    }
    cerr << __FUNCTION__  << "\tend\n" <<flush;
}

FFPlay_sptr new_FFPlay() noexcept(false) {
    FFPlay_sptr obj;
    try {
        obj.reset(new FFPlay());
        return obj;
    } catch (const std::runtime_error &e ) {
        obj.reset();
        throw std::runtime_error("");
    } catch (const std::bad_alloc &e) {
        obj.reset();
        throw std::runtime_error("new FFPlay error");
    }
}

