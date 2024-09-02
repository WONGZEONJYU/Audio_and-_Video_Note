//
// Created by Administrator on 2024/8/28.
//

#include "xvideo_view.hpp"
#include "xsdl.hpp"
#include "XAVFrame.hpp"
#include <thread>

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
        m_begin_time = Get_time_ms();
    } else if (Get_time_ms() - m_begin_time >= TIME_MS){
        m_view_fps = m_count.load();
        m_count = {};
        m_begin_time = Get_time_ms();
    } else{}
}

bool XVideoView::DrawFrame(const XAVFrame_sptr &frame) {

    if (!frame || !frame->data[0] || !frame->width) {
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
        std::this_thread::sleep_for(std::chrono::microseconds(1));
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
