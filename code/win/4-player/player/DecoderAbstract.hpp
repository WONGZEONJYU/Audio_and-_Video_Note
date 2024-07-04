//
// Created by Administrator on 2024/7/3.
//

#ifndef PLAYER_DECODERABSTRACT_HPP
#define PLAYER_DECODERABSTRACT_HPP

#include <thread>
#include <condition_variable>
#include "ff_ffplay_def.hpp"

class DecoderAbstract {

public:
    using Cv_Any_Type = std::condition_variable_any;

    void av_decoder_abort(FrameQueue &) noexcept(true);
    void av_decoder_start(void *) noexcept(true);

protected:
    explicit DecoderAbstract(Cv_Any_Type &,PacketQueue &,AVCodecContext &);
    virtual ~DecoderAbstract();
    virtual void av_decoder_thread(void *) = 0;
    void Notify_All() noexcept(true);
    int decode_frame(AVFrame*);
    void Set_Pkt_Serial(const int &n){
        m_pkt_serial = n;
    };

    [[nodiscard]] auto Pkt_Serial() const {
        return m_pkt_serial;
    }

    void Set_Finished(const bool &b){
        m_finished_ = b;
    }

    [[nodiscard]] auto Finished() const {
        return m_finished_.load();
    }

    [[nodiscard]] auto AVCodecCtx() const{
        return m_avcodec_ctx;
    }

    [[nodiscard]] auto AV_Packet() const{
        return &m_pkt;
    }

    [[nodiscard]] auto PktQueue() const{
        return &m_queue;
    }

private:
    Cv_Any_Type& m_cv;
    PacketQueue &m_queue;
    AVCodecContext *m_avcodec_ctx{};
    std::thread m_av_decode_thread;
    int m_pkt_serial{};
    std::atomic_bool m_finished_{};
    AVPacket m_pkt{};

public:
    DecoderAbstract(const DecoderAbstract&) = delete;
    DecoderAbstract &operator=(const DecoderAbstract&) = delete;
    DecoderAbstract(DecoderAbstract&&) = delete;
    DecoderAbstract &operator=(DecoderAbstract&&) = delete;
};

#endif
