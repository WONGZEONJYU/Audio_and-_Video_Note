//
// Created by Administrator on 2024/8/6.
//

#ifndef XPLAY2_QXAUDIOPLAY_HPP
#define XPLAY2_QXAUDIOPLAY_HPP

#include "XAudioPlay.hpp"
#include "XHelper.hpp"
#include <QMutex>
#include <QSharedPointer>

class QAudioSink;
class QIODevice;

/**
 * Open Close Write需在同一个线程下操作以下函数,否则容易出现问题
 */

class QXAudioPlay final : public XAudioPlay {

    explicit QXAudioPlay() = default;
    void Deconstruct() noexcept(true);
    void Open() noexcept(false) override ;
    void Close() noexcept(true) override;
    [[nodiscard]] uint64_t FreeSize() const noexcept(false) override;
    [[nodiscard]] uint64_t BufferSize() const noexcept(true) override;
    void Write(const uint8_t *,const int64_t &) noexcept(false) override;
    void QtSetParent(void *) noexcept(true) override;
    [[nodiscard]] int64_t NoPlayMs() const override;
    void SetPause(const bool &) noexcept(true) override;
    void Clear() noexcept(true) override;
    void SetVolume(const double &) noexcept(true) override;
    double Volume() const noexcept(true) override;
public:
    static XAudioPlay *handle();

private:
    QMutex m_mux;
    QSharedPointer<QAudioSink> m_output;
    /**
     * 不能delete m_IO,只能置空
     */
    QIODevice* m_IO{};

public:
    ~QXAudioPlay() override;
    X_DISABLE_COPY_MOVE(QXAudioPlay)
};

#endif
