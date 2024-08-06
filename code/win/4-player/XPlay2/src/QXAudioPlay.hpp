//
// Created by Administrator on 2024/8/6.
//

#ifndef XPLAY2_QXAUDIOPLAY_HPP
#define XPLAY2_QXAUDIOPLAY_HPP

#include "XAudioPlay.hpp"
#include "XHelper.hpp"
#include <QRecursiveMutex>
#include <QSharedPointer>

class QAudioSink;
class QIODevice;

class QXAudioPlay final : public XAudioPlay {

    QXAudioPlay() = default;
    void Deconstruct() noexcept(true);
    void Open() noexcept(false) override ;
    void Close() noexcept(true) override;
    void Write(const uint8_t *,const int64_t &) noexcept(false) override;
    [[nodiscard]] uint64_t FreeSize() const noexcept(false) override;

public:
    static XAudioPlay* handle() ;

private:
    QRecursiveMutex m_re_mux;
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
