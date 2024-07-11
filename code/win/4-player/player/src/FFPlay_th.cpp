#include "FFPlay.hpp"
#include "ff_ffmsg.h"
#include <iostream>
#include <algorithm>
#include "ShareAVPacket.hpp"
#include "AVHelper.h"

using namespace std;
using namespace chrono;
using namespace this_thread;

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

        while (!m_abort_request){
            auto ret{av_read_frame(m_ic, *pkt)}; //不会释放pkt的数据,需要我们自己释放packet的数据
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
            } else if (m_video_stream == (*pkt)->stream_index){
                packet_queue_put(&m_videoq,*pkt);
            }else{
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

    try {

        double remaining_time {};
        while (!m_abort_request) {

            if (remaining_time > 0.0){ //sleep控制画面输出的时机
                //av_usleep((int64_t)(remaining_time * 1000000.0)); // remaining_time <= REFRESH_RATE
                sleep_for(microseconds(static_cast<int64_t>(remaining_time * 1000000.0)));
            }

            remaining_time = REFRESH_RATE;
            video_refresh(remaining_time);
        }

    } catch (const std::exception &e) {
        cerr << e.what() << "\n";
        mq_msg_put(FFP_MSG_ERROR); //线程内出错,通知UI和IJKMediaPlay处理事件
    }

    cerr << __FUNCTION__  << "\tend\n" <<flush;
}
