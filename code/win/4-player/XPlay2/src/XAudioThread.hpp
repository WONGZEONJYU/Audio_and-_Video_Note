//
// Created by wong on 2024/8/8.
//

#ifndef XPLAY2_XAUDIOTHREAD_HPP
#define XPLAY2_XAUDIOTHREAD_HPP

#include "XAVQThread_Abstract.hpp"

class XResample;
class XAudioPlay;

class XAudioThread : public XAVQThread_Abstract {

Q_OBJECT
    void DeConstruct() noexcept(true);
    void run() noexcept(false) override;
public:
    explicit XAudioThread(std::exception_ptr * = nullptr);
    void Open(const XAVCodecParameters_sptr &) noexcept(false) override;
    //virtual void Push(XAVPacket_sptr &&) ;

protected:
    std::vector<uint8_t> m_resample_datum;
    QSharedPointer<XResample> m_resample;
    XAudioPlay *m_audio_play{};

public:
    ~XAudioThread() override;
};

#endif
