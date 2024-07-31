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
    //QApplication a(argc, argv);

    XDemux x;
    XDecode vd,ad;
    XAVCodecParameters_sptr_container_sptr c;
    try {
        x.Open("2_audio.mp4");
        x.Open("2_audio.mp4");
        c = x.copy_ALLCodec_Parameters();
        vd.Open(c->at(2));
        ad.Open(c->at(0));

        while (true){
            auto p{x.Read()};
            if (!p){
                qDebug() << "read finish\n";
                ad.Send({});
                vd.Send({});

                while (true){

                    auto af{ad.Receive()};
                    auto vf{vd.Receive()};

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
                    auto f = ad.Receive();
                    if (f){
                        qDebug() << av_get_sample_fmt_name(static_cast<AVSampleFormat>(f->format));
                    } else{
                        break;
                    }
                }

            } else{
                vd.Send(p);
                while (true){
                    auto f = vd.Receive();
                    if (f) {
                        qDebug() << av_get_pix_fmt_name(static_cast<AVPixelFormat>(f->format));
                    } else{
                        break;
                    }
                }
            }
        }

        return 0;
        //XPlay2Widget::Handle()->show();
        //return QApplication::exec();
    } catch (const std::exception &e) {
        x.Close();
        ad.Close();
        vd.Close();
        c.reset();
        qDebug() << e.what();
        return -1;
    }
}
