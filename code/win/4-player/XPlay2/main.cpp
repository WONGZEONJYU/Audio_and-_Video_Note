extern "C" {
#include <libavutil/pixdesc.h>
}

#include <QApplication>
#include <QSurfaceFormat>
#include "XPlay2Widget.hpp"
#include "XDemux.hpp"
#include "XAVPacket.hpp"
#include "XAVFrame.hpp"
#include "XAVCodecParameters.hpp"
#include "XDecode.hpp"

int main(int argc, char *argv[]) {
    QApplication a(argc, argv);

//    XDemux x;
//    XDecode vd,ad;
//    XAVCodecParameters_sptr_container_sptr c;
//    XAVPacket_sptr p;
//    XAVFrame_sptr af,vf;
    try {
//        x.Open("2_audio.mp4");
//        x.Open("2_audio.mp4");
//        c = x.copy_ALLCodec_Parameters();
//        vd.Open(c->at(2));
//        ad.Open(c->at(0));
#if 0
        while (true) {
            p = x.Read();
            if (!p){
                qDebug() << "read finish\n";
                ad.Send({});
                vd.Send({});

                while (true){

                    af = ad.Receive();
                    vf = vd.Receive();

                    if (af){
                        qDebug() << av_get_sample_fmt_name(static_cast<AVSampleFormat>(af->format));
                    }

                    if (vf){
                        qDebug() << av_get_pix_fmt_name(static_cast<AVPixelFormat>(vf->format));
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
                        qDebug() << av_get_sample_fmt_name(static_cast<AVSampleFormat>(af->format));
                    } else{
                        break;
                    }
                }

            } else{
                vd.Send(p);
                while (true){
                    vf = vd.Receive();
                    if (vf) {
                        qDebug() << av_get_pix_fmt_name(static_cast<AVPixelFormat>(vf->format));
                    } else{
                        break;
                    }
                }
            }
        }
#endif
        //return 0;
        //XPlay2Widget::Handle()->show();
        auto w{XPlay2Widget::Handle()};
        w->show();

        auto ret{-1};
        ret = QApplication::exec();
        w.reset();
        return ret;
    } catch (const std::exception &e) {
//        x.Close();
//        ad.Close();
//        vd.Close();
//        c.reset();
//        p.reset();
//        af.reset();
//        vf.reset();
        qDebug() << e.what();
        return -1;
    }
}
