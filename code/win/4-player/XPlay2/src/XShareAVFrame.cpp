//
// Created by Administrator on 2024/4/19.
//

extern "C"{
#include <libavutil/frame.h>
}

#include "XShareAVFrame.hpp"
#include <string>

XShareAVFrame:: XShareAVFrame() noexcept(true):
m_frame{av_frame_alloc()}{

}

void XShareAVFrame::Construct() const noexcept(false) {
    if (!m_frame){
        throw std::runtime_error("av_frame_alloc failed");
    }
}

ShareAVFrame_sp_type XShareAVFrame::create() {
    ShareAVFrame_sp_type obj;
    try {
        obj.reset(new XShareAVFrame);
        obj->Construct();
        return obj;
    }catch (const std::bad_alloc &e){
        throw std::runtime_error("new XShareAVFrame  failed: " + std::string (e.what()) + "\n");
    }catch (const std::runtime_error &e) {
        obj.reset();
        throw std::runtime_error("XShareAVFrame Construct failed: " + std::string (e.what()) + "\n");
    }
}

void XShareAVFrame::DeConstruct() noexcept(true) {
    av_frame_free(&m_frame);
}

XShareAVFrame::~XShareAVFrame() noexcept(true) {
    DeConstruct();
}

ShareAVFrame_sp_type new_ShareAVFrame() noexcept(false)
{
    return XShareAVFrame::create();
}
