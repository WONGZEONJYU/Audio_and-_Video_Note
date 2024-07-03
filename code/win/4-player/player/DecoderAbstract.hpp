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
    DecoderAbstract(const DecoderAbstract&) = delete;
    DecoderAbstract &operator=(const DecoderAbstract&) = delete;
    DecoderAbstract(DecoderAbstract&&) = delete;
    DecoderAbstract &operator=(DecoderAbstract&&) = delete;

protected:
    explicit DecoderAbstract(Cv_Any_Type &,PacketQueue &);
    virtual void av_decode_thread(void *) = 0;
    void Notify_All() noexcept(true);
    void Set_Pkt_Serial(const int &n){
        m_pkt_serial = n;
    };

    [[nodiscard]] auto Pkt_Serial() const {return m_pkt_serial;}

private:
    Cv_Any_Type& m_cv;
    PacketQueue &m_queue;
    std::thread m_av_decode_thread;
    int m_pkt_serial{};
    std::atomic_bool m_finished_{};
};


#endif //PLAYER_DECODERABSTRACT_HPP
