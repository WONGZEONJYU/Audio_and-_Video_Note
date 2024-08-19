//
// Created by wong on 2024/8/8.
//

#ifndef XPLAY2_XAUDIOTHREAD_HPP
#define XPLAY2_XAUDIOTHREAD_HPP

#include "XDecodeThread.hpp"

class XResample;
class XAudioPlay;
struct sonicStreamStruct;

class XAudioThread : public XDecodeThread {

Q_OBJECT
    void DeConstruct() noexcept(true);
    void entry() noexcept(false) override;
public:
    explicit XAudioThread(std::exception_ptr * = nullptr);
    ~XAudioThread() override;
    void Open(const XAVCodecParameters_sptr &) noexcept(false) override;
    void Close() noexcept(true) override;
    void Clear() noexcept(true) override;
    void SetPause(const bool &b) noexcept(true) override;
    void SetVolume(const double &) noexcept(true);
    [[nodiscard]] double Volume() const noexcept(true);
protected:
    std::atomic<XAudioPlay *> m_audio_play{};
    QSharedPointer<XResample> m_resample;
    QMutex m_a_mux;
    QWaitCondition m_a_cv;
    sonicStreamStruct *m_SonicStream{};
};

#endif
