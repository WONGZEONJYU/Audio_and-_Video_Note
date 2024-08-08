extern "C" {
#include <libavutil/pixdesc.h>
}

#include <QApplication>
#include <QSurfaceFormat>
#include <QThread>
#include <QMetaType>
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
            QXAudioPlay::handle()->set_Audio_parameter(xac->Sample_rate(),xac->Ch_layout()->nb_channels);
            QXAudioPlay::handle()->Open();

            while (!m_stop) {
                p = x.Read();

                if (!p){ //媒体文件读取为空
                    qDebug() << "read finish\n";
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

                if (x.Present_Audio_Index() == p->stream_index){
                    ad.Send(p);
                    while (true){
                        af = ad.Receive();
                        if (af){
                            const auto relen {re.Resample(af,resampleData)};
                            //qDebug() << "ReSample_nb: " << relen << " capacity: " << resampleData.capacity();

                            while (relen > 0){
                                if (QXAudioPlay::handle()->FreeSize() >= relen){
                                    QXAudioPlay::handle()->Write(resampleData.data(),relen);
                                    break;
                                }
                                QThread::msleep(1);
                            }

                        } else{
                            break;
                        }
                    }
                } else if(x.Present_Video_Index() == p->stream_index){
                    vd.Send(p);
                    while ((vf = vd.Receive())){
                        xVideoWidget->Repaint(vf);
                        QThread::msleep(30);
                    }
                }else{
                    p.reset();
                }
            }
        } catch (...) {
            QXAudioPlay::handle()->Close();
            *eptr = std::current_exception();
        }
        QXAudioPlay::handle()->Close();
    }

public:
    void init() {
        x.Open("2_audio.mp4");
        xac = x.Copy_Present_AudioCodecParam();
        xvc = x.Copy_Present_VideoCodecParam();

        if (xac){
            ad.Open(xac);
            re.Open(xac);
        }

        if (xvc){
            xVideoWidget->Init(xvc->Width(),xvc->Height());
            vd.Open(xvc);
        }
    }

    XDemux x;
    XDecode vd,ad;
    XAVCodecParameters_sptr xac;
    XAVCodecParameters_sptr xvc;

    XAVPacket_sptr p;
    XAVFrame_sptr af,vf;
    XResample re;
    std::exception_ptr *eptr{};
    XVideoWidget *xVideoWidget{};
    resample_data_t resampleData;
    std::atomic_bool m_stop;
};

int main(int argc, char *argv[]) {

    QApplication a(argc, argv);

#if defined(__APPLE__) && defined(__MACH__)
    QSurfaceFormat format;
    format.setVersion(4, 1);
    format.setDepthBufferSize(24);
    format.setStencilBufferSize(8);
    format.setProfile(QSurfaceFormat::CoreProfile);
    QSurfaceFormat::setDefaultFormat(format);
#endif
    std::exception_ptr eptr;
    try {
        auto w{XPlay2Widget::Handle()};
        w->show();
        TestThread t;
        t.xVideoWidget = w->m_ui->VideoWidget;

        t.eptr = std::addressof(eptr);
        t.init();
        t.start();
        const auto ret{QApplication::exec()};
        t.quit();
        t.m_stop = true;
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
