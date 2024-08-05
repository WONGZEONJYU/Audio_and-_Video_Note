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

class TestThread : public QThread {

    void run() override{

        try {

            while (true) {
                p = x.Read();
                if (!p){ //媒体文件读取为空
                    qDebug() << "read finish\n";
                    ad.Send({});
                    vd.Send({});

                    while (true){

                        af = ad.Receive();
                        vf = vd.Receive();

                        if (af){
                            //qDebug() << av_get_sample_fmt_name(static_cast<AVSampleFormat>(af->format));
                            qDebug() << "ReSample_nb: " << re.Resample(af,resampleData) << " capacity: " << resampleData.capacity();
                        }

                        if (vf){
                            xVideoWidget->Repaint(vf);
                            //qDebug() << av_get_pix_fmt_name(static_cast<AVPixelFormat>(vf->format));
                            QThread::msleep(40);
                        }

                        if (!af && !vf){
                            break;
                        }
                    }
                    break;
                }

                if (x.is_Audio(p)){
                    ad.Send(p);
                    while (true){
                        af = ad.Receive();
                        if (af){
                            //qDebug() << av_get_sample_fmt_name(static_cast<AVSampleFormat>(af->format));
                            qDebug() << "ReSample_nb: " << re.Resample(af,resampleData) << " capacity: " << resampleData.capacity();
                        } else{
                            break;
                        }
                    }

                } else{
                    vd.Send(p);
                    while (true){
                        vf = vd.Receive();
                        if (vf) {
                            xVideoWidget->Repaint(vf);
                            qDebug() << av_get_pix_fmt_name(static_cast<AVPixelFormat>(vf->format));
                            QThread::msleep(40);
                        } else{
                            break;
                        }
                    }
                }
            }
        } catch (...) {
            *eptr = std::current_exception();
        }
    }

public:
    void init(){
        try {
            x.Open("2_audio.mp4");
            xVideoWidget->Init(x.widget(),x.height());
            c = x.copy_ALLCodec_Parameters();
            vd.Open(c->at(2));
            ad.Open(c->at(0));
            re.Open(c->at(0));
        } catch (...) {
            *eptr = std::current_exception();
        }
    }

    XDemux x;
    XDecode vd,ad;
    XAVCodecParameters_sptr_container_sptr c;
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
