//
// Created by Administrator on 2024/6/27.
//

#include "FFPlay.hpp"
#include "ff_ffmsg.h"
#include <iostream>

extern "C"{
#include <libavutil/log.h>
}

#include <SDL.h>

using namespace std;
using namespace chrono;
using namespace this_thread;

void FFPlay::prepare_async(const std::string &url) noexcept(false){
    if (url.empty()){
        throw std::runtime_error("FFPlay::prepare_async url empty\n");
    }
    m_url = url;
    //无需初始化队列,队列在构造的时候已经构造完成
    stream_open();
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
    frame_queue_init(&pictq,&videoq,VIDEO_PICTURE_QUEUE_SIZE,1);

    //初始化packet队列

    //初始化时钟
    //初始化音量
    //创建解复用器读线程

    m_read_th = std::move(std::thread(&FFPlay::read_thread,this));

    //创建视频刷新线程

    //处理失败调用stream_close()
}

void FFPlay::stream_close()
{
    abort_request = true;

    //关闭解复用器avformat_close_input(...)
    //释放packet队列
    //frame队列
}

void FFPlay::stream_component_open(const int &index){

}

void FFPlay::stream_component_close(const int &index){

}

void FFPlay::f_start() {
    cerr << __FUNCTION__ << "\n";
}

void FFPlay::f_stop(){
    cerr << __FUNCTION__ << "\t";
    abort_request = true;
    mq_abort();
}

FFPlay::~FFPlay(){

    cerr << __FUNCTION__ << "\n";

    if (m_read_th.joinable()){
        m_read_th.join();
    }

    if (m_video_refresh_th.joinable()){
        m_video_refresh_th.join();
    }

    if (m_audio_th.joinable()){
        m_audio_th.join();
    }
}

void FFPlay::read_thread() {

    cerr << __FUNCTION__ << "\tbegin\n";

    sleep_for(10ms);

    mq_msg_put(FFP_MSG_OPEN_INPUT);
    std::cerr << __FUNCTION__ << "\tFFP_MSG_OPEN_INPUT\n";
    mq_msg_put(FFP_MSG_FIND_STREAM_INFO);
    std::cerr << __FUNCTION__ << "\tFFP_MSG_FIND_STREAM_INFO\n";
    mq_msg_put(FFP_MSG_COMPONENT_OPEN);
    std::cerr << __FUNCTION__ << "\tFFP_MSG_COMPONENT_OPEN\n";
    mq_msg_put(FFP_MSG_PREPARED);
    std::cerr << __FUNCTION__ << "\tFFP_MSG_PREPARED\n";

    while (true){

        if (abort_request){
            break;
        }

        sleep_for(10ms);
    }
    cerr << __FUNCTION__  << "\tend\n" <<flush;
}

void FFPlay::video_refresh_thread() {

}

FFPlay_sptr new_FFPlay() noexcept(false) {
    FFPlay_sptr obj;
    try {
        obj.reset(new FFPlay());
        return obj;
    } catch (const std::runtime_error &e ) {
        obj.reset();
        throw e;
    } catch (const std::bad_alloc &e) {
        obj.reset();
        throw e;
    }
}

