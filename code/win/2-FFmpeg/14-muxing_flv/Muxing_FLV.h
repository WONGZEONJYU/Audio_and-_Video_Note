//
// Created by Administrator on 2024/4/10.
//

#ifndef INC_14_MUXING_FLV_MUXING_FLV_H
#define INC_14_MUXING_FLV_MUXING_FLV_H

#include <memory>
#include <string>

extern "C"{
#include <libavutil/pixfmt.h>
};

struct AVFormatContext;

struct AVCodecContext;
struct AVOutputFormat;
struct AVCodec;
struct AVDictionary;

class Muxing_FLV final {

    static inline constexpr auto STREAM_DURATION{5.0};
    static inline constexpr auto STREAM_FRAME_RATE{25};
    static inline constexpr auto STREAM_PIX_FMT{AV_PIX_FMT_YUV420P};
    // 封装单个输出AVStream
//    struct OutputStream final {
//        OutputStream(const OutputStream&) = delete;
//        OutputStream& operator=(const OutputStream&) = delete;
//        explicit OutputStream() = default;
//
//        AVStream *m_avstream{}; // 代表一个stream, 1路audio或1路video都代表独立的steam
//        AVCodecContext *m_codecContext{};// 编码器上下文
//
//        /* pts of the next frame that will be generated */
//        int64_t m_next_pts{};
//
//        AVFrame *m_frame{},     //重采样后的frame,视频叫scale
//                *m_tmp_frame{}; //重采样前
//
//        double m_t{}, m_tincr{}, m_tincr2{}; //这几个参数用来生成PCM和YUV用的
//
//        uint32_t m_samples_count{};  //音频的采样数量累计
//
//        SwsContext_t::SwsContext_t_sp_type m_sws{}; //图像scale
//        SwrContext_t::SwrContext_sp_t m_swr{}; //音频重采样
//        ~OutputStream();
//    };

    explicit Muxing_FLV(std::string );
    bool construct() noexcept;
    void destory();
    bool add_stream();
    void init_codec_parms();

public:
    using Muxing_FLV_sp_type = std::shared_ptr<Muxing_FLV>;
    Muxing_FLV(const Muxing_FLV&) = delete;
    Muxing_FLV& operator= (const Muxing_FLV&) = delete;
    static Muxing_FLV_sp_type create(const std::string &) noexcept(false);
    ~Muxing_FLV();

private:
    const std::string m_filename;
    AVFormatContext *m_fmt_ctx{};
    const AVCodec *video_codec{},*audio_codec{};
    AVDictionary* opt{};

};

#endif //INC_14_MUXING_FLV_MUXING_FLV_H
