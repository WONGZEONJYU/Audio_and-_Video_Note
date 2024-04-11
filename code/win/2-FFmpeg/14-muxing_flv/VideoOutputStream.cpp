#include "VideoOutputStream.h"

extern "C" {
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libavformat/avio.h>
}

#include <iostream>
#include "AVHelper.h"

AVFrame *VideoOutputStream::alloc_picture()
{
    auto frame{av_frame_alloc()};
    if (!frame){
        std::cerr << "av_frame_alloc failed\n";
        return {};
    }

    frame->width = m_avCodecContext->width;
    frame->height = m_avCodecContext->height;
    frame->format = m_avCodecContext->pix_fmt;

    if (av_frame_get_buffer(frame, 32) < 0){
        std::cerr << "Could not allocate frame data.\n";
        av_frame_free(&frame);
        return {};
    }

    return frame;
}

void VideoOutputStream::fill_yuv_image(AVFrame &pict)
{
    const auto frame_index{m_next_pts};
    const auto width {m_avCodecContext->width},
            height{m_avCodecContext->height};

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

bool VideoOutputStream::construct()
{
   return add_stream() && open();
}

void VideoOutputStream::init_codec_parms()
{
    m_avCodecContext->codec_id = m_avFormatContext.oformat->video_codec;
    m_avCodecContext->bit_rate = 400000;
    /* Resolution must be a multiple of two. */
    m_avCodecContext->width = 352;      // 分辨率
    m_avCodecContext->height = 288;
    m_avCodecContext->max_b_frames = 1;
    /* timebase: This is the fundamental unit of time (in seconds) in terms
     * of which frame timestamps are represented. For fixed-fps content,
     * timebase should be 1/framerate and timestamp increments should be
     * identical to 1. */
    m_stream->time_base = { 1, AV_PIX_FMT_YUV420P };  // 时基
    m_avCodecContext->time_base = m_stream->time_base;    // 为什么这里需要设置
    m_avCodecContext->gop_size = STREAM_FRAME_RATE; //
    m_avCodecContext->pix_fmt = STREAM_PIX_FMT;

    /* Some formats want stream headers to be separate. */
    if (m_avFormatContext.oformat->flags & AVFMT_GLOBALHEADER){
        m_avCodecContext->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
    }
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

bool VideoOutputStream::open()
{
    auto ret {avcodec_open2(m_avCodecContext, m_codec, nullptr)};
    if (ret < 0){
        std::cerr << "Could not open video codec: " << AVHelper::av_get_err(ret) << "\n";
        return {};
    }

    m_frame = alloc_picture();
    if (!m_frame){
        return {};
    }

    if (AV_PIX_FMT_YUV420P != m_avCodecContext->pix_fmt){
        // 编码器格式需要的数据不是 AV_PIX_FMT_YUV420P才需要 调用图像scale
        m_tmp_frame = alloc_picture();
        if (!m_tmp_frame){
            std::cerr << "Could not allocate temporary picture\n";
            return {};
        }
    }

    /* copy the stream parameters to the muxer */
    if(avcodec_parameters_from_context(m_stream->codecpar, m_avCodecContext) < 0){
        std::cerr << "Could not copy the stream parameters\n";
        return {};
    }

    return true;
}

void VideoOutputStream::write_frame()
{

}

VideoOutputStream::~VideoOutputStream() {
    avcodec_free_context(&m_avCodecContext);
    av_frame_free(&m_frame);
    av_frame_free(&m_tmp_frame);
}

std::shared_ptr<OutputStreamAbstract> VideoOutputStream::create(AVFormatContext &oc)
{
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

bool VideoOutputStream::sws_init() {

//    sws_getContext(codec_ctx->width, codec_ctx->height,
//                   AV_PIX_FMT_YUV420P,
//                   codec_ctx->width, codec_ctx->height,
//                   codec_ctx->pix_fmt,
//                   SCALE_FLAGS, NULL, NULL, NULL);

    if (STREAM_PIX_FMT != m_avCodecContext->pix_fmt){
        try {
            m_sws = SwsContext_t::create();
        } catch (std::runtime_error &e) {
            return {};
        }
    }

    return true;
}






