//
// Created by Administrator on 2024/4/18.
//

#include "VideoOutputStream.h"


void VideoOutputStream::Construct() noexcept(false) {

}

VideoOutputStream::VideoOutputStream_sp_type VideoOutputStream::create() {

    VideoOutputStream_sp_type obj;

    try {
        obj = VideoOutputStream_sp_type(new VideoOutputStream);
    } catch (const std::bad_alloc &e) {
        throw std::runtime_error("new VideoOutputStream failed: " + std::string (e.what()) + "\n");
    }

    try {
        obj->Construct();
        return obj;
    } catch (const std::runtime_error &e) {
        obj.reset();
        throw std::runtime_error("VideoOutputStream Construct failed: "  + std::string (e.what()) + "\n");
    }
}

