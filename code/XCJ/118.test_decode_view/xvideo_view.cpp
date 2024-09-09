//
// Created by Administrator on 2024/8/28.
//

#include "xvideo_view.hpp"
#include "xsdl.hpp"
#include "XAVFrame.hpp"
#include <thread>

using namespace std;

XVideoView *XVideoView::create(const XVideoView::RenderType &renderType) {
    switch (renderType) {
        case SDL:
            return new XSDL();
        default:
            return {};
    }
}

void XVideoView::calc_fps() {
    static constexpr auto TIME_MS{1000LL};
    ++m_count;
    if (m_begin_time <= 0){
        m_begin_time = Get_time_ms(); //更新时间
    } else if (Get_time_ms() - m_begin_time >= TIME_MS){
        m_render_fps = m_count.load();
        m_count = {};
        m_begin_time = Get_time_ms(); //更新时间
    } else{}
}

bool XVideoView::DrawFrame(const XAVFrame_sptr &frame) {

    const auto b {!frame || !frame->data[0] || !frame->width || !frame->height};
    if (b) {
        PRINT_ERR_TIPS(GET_STR(Non-video frames));
        return {};
    }

    calc_fps();

    switch (frame->format) {
        case AV_PIX_FMT_YUV420P:
            return Draw(frame->data[0],frame->linesize[0],
                        frame->data[1],frame->linesize[1],
                        frame->data[2],frame->linesize[2]);
        case AV_PIX_FMT_BGRA:
        case AV_PIX_FMT_RGBA:
        case AV_PIX_FMT_ARGB:
        case AV_PIX_FMT_RGB24:
            return Draw(frame->data[0],frame->linesize[0]);
        default:
            PRINT_ERR_TIPS(GET_STR(The current pixel format is not supported));
            return {};
    }
}

void XVideoView::MSleep(const uint64_t &ms) {
    const auto begin{Get_time_ms()};
    auto ms_{ms};
    while (ms_--) {
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
        const auto now{Get_time_ms()};
        if (now - begin >= ms){
            return;
        }
    }
}

int64_t XVideoView::Get_time_ms(){
    const auto now_{std::chrono::high_resolution_clock::now()};
    const auto now_ms{std::chrono::time_point_cast<std::chrono::milliseconds>(now_)};
    return now_ms.time_since_epoch().count();
}

bool XVideoView::Open(const std::string &file_path) {

    if (m_ifs.is_open()) {
        m_ifs.close();
    }

    m_ifs.open(file_path,ios::binary);
    if (!m_ifs) {
        PRINT_ERR_TIPS(GET_STR(open file error!));
        return {};
    }

    return true;
}

XAVFrame_sptr XVideoView::Read() {

    auto b {m_width <= 0 || m_height <= 0 || m_fmt < 0 || !m_ifs};

    if (b) { //参数打开失败
        //PRINT_ERR_TIPS(GET_STR(m_width <= 0 || m_height <= 0 || m_fmt < 0));
        return {};
    }

    if (m_ifs.eof()) { //文件是否到达结尾
        PRINT_ERR_TIPS(GET_STR(file at end!));
        return {};
    }

    //AVFrame空间已经申请,如果参数发生变化,需先释放空间
    if (m_frame) {
        b = m_frame->width != m_width || m_frame->height != m_height || m_frame->format != m_fmt;
        if (b) { //判断格式是否发生变化
            //释放AVFrame对象空间和buf引用计数减一
            m_frame.reset();
        }
    }

    //AVFrame空间申请
    if (!m_frame) {
        TRY_CATCH(CHECK_EXC(m_frame = new_XAVFrame()),return {});
        m_frame->width = m_width;
        m_frame->height = m_height;
        m_frame->format = m_fmt;
        m_frame->linesize[0] = m_width * m_pixel_Byte_size; //RGBA ARGB BGRA RGB24 交错模式适用
        if (AV_PIX_FMT_YUV420P == m_frame->format) {
            m_frame->linesize[0] = m_width;
            m_frame->linesize[1] = m_width / 2;
            m_frame->linesize[2] = m_width / 2;
        }
#if 0
        if (AV_PIX_FMT_RGB24 == m_frame->format) { //RGB24
            m_frame->linesize[0] = m_width * 3;
        }
#endif
        if (!m_frame->Get_Buffer(1)) {
            m_frame.reset();
            return {};
        }
    }

    //读取数据
    if (AV_PIX_FMT_YUV420P == m_frame->format) {
        for (uint32_t i{}; i < 3; ++i) {
            const auto read_size{ i ? m_frame->linesize[i] * m_height / 2 :
                                 m_frame->linesize[i] * m_height };
            m_ifs.read(reinterpret_cast<char *>(m_frame->data[i]),read_size);
        }
    } else { //只适合交错模式
        const auto read_size{ m_frame->linesize[0] * m_height};
        m_ifs.read(reinterpret_cast<char *>(m_frame->data[0]),read_size);
    }

    return m_frame;
}
