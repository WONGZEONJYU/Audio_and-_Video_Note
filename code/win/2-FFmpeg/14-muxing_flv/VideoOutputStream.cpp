#include "VideoOutputStream.h"

extern "C" {
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
}

#include <iostream>
#include "AVHelper.h"

AVFrame *VideoOutputStream::alloc_picture()
{
    auto frame = av_frame_alloc();
    if (!frame){
        std::cerr << "av_frame_alloc failed\n";
        return {};
    }

    frame->width = m_codec_ctx->width;
    frame->height = m_codec_ctx->height;
    frame->format = m_codec_ctx->pix_fmt;

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
    const auto width {m_codec_ctx->width},
            height{m_codec_ctx->height};

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

bool VideoOutputStream::construct() noexcept
{
   return add_stream(m_fmt_ctx.oformat->video_codec) && open_video() && sws_init();
}

/*
 * 初始化编码器参数
 */
void VideoOutputStream::config_codec_params()
{
   // m_avCodecContext->codec_id = m_fmt_ctx.oformat->video_codec;
    m_codec_ctx->bit_rate = 400000;
    /* Resolution must be a multiple of two. */
    m_codec_ctx->width = 352;      // 分辨率
    m_codec_ctx->height = 288;
    //m_avCodecContext->max_b_frames = 1;
    /* timebase: This is the fundamental unit of time (in seconds) in terms
     * of which frame timestamps are represented. For fixed-fps content,
     * timebase should be 1/framerate and timestamp increments should be
     * identical to 1. */
    m_stream->time_base = { 1, STREAM_FRAME_RATE };  // 时基
    //m_avCodecContext->framerate = {STREAM_FRAME_RATE,1};

    m_codec_ctx->time_base = m_stream->time_base;    // 为什么这里需要设置

    m_codec_ctx->gop_size = STREAM_FRAME_RATE; //
    m_codec_ctx->pix_fmt = STREAM_PIX_FMT;

    /* Some formats want stream headers to be separate. */
    if (m_fmt_ctx.oformat->flags & AVFMT_GLOBALHEADER){
        m_codec_ctx->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
    }
}

bool VideoOutputStream::open_video()
{
    auto ret {avcodec_open2(m_codec_ctx, m_codec, nullptr)};
    if (ret < 0){
        std::cerr << "Could not open video codec: " << AVHelper::av_get_err(ret) << "\n";
        return {};
    }

    m_frame = alloc_picture();
    if (!m_frame){
        return {};
    }

    if (AV_PIX_FMT_YUV420P != m_codec_ctx->pix_fmt){
        // 编码器格式需要的数据不是 AV_PIX_FMT_YUV420P才需要 调用图像scale
        m_tmp_frame = alloc_picture();
        if (!m_tmp_frame){
            std::cerr << "Could not allocate temporary picture\n";
            return {};
        }
    }

    /* copy the stream parameters to the muxer */
    if(avcodec_parameters_from_context(m_stream->codecpar, m_codec_ctx) < 0){
        std::cerr << "Could not copy the stream parameters\n";
        return {};
    }

    return true;
}

bool VideoOutputStream::sws_init() noexcept{

    if (STREAM_PIX_FMT != m_codec_ctx->pix_fmt){
        try {
            m_sws = SwsContext_t::create(m_codec_ctx->width,m_codec_ctx->height,STREAM_PIX_FMT,
                                         m_codec_ctx->width,m_codec_ctx->height,STREAM_PIX_FMT,
                                         SWS_BICUBIC, nullptr, nullptr, nullptr);
        } catch (const std::runtime_error &e) {
            std::cerr << e.what() << "\n";
            return {};
        }
    }

    return true;
}

bool VideoOutputStream::get_one_frame() noexcept(false)
{
    /*生成5秒的视频 , 如果超过5秒 , 不再生成帧*/
    if (av_compare_ts(m_next_pts,m_codec_ctx->time_base,
                      STREAM_DURATION,{1,1}) >= 0){
        return {};
    }

    /* when we pass a frame to the encoder, it may keep a reference to it
    * internally; make sure we do not overwrite it here */
    auto ret {av_frame_make_writable(m_frame)};
    if (ret < 0){
        throw std::runtime_error("av_frame_make_writable failed :" + AVHelper::av_get_err(ret) + "\n");
    }

    if (STREAM_PIX_FMT != m_codec_ctx->pix_fmt){
        fill_yuv_image(*m_tmp_frame);
        m_sws->sws_scale(m_tmp_frame->data,m_tmp_frame->linesize,0,m_codec_ctx->height,
                         m_frame->data,m_frame->linesize);
    }else{
        fill_yuv_image(*m_frame);
    }

    m_frame->pts = m_next_pts++;

    return true;
}

bool VideoOutputStream::write_frame() noexcept(false)
{
    try {
        const auto b { get_one_frame()};
        const auto *frame{b ? m_frame : nullptr};
        AVPacket pkt{};
        AVHelper::encode(m_codec_ctx,frame,&pkt,[&]{
               write_media_file(pkt);
            });
        return b;
    } catch (const std::runtime_error &e) {
        throw std::runtime_error(e.what());
    }
}

VideoOutputStream::VideoOutputStream(AVFormatContext &oc):
OutputStreamAbstract(oc){}

std::shared_ptr<OutputStreamAbstract> VideoOutputStream::create(AVFormatContext &oc)
{
    try {
        VideoOutputStream_sp_type obj(new VideoOutputStream(oc));
        if (!obj->construct()){
            obj.reset();
            throw std::runtime_error("VideoOutputStream construct failed\n");
        }
        return obj;
    } catch (const std::bad_alloc &e) {
        std::cerr << e.what() << "\n";
        throw std::runtime_error("new VideoOutputStream failed\n");
    }
}
