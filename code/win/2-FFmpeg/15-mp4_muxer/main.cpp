#include <iostream>

//#include "Muxer_mp4.h"

//ffmpeg -i sound_in_sync_test.mp4 -pix_fmt yuv420p 720x576_yuv420p.yuv
//ffmpeg -i sound_in_sync_test.mp4 -vn -ar 44100 -ac 2 -f s16le 44100_2_s16le.pcm
// 执行文件  yuv文件 pcm文件 输出mp4文件

#include "Audio_Resample.h"

int main(const int argc,const char* const *argv) {
    (void )argc,(void )(argv);
   constexpr Audio_Resample_Params s {AV_SAMPLE_FMT_DBLP,
                                  AV_CHANNEL_LAYOUT_STEREO,
                            48000,
                                       AV_SAMPLE_FMT_S16,
                                      AV_CHANNEL_LAYOUT_STEREO,
                            44100};

    try {
        Audio_Resample::create(s);
    } catch (const std::exception &e) {
        std::cerr << e.what()  << "\n";
        return -1;
    }


    return 0;
}
