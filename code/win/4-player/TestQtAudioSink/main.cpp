#include <QCoreApplication>
#include <QAudioFormat>
#include <QFile>
#include <QAudioSink>
#include <QThread>
#include <QMediaDevices>

#define GET_STR(args) #args

int main(int argc, char *argv[]) {

    QCoreApplication a(argc, argv);

    auto device {QMediaDevices::defaultAudioOutput()};
    auto fmt {device.preferredFormat()};

    fmt.setSampleRate(44100);
    fmt.setChannelCount(2);
    fmt.setSampleFormat(QAudioFormat::Int16);
    fmt.setChannelConfig(QAudioFormat::ChannelConfigStereo);

    auto audio_sink{new QAudioSink(device,fmt)};
    QCoreApplication::connect(audio_sink,&QAudioSink::stateChanged,[audio_sink](QAudio::State state){
        switch (state) {
            case QAudio::ActiveState:
                break;

            case QAudio::SuspendedState:
                break;

            case QAudio::StoppedState:
                break;

            case QAudio::IdleState:
                break;
        }
        qDebug() << state;
    });

    QFile file("out.pcm");
    if (!file.open(QFile::ReadOnly)){
        qDebug() << GET_STR(out.pcm open failed);
        return -1;
    }

#define EN 0

#if EN
    audio_sink->start(&file);
#else
    auto io{audio_sink->start()};
    const auto buf_size{audio_sink->bufferSize()};
    auto buf{new char [buf_size]{}};
    while (true){
        const auto free_size {audio_sink->bytesFree()};

        if (free_size < buf_size){
            qDebug() << GET_STR(free_size = ) << free_size;
            QThread::usleep(1000);
            continue;
        }
        const auto re{file.read(buf,buf_size)};
        if(re <= 0){
            break;
        }
        io->write(buf,re);
    }
#endif
    const auto ret {QCoreApplication::exec()};
    delete audio_sink;
#if !EN
    delete []buf;
#endif
    return ret;
}


