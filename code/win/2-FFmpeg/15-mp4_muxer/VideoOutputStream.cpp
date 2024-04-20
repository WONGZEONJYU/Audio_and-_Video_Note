//
// Created by Administrator on 2024/4/18.
//

#include "VideoOutputStream.h"
#include "Muxer.h"
#include "VideoEncoder.h"

void VideoOutputStream::Construct(const Muxer_sp_type& muxer,
                                  const Video_Encoder_params& params) noexcept(false) {

    m_encoder = VideoEncoder::create(params);

    m_stream = muxer->create_stream();
    if (!m_stream){
        throw std::runtime_error("");
    }

    m_encoder->parameters_from_context(m_stream->codecpar);
    muxer->dump_format(m_stream->index);
}

VideoOutputStream::VideoOutputStream_sp_type VideoOutputStream::create(const Muxer_sp_type& muxer,
                                                                       const Video_Encoder_params& params) {
    VideoOutputStream_sp_type obj;

    try {
        obj = VideoOutputStream_sp_type(new VideoOutputStream);
    } catch (const std::bad_alloc &e) {
        throw std::runtime_error("new VideoOutputStream failed: " + std::string (e.what()) + "\n");
    }

    try {
        obj->Construct(muxer,params);
        return obj;
    } catch (const std::runtime_error &e) {
        obj.reset();
        throw std::runtime_error("VideoOutputStream Construct failed: "  + std::string (e.what()) + "\n");
    }
}

