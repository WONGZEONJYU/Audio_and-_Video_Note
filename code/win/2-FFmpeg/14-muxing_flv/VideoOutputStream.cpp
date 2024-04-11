
#include "VideoOutputStream.h"

extern "C"{
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libavformat/avio.h>
}

#include <iostream>

void VideoOutputStream::fill_yuv_image(AVFrame &pict,const int &frame_index,
                    const int &width, const int &height)
{
    /* Y */
    for (int y {}; y < height; y++){
        for (int x {}; x < width; x++){
            pict.data[0][y * pict.linesize[0] + x] = x + y + frame_index * 3;
        }
    }

    /* Cb and Cr */
    for (int y {}; y < height / 2; y++){
        for (int x {}; x < width / 2; x++) {
            pict.data[1][y * pict.linesize[1] + x] = 128 + y + frame_index * 2;
            pict.data[2][y * pict.linesize[2] + x] = 64 + x + frame_index * 5;
        }
    }
}

bool VideoOutputStream::construct() {
    return true;
}

void VideoOutputStream::init_codec_parms() {

    m_avCodecContext->codec_id = m_avFormatContext.oformat->video_codec;
    

}

VideoOutputStream::VideoOutputStream(AVFormatContext &oc):m_avFormatContext(oc){}

bool VideoOutputStream::add_stream() {

    const auto v_codec_id{m_avFormatContext.oformat->video_codec};
    /* 查找编码器 */
    m_codec = avcodec_find_encoder(v_codec_id);

    if (!m_codec){
        std::cerr << "Could not find encoder for " << avcodec_get_name(v_codec_id) << "\n";
        return {};
    }

    m_stream = avformat_new_stream(&m_avFormatContext, nullptr);
    if (!m_stream){
        std::cerr << "Could not allocate stream\n";
        return {};
    }

    m_stream->id = static_cast<int>(m_avFormatContext.nb_streams - 1);

    m_avCodecContext = avcodec_alloc_context3(m_codec);
    if (!m_avCodecContext){
        std::cerr << "Could not alloc an encoding context\n";
        return {};
    }

    init_codec_parms();
    /*初始化编码器参数*/

    return true;
}



bool VideoOutputStream::open() {

    return true;
}

void VideoOutputStream::write_frame() {

}

VideoOutputStream::~VideoOutputStream() {
    av_frame_free(&m_frame);
    av_frame_free(&m_tmp_frame);
}

std::shared_ptr<OutputStreamAbstract> VideoOutputStream::create(AVFormatContext &oc) {

    try {
        auto object{new VideoOutputStream(oc)};
        std::shared_ptr<OutputStreamAbstract> obj(object);
        if (!object->construct()){
            obj.reset();
            throw std::runtime_error("VideoOutputStream construct failed\n");
        }
        return obj;
    } catch (std::exception &e) {
        std::cerr << e.what() << "\n";
        throw std::runtime_error("new VideoOutputStream failed\n");
    }


}






