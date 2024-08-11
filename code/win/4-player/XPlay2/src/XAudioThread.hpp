//
// Created by wong on 2024/8/8.
//

#ifndef XPLAY2_XAUDIOTHREAD_HPP
#define XPLAY2_XAUDIOTHREAD_HPP

#include "XAVQThreadAbstract.hpp"

class XResample;
class XAudioPlay;

class XAudioThread : public XAVQThreadAbstract {

Q_OBJECT
    void DeConstruct() noexcept(true);
    void run() noexcept(false) override;
public:
    explicit XAudioThread(std::exception_ptr * = nullptr);
    void Open(const XAVCodecParameters_sptr &) noexcept(false) override;
    using XAVQThreadAbstract::Push;
    using XAVQThreadAbstract::SetException_ptr;
    using XAVQThreadAbstract::Pts;

protected:
    std::vector<uint8_t> m_resample_datum;
    QSharedPointer<XResample> m_resample;
    XAudioPlay *m_audio_play{};

public:
    ~XAudioThread() override;
};

#endif
