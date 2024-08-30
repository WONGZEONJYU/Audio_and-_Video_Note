//
// Created by Administrator on 2024/8/28.
//

#include "xvideo_view.hpp"
#include "xsdl.hpp"
#include "XAVFrame.hpp"

XVideoView *XVideoView::create(const XVideoView::RenderType &renderType) {
    switch (renderType) {
        case SDL:
            return new XSDL();
        default:
            return {};
    }
}

bool XVideoView::DrawFrame(const XAVFrame_sptr &frame) {

    if (!frame || !frame->data[0] || !frame->width) {
        PRINT_ERR_TIPS(GET_STR(Non-video frames));
        return {};
    }

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
