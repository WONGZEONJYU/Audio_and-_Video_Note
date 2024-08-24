//
// Created by Administrator on 2024/8/6.
//

#include "QXAudioPlay.hpp"
#include <QAudioFormat>
#include <QAudioSink>
#include <QAudio>
#include <QMediaDevices>
#include <QtConcurrent/QtConcurrent>

void QXAudioPlay::Open() {
    Close();
    const auto dev{QMediaDevices::defaultAudioOutput()};
    auto fmt{dev.preferredFormat()};

    fmt.setSampleRate(m_SampleRate);
    fmt.setChannelCount(m_SampleFormat);
    fmt.setSampleFormat(static_cast<QAudioFormat::SampleFormat>(m_SampleFormat.load()));
    fmt.setChannelCount(m_Channels);
    fmt.setChannelConfig(QAudioFormat::ChannelConfigStereo);

    QMutexLocker locker(&m_mux);
    CHECK_EXC(m_output.reset(new QAudioSink(dev,fmt)),locker.unlock());
    m_output->setBufferSize(m_output->bufferSize() * 8);
    CHECK_NULLPTR(m_IO = m_output->start(),m_output.reset(),locker.unlock());
    m_is_change = false;
    QObject::connect(m_output.get(),&QAudioSink::stateChanged,[&](QAudio::State state){
        qDebug() << state;
        qDebug() << m_output->error();
    });
}

void QXAudioPlay::Close() noexcept(true) {
    QMutexLocker locker(&m_mux);
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

void QXAudioPlay::Write(const uint8_t *data, const int64_t &data_size) noexcept(false){

    if (!data || data_size <= 0) {
        PRINT_ERR_TIPS(GET_STR(data or data_size error));
        return;
    }

    QMutexLocker locker(&m_mux);
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
    QMutexLocker locker(const_cast<decltype(m_mux)*>(std::addressof(m_mux)));
    if (!m_output){
        PRINT_ERR_TIPS(GET_STR(Please turn on the device first));
        return 0;
    }
    return m_output->bytesFree();
}

uint64_t QXAudioPlay::BufferSize() const noexcept(true) {
    QMutexLocker locker(const_cast<decltype(m_mux)*>(std::addressof(m_mux)));
    if (!m_output){
        PRINT_ERR_TIPS(GET_STR(Please turn on the device first));
        return 0;
    }
    return m_output->bufferSize();
}

int64_t QXAudioPlay::NoPlayMs() const {
    QMutexLocker locker(const_cast<decltype(m_mux)*>(std::addressof(m_mux)));
    if (!m_output){
        PRINT_ERR_TIPS(GET_STR(Please turn on the device first));
        return 0;
    }
    /**
     * 未播放到字节数
     */
    const auto no_play_bytes{
        static_cast<double>(m_output->bufferSize() - m_output->bytesFree())
    };
    locker.unlock();
    /**
     * 每秒采样到的字节数,公式 = 采样率 * 通道数 * (采样深度/8)
     */
    const auto Bytes_per_second{
        static_cast<double>(m_SampleRate * m_Channels * m_SampleFormat)
    };

    return Bytes_per_second <= 0 ? 0 : static_cast<int64_t >(no_play_bytes / Bytes_per_second * 1000.0);
}

void QXAudioPlay::SetPause(const bool &b) noexcept(true) {
    QMutexLocker locker(&m_mux);
    if (!m_output){
        PRINT_ERR_TIPS(GET_STR(Please turn on the device first));
        return;
    }
    b ? m_output->suspend() : m_output->resume();
}

void QXAudioPlay::Clear() noexcept(true) {
    QMutexLocker locker(&m_mux);
    if (m_IO){
        m_IO->reset();
    }
}

void QXAudioPlay::SetVolume(const double &n) noexcept(true){
    QMutexLocker locker(&m_mux);
    if (m_output){
        m_output->setVolume(n);
    }
}

double QXAudioPlay::Volume() const noexcept(true){
    QMutexLocker locker(const_cast<QMutex*>(&m_mux));
    if (m_output){
        return m_output->volume();
    }
    return -1.0;
}
