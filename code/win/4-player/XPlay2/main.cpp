extern "C" {
#include <libavutil/pixdesc.h>
}

#include <QApplication>
#include <QThread>
#include "IVideoCall.hpp"
#include "XPlay2Widget.hpp"
#include "XDemux.hpp"
#include "XAVPacket.hpp"
#include "ui_XPlay2Widget.h"
#include "XAudioThread.hpp"
#include "XVideoThread.hpp"

#include "XDemuxThread.hpp"

class TestThread : public QThread {

    void run() override {
#if 0
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
#endif
        try {
            while (!m_stop){
                auto pkt{x.Read()};
                if (!pkt){
                    m_stop = true;
                }

                if (x.Present_Audio_Index() == pkt->stream_index){
                    at.Push(std::move(pkt));
                } else if (x.Present_Video_Index() == pkt->stream_index){
                    vt.Push(std::move(pkt));
                }else{
                    pkt.reset();
                }
            }

        } catch (...) {
            if (m_rt_ex){
                *m_rt_ex = std::current_exception();
            }
        }
    }

public:
    ~TestThread() override{
        m_stop = true;
        quit();
        wait();
    }

    void init() {
#if 0
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
#endif
    x.Open("2_audio.mp4");
    xac = x.Copy_Present_AudioCodecParam();
    xvc = x.Copy_Present_VideoCodecParam();
        if (xac){
            at.Open(xac);
            at.start();
        }

        if (xvc){
            vt.Open(xvc,xVideoWidget);
            vt.start();
        }
    }

    XDemux x;
    XAudioThread at;
    XVideoThread vt;
//    XDecode vd,ad;
    XAVCodecParameters_sptr xac;
    XAVCodecParameters_sptr xvc;

    //XAVPacket_sptr p;
//    XAVFrame_sptr af,vf;
//    XResample re;
    std::exception_ptr *m_rt_ex{};

    XVideoWidget *xVideoWidget{};
    //resample_data_t resampleData;
    std::atomic_bool m_stop;
};

int main(int argc, char *argv[]) {

    QApplication a(argc, argv);
    std::exception_ptr at_exp,vt_exp,rt_exp;
    XPlay2Widget_sptr w;
    //QSharedPointer<TestThread> t;
    QSharedPointer<XDemuxThread> de;

    int ret;
    try {
        w = XPlay2Widget::Handle();
        w->show();

//        t.reset(new TestThread());
//        t->xVideoWidget = w->m_ui->VideoWidget;
//        t->m_rt_ex = std::addressof(rt_exp);
//        t->at.SetException_ptr(std::addressof(at_exp));
//        t->vt.SetException_ptr(std::addressof(vt_exp));
//        t->init();
//        t->start();
        de.reset(new XDemuxThread());
        de->Open("2_audio.mp4",w->m_ui->VideoWidget);
        de->Start();
        ret = QApplication::exec();

//        if (at_exp){
//            std::rethrow_exception(at_exp);
//        }
//
//        if (vt_exp){
//            std::rethrow_exception(vt_exp);
//        }
//
//        if (rt_exp){
//            std::rethrow_exception(rt_exp);
//        }

        return ret;
    } catch (const std::exception &e) {
        w.reset();
//        t.reset();
        qDebug() << e.what();
        return -1;
    }
}
