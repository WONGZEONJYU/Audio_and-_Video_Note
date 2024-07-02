//
// Created by Administrator on 2024/6/27.
//

#include "FFPlay.hpp"
#include "ff_ffmsg.h"
#include <iostream>

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
    //fprintf(stderr,"decode_interrupt_cb interval: %lldms",cur_time-s_pre_time);
    cerr << "decode_interrupt_cb interval:\t" << (cur_time - s_pre_time) << "\n";
    s_pre_time = cur_time;
    auto this_{static_cast<FFPlay*>(_this)};
    return this_->m_abort_request;
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

    if (m_read_th.joinable()){ //等待读线程退出
        m_read_th.join();
    }

    if (m_video_refresh_th.joinable()){ //等待视频刷新线程退出
        m_video_refresh_th.join();
    }

//    if (m_decode_audio_th.joinable()){
//        m_decode_audio_th.join();
//    }

    if (m_video_stream >= 0){ //关闭视频流
        stream_component_close(m_video_stream);
    }

    if (m_audio_stream >= 0){ //关闭音频流
        stream_component_close(m_audio_stream);
    }

    //关闭解复用器avformat_close_input(...)
    avformat_close_input(&m_ic);

    //frame队列
    frame_queue_destroy(&m_pictq);
    frame_queue_destroy(&m_sampq);
    //释放packet队列
    packet_queue_destroy(&m_videoq);
    packet_queue_destroy(&m_audioq);
}

void FFPlay::stream_component_open(const int &index) noexcept(false) {

    AVCodecContext *avctx{};
    const AVCodec *codec{};
    int sample_rate{},nb_channels{};
    AVChannelLayout channel_layout{};

}

void FFPlay::stream_component_close(const int &index){

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
    int err{},st_index[AVMEDIA_TYPE_NB]{};
    std::fill_n(st_index, sizeof(st_index),-1);

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

//        mq_msg_put(FFP_MSG_PREPARED);
//        std::cerr << __FUNCTION__ << "\tFFP_MSG_PREPARED\n";

        while (!m_abort_request){
            sleep_for(100ms);
        }

    } catch (const std::exception &e) {
        pkt.reset();
        //avformat_close_input(&m_ic);
        mq_msg_put(FFP_MSG_ERROR); //线程内出错,通知UI和IJKMediaPlay处理事件
        cerr << e.what() << "\n";
    }

    cerr << __FUNCTION__  << "\tend\n" <<flush;
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

