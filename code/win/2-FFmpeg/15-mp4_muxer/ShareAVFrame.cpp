//
// Created by Administrator on 2024/4/19.
//

extern "C"{
#include <libavutil/frame.h>
}

#include "ShareAVFrame.hpp"
#include <string>

ShareAVFrame:: ShareAVFrame() noexcept(true):
m_frame{av_frame_alloc()}{

}

void ShareAVFrame::Construct() const noexcept(false) {
    if (!m_frame){
        throw std::runtime_error("av_frame_alloc failed");
    }
}

ShareAVFrame::ShareAVFrame_sp_type ShareAVFrame::create() {
    ShareAVFrame_sp_type obj;
    try {
        obj = std::move(ShareAVFrame_sp_type(new ShareAVFrame));
    }catch (const std::bad_alloc &e){
        throw std::runtime_error("new ShareAVFrame  failed: " + std::string (e.what()) + "\n");
    }

    try {
        obj->Construct();
        return obj;
    } catch (const std::runtime_error &e) {
        obj.reset();
        throw std::runtime_error("ShareAVFrame Construct failed: " + std::string (e.what()) + "\n");
    }
}

ShareAVFrame::~ShareAVFrame() noexcept(true) {
    DeConstruct();
}

void ShareAVFrame::DeConstruct() noexcept(true) {
    av_frame_free(const_cast<AVFrame**>(&m_frame));
}
