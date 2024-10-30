#include "xvideo_view.hpp"
#include "xsdl.hpp"
#include "xavframe.hpp"
#include "xcodec_parameters.hpp"

using namespace std;
using namespace this_thread;
using namespace chrono;

void XVideoView::merge_nv12(uint8_t *const cache_,const XAVFrame &frame){

    const auto half_height{frame.height >> 1}; //frame.height / 2

    if (frame.width == frame.linesize[0]) { //无需对齐
        auto src_{frame.data[0]},dst_{cache_};
        const auto cp_y_size{frame.linesize[0] * frame.height};
        copy_n(src_,cp_y_size,dst_); //Y

        src_ = frame.data[1];
        dst_+= cp_y_size;
        const auto cp_uv_size{frame.linesize[1] * half_height};
        copy_n(src_,cp_uv_size,dst_) ; //UV
    }else { //有对齐情况
        for (int i {}; i < frame.height; ++i) { //Y
            const auto src_ {frame.data[0] + frame.linesize[0] * i},
                    dst_{cache_ + frame.width * i};
            copy_n(src_,frame.width,dst_);
        }

        const auto uv_start{cache_ + frame.width * frame.height};

        for (int i {}; i < half_height; ++i) { //UV
            const auto src_{frame.data[1] + frame.linesize[1] * i},
                    dst_{uv_start + frame.width * i};
            copy_n(src_,frame.width,dst_);
        }
    }
}

XVideoView *XVideoView::create(const RenderType &renderType) {
    XVideoView *ptr{};
    switch (renderType) {
        case SDL:
            TRY_CATCH(ptr = new XSDL(),return {});
            return ptr;
        default:
            return {};
    }
}

XVideoView_sp XVideoView::create_sp(const RenderType &renderType) {
    XVideoView_sp sp;
    switch (renderType) {
        case SDL:
            TRY_CATCH(sp.reset(new XSDL()),return {});
            return sp;
        default:
            return {};
    }
}

bool XVideoView::Init(const XCodecParameters &parameters,const std::string &win_title){
    
    auto fmt{parameters.Video_pixel_format()};
    switch (fmt) {
        case AV_PIX_FMT_YUV420P:
        case AV_PIX_FMT_YUVJ420P:
            fmt = YUV420P;
            break;
        default:
            break;
    }
    
    return Init(parameters.Width(),parameters.Height(),static_cast<Format>(fmt),win_title);
}

void XVideoView::calc_fps() {
    static constexpr auto TIME_MS{1000LL};
    ++m_count_;
    if (m_begin_time_ <= 0LL){
        m_begin_time_ = Get_time_ms(); //更新时间
    } else if (Get_time_ms() - m_begin_time_ >= TIME_MS){
        m_render_fps_ = m_count_.load();
        m_count_ = {};
        m_begin_time_ = Get_time_ms(); //更新时间
    } else{}
}

bool XVideoView::DrawFrame(const XAVFrame &frame) {

    const auto b {!frame.data[0] || !frame.width || !frame.height};
    if (b) {
        PRINT_ERR_TIPS(GET_STR(Non-video frames));
        return {};
    }

    calc_fps();

    switch (frame.format) {
        case AV_PIX_FMT_YUV420P:
        case AV_PIX_FMT_YUVJ420P:
            return Draw(frame.data[0],frame.linesize[0],
                        frame.data[1],frame.linesize[1],
                        frame.data[2],frame.linesize[2]);
        case AV_PIX_FMT_NV12:
        case AV_PIX_FMT_NV21:{
#if 1
            return Draw(frame.data[0],frame.linesize[0],
                        frame.data[1],frame.linesize[1]);
#else
            //此处是为了演示如何把NV12/NV21合成一个类似RGB这样的数据结构去渲染
            const auto cache_size{static_cast<uint64_t>(frame.width * frame.height * 1.5)};
            if (m_cache.capacity() <= cache_size) {
                m_cache.resize(cache_size + (cache_size >> 1),0);
            }
            merge_nv12(m_cache.data(),frame);
            return Draw(m_cache.data(),frame.width);
#endif
        }
        case AV_PIX_FMT_BGRA:
        case AV_PIX_FMT_RGBA:
        case AV_PIX_FMT_ARGB:
        case AV_PIX_FMT_ABGR:
        case AV_PIX_FMT_RGB24:
        case AV_PIX_FMT_BGR24:
            return Draw(frame.data[0],frame.linesize[0]);
        default:
            PRINT_ERR_TIPS(GET_STR(The current pixel format is not supported));
            return {};
    }
}

bool XVideoView::Open(const std::string &file_path) {

    if (m_ifs_.is_open()) {
        m_ifs_.close();
    }

    m_ifs_.open(file_path,ios::binary);
    if (!m_ifs_) {
        PRINT_ERR_TIPS(GET_STR(open file error!));
        return {};
    }

    return true;
}

XAVFrame_sp XVideoView::Read() {

    auto b{m_width_ <= 0 || m_height_ <= 0 || m_fmt_ < 0 || !m_ifs_};

    if (b) { //参数打开失败
        PRINT_ERR_TIPS(GET_STR(width height fmt error!));
        return {};
    }

    if (m_ifs_.eof()) { //文件是否到达结尾
        PRINT_ERR_TIPS(GET_STR(file at end!));
        return {};
    }

    //AVFrame空间已经申请,如果参数发生变化,需先释放空间
    if (m_frame_) {
        b = m_frame_->width != m_width_ || m_frame_->height != m_height_ || m_frame_->format != m_fmt_;
        if (b) { //判断格式是否发生变化
            //释放AVFrame对象空间和buf引用计数减一
            m_frame_.reset();
        }
    }

    //AVFrame空间申请
    if (!m_frame_) {
        IS_SMART_NULLPTR((m_frame_ = new_XAVFrame()),return {});
        m_frame_->width = m_width_;
        m_frame_->height = m_height_;
        m_frame_->format = m_fmt_;

        m_frame_->linesize[0] = m_width_ * m_pixel_Byte_size_; //RGBA ARGB BGRA RGB24 交错模式适用

        switch (m_frame_->format) {
            case AV_PIX_FMT_YUV420P :
            case AV_PIX_FMT_YUVJ420P:
                m_frame_->linesize[0] = m_width_;
                m_frame_->linesize[1] = m_width_ / 2;
                m_frame_->linesize[2] = m_width_ / 2;
                break;
            case AV_PIX_FMT_NV12:
            case AV_PIX_FMT_NV21:
                m_frame_->linesize[0] = m_width_;
                m_frame_->linesize[1] = m_width_;
                break;
            default:
                break;
        }

        if (!m_frame_->Get_Buffer(1)) {
            m_frame_.reset();
            return {};
        }
    }

    switch (m_frame_->format) {
        case AV_PIX_FMT_YUV420P:
        case AV_PIX_FMT_YUVJ420P:
            for (uint32_t i{}; i < 3; ++i) {
                const auto len{ i ? m_frame_->linesize[i] * m_height_ / 2 :
                                m_frame_->linesize[i] * m_height_ };
                m_ifs_.read(reinterpret_cast<char *>(m_frame_->data[i]),len);
            }
            break;
        case AV_PIX_FMT_NV12:
        case AV_PIX_FMT_NV21:
            for(uint32_t i{};i < 2;++i){
                const auto len{i ? m_frame_->linesize[i] * m_height_ / 2 :
                               m_frame_->linesize[i] * m_height_};
                m_ifs_.read(reinterpret_cast<char*>(m_frame_->data[i]),len);
            }
            break;
        case AV_PIX_FMT_ARGB:
        case AV_PIX_FMT_RGBA:
        case AV_PIX_FMT_ABGR:
        case AV_PIX_FMT_BGRA:
        case AV_PIX_FMT_RGB24:
        case AV_PIX_FMT_BGR24:{
            const auto len{m_frame_->linesize[0] * m_height_};
            m_ifs_.read(reinterpret_cast<char *>(m_frame_->data[0]), len);
        }
            break;
        default:
            PRINT_ERR_TIPS(GET_STR(video format error!));
            m_frame_.reset();
            break;
    }

    return m_frame_;
}

void XVideoView::MSleep(const uint64_t &ms) {
    const auto begin{Get_time_ms()};
    auto ms_{ms};
    while (ms_--) {
        sleep_for(milliseconds(1));
        if (Get_time_ms() - begin >= ms){
            return;
        }
    }
}

int64_t XVideoView::Get_time_ms(){
    const auto now_{high_resolution_clock::now()};
    const auto now_ms{time_point_cast<milliseconds>(now_)};
    return now_ms.time_since_epoch().count();
}
