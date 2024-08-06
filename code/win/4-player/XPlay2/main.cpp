extern "C" {
#include <libavutil/pixdesc.h>
}

#include <QApplication>
#include <QSurfaceFormat>
#include <QThread>
#include "XVideoWidget.hpp"
#include "XPlay2Widget.hpp"
#include "XDemux.hpp"
#include "XAVPacket.hpp"
#include "XAVFrame.hpp"
#include "XAVCodecParameters.hpp"
#include "XDecode.hpp"
#include "ui_XPlay2Widget.h"
#include "XResample.hpp"
#include "QXAudioPlay.hpp"

class TestThread : public QThread {

    void run() override{

        try {
            while (true) {
                p = x.Read();
                if (!p){ //媒体文件读取为空
                    qDebug() << "read finish\n";
                    break;
                    ad.Send({});
                    vd.Send({});

                    while (true){
                        if ((af = ad.Receive())) {
                            const auto relen {re.Resample(af,resampleData)};
                            //qDebug() << "ReSample_nb: " << relen << " capacity: " << resampleData.capacity();
                            if (QXAudioPlay::handle()->FreeSize() >= relen){
                                QXAudioPlay::handle()->Write(resampleData.data(),relen);
                                QThread::msleep(1);
                            }
                        }

                        if ((vf = vd.Receive())) {
                            xVideoWidget->Repaint(vf);
                            QThread::msleep(30);
                        }

                        if (!af && !vf){
                            break;
                        }
                    }
                    break;
                }

                if (x.is_Audio(p)){
                    ad.Send(p);
                    const auto buffersize{QXAudioPlay::handle()->BufferSize()};
                    while (true){

                        const auto freesize {QXAudioPlay::handle()->FreeSize()};
                        if (freesize < buffersize){
                            QThread::msleep(1);
                            continue;
                        }

                        if ((af = ad.Receive())){
                            const auto relen {re.Resample(af,resampleData)};
                            qDebug() << "ReSample_nb: " << relen << " capacity: " << resampleData.capacity();
                            QXAudioPlay::handle()->Write(resampleData.data(),relen);
                            //QThread::msleep(1);
                        }else{
                            break;
                        }
                    }
                } else{
//                    vd.Send(p);
//                    while ((vf = vd.Receive())){
//                        xVideoWidget->Repaint(vf);
//                        //QThread::msleep(30);
//                    }
                }
            }
        } catch (...) {
            *eptr = std::current_exception();
        }
    }

public:
    void init(){
        x.Open("2_audio.mp4");
        //x.Open("");
        xac = x.copy_ACodec_Parameters();
        xvc = x.copy_VCodec_Parameters();

        if (xac){
            ad.Open(xac->at(0));
            re.Open(xac->at(0));
            QXAudioPlay::handle()->set_Audio_parameter(xac->at(0)->Sample_rate(),xac->at(0)->Ch_layout()->nb_channels);
            QXAudioPlay::handle()->Open();
        }

        if (xvc){
            xVideoWidget->Init(xvc->at(2)->Width(),xvc->at(2)->Height());
            vd.Open(xvc->at(2));
        }
    }

    XDemux x;
    XDecode vd,ad;
    XAudio_CodecParameters xac;
    XVideo_CodecParameters xvc;
    XAVPacket_sptr p;
    XAVFrame_sptr af,vf;
    XResample re;
    std::exception_ptr *eptr{};
    XVideoWidget *xVideoWidget{};
    resample_data_t resampleData;
};

int main(int argc, char *argv[]) {
    QApplication a(argc, argv);
    std::exception_ptr eptr;
#if defined(__APPLE__) && defined(__MACH__)
    QSurfaceFormat format;
    format.setVersion(4, 1);
    format.setDepthBufferSize(24);
    format.setStencilBufferSize(8);
    format.setProfile(QSurfaceFormat::CoreProfile);
    QSurfaceFormat::setDefaultFormat(format);
#endif

    try {
        auto w{XPlay2Widget::Handle()};
        w->show();

        TestThread t;
        t.eptr = std::addressof(eptr);
        t.xVideoWidget = w->m_ui->VideoWidget;
        t.init();
        t.start();

        const auto ret{QApplication::exec()};
        t.quit();
        t.terminate();
        t.wait();
        if (eptr){
            std::rethrow_exception(eptr);
        }
        return ret;
    } catch (const std::exception &e) {
        qDebug() << e.what();
        return -1;
    }
}
