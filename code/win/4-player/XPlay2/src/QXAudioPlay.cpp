//
// Created by Administrator on 2024/8/6.
//

#include "QXAudioPlay.hpp"
#include <QAudioFormat>
#include <QAudioSink>
#include <QMediaDevices>
#include <QEventLoop>
#include <QFuture>
#include <QFutureWatcher>
#include <QtConcurrent/QtConcurrent>


void QXAudioPlay::Open() {
    Close();
    const auto dev{QMediaDevices::defaultAudioOutput()};
    auto fmt{dev.preferredFormat()};

    fmt.setSampleRate(m_SampleRate);
    fmt.setChannelCount(m_SampleFormat);
    fmt.setSampleFormat(static_cast<QAudioFormat::SampleFormat>(m_SampleFormat));
    fmt.setChannelCount(m_Channels);
    fmt.setChannelConfig(QAudioFormat::ChannelConfigStereo);

    QMutexLocker locker(&m_re_mux);
    CHECK_EXC(m_output.reset(new QAudioSink(dev,fmt)),locker.unlock());
    CHECK_NULLPTR(m_IO = m_output->start(),m_output.reset(),locker.unlock());
}

void QXAudioPlay::Close() noexcept(true) {
    QMutexLocker locker(&m_re_mux);
    Deconstruct();
}

QXAudioPlay::~QXAudioPlay() {
    Deconstruct();
}

XAudioPlay *QXAudioPlay::handle() {
    static QMutex mux;
    QMutexLocker locker(&mux);
    static QXAudioPlay qx;
    return std::addressof(qx);
}

void QXAudioPlay::Deconstruct() noexcept(true) {

    if (m_IO){
        m_IO->close();
        m_IO = nullptr;
    }

    if (m_output){
        m_output->stop();
        m_output.reset();
    }
}

void QXAudioPlay::Write(const uint8_t *data, const int64_t &data_size) {

    if (!data || data_size <= 0) {
        PRINT_ERR_TIPS(GET_STR(data or data_size error));
        return;
    }

    QMutexLocker locker(&m_re_mux);
    if (!m_output || !m_IO) {
        PRINT_ERR_TIPS(GET_STR(Please turn on the device first));
        return;
    }

    const auto ret {m_IO->write(reinterpret_cast<const char *>(data),data_size)};

    if (ret < 0){
        CHECK_EXC(throw std::runtime_error(m_IO->errorString().toStdString()),locker.unlock());
    }else if (data_size != ret) {
        PRINT_ERR_TIPS(GET_STR(data_size != ret));
    } else{}

}

void QXAudioPlay::QtSetParent(void *p) noexcept(true){
    if (!m_output){
        PRINT_ERR_TIPS(GET_STR(Please turn on the device first));
        return;
    }
    m_output->setParent(static_cast<QObject*>(p));
}

uint64_t QXAudioPlay::FreeSize() const noexcept(false) {
    QMutexLocker locker(const_cast<decltype(m_re_mux)*>(std::addressof(m_re_mux)));
    if (!m_output){
        PRINT_ERR_TIPS(GET_STR(Please turn on the device first));
        return 0;
    }
    return m_output->bytesFree();
}

uint64_t QXAudioPlay::BufferSize() const noexcept(true) {
    QMutexLocker locker(const_cast<decltype(m_re_mux)*>(std::addressof(m_re_mux)));
    if (!m_output){
        PRINT_ERR_TIPS(GET_STR(Please turn on the device first));
        return 0;
    }
    return m_output->bufferSize();
}
