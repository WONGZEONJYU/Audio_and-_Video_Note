//
// Created by wong on 2024/8/8.
//

#ifndef XPLAY2_XAUDIOTHREAD_HPP
#define XPLAY2_XAUDIOTHREAD_HPP

#include "XDecodeThread.hpp"

class XResample;
class XAudioPlay;
class XSonic;
class XAVFrame;

class XAudioThread : public XDecodeThread {

Q_OBJECT
    void DeConstruct() noexcept(true);
    void entry() noexcept(false) override;
    int Resample(std::shared_ptr<XAVFrame> &&,
            std::vector<uint8_t> &,
                    int &) noexcept(false);
    int Speed_Change(std::vector<uint8_t> &,
                    const int &,const int &,
                    std::vector<uint8_t> &);

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
    QMutex m_a_mux;
    std::atomic<XAudioPlay *> m_audio_play{};
    QSharedPointer<XResample> m_resample;

    //QWaitCondition m_a_cv;
    QSharedPointer<XSonic> m_xSonic;
    std::atomic_int m_Channels{},
                m_Out_Sample_Format{},
                m_Sample_Rate{};
};

#endif
