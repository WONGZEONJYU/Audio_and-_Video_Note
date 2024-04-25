#include <iostream>
#include "Muxer_mp4.h"

//ffmpeg -i sound_in_sync_test.mp4 -pix_fmt yuv420p 720x576_yuv420p.yuv
//ffmpeg -i sound_in_sync_test.mp4 -vn -ar 44100 -ac 2 -f s16le 44100_2_s16le.pcm
// 执行文件  yuv文件 pcm文件 输出mp4文件

#if 1

int main(const int argc,const char* const *argv)
{
    if (argc < 4){
        std::cerr << "usage : in.yuv in.pcm out.mp4\n";
        return -1;
    }

    try {
         auto mp4_muxer{new_Muxer_mp4(argv[1],argv[2],argv[3])};
        mp4_muxer->exec();
    }catch (const std::exception &e){
        std::cerr << e.what() << "\n";
    }

    std::cerr << "process normal exit\n";
    return 0;
}
#else

#include "Audio_Resample.h"
#include "SwrContext_t.h"
#include "VideoEncoder.h"
#include "AudioEncoder.h"
int main(const int argc,const char* const *argv) {
    (void )argc,(void )(argv);

    if (argc < 3){
        std::cerr << "usage -> in.pcm out.pcm\n";
        return -1;
    }

   std::ifstream in_pcm_file(argv[1],std::ios::binary);

    if (!in_pcm_file){
        std::cerr << "open in_pcm_file failed\n";
        return -1;
    }

    std::ofstream out_pcm_file(argv[2] , std::ios::binary);
    if (!out_pcm_file){
        in_pcm_file.close();
        std::cerr << "open out_pcm_file failed\n";
        return -1;
    }

    try {
        const Audio_Resample_Params s;
        auto audio_resample = Audio_Resample::create(s);
        auto swr{SwrContext_t::create(
                &s.m_dst_ch_layout,
                s.m_dst_sample_fmt,
                s.m_dst_sample_rate,
                &s.m_src_ch_layout,
                s.m_src_sample_fmt,
                s.m_src_sample_rate
                )};
        const auto s16_pcm_buffer_size {av_get_bytes_per_sample(s.m_src_sample_fmt) *
        1024 * s.m_src_ch_layout.nb_channels};
        auto s16_pcm_buffer{new uint8_t[s16_pcm_buffer_size]{0}};
        int64_t src_pts{};

        for(;;){

            std::fill_n(s16_pcm_buffer,s16_pcm_buffer_size,0);
            in_pcm_file.read(reinterpret_cast< char*>(s16_pcm_buffer),s16_pcm_buffer_size);
            if (in_pcm_file.eof()){
                break;
            }
            const auto read_size{in_pcm_file.gcount()};
            audio_resample->send_frame(s16_pcm_buffer,read_size,src_pts);
            auto frame {audio_resample->receive_frame(0)};

            std::cerr << "fltp pts: " << frame->m_frame->pts << "\n";

            const auto data_size = av_get_bytes_per_sample(static_cast<AVSampleFormat>(frame->m_frame->format));

            for (int i {};i < frame->m_frame->nb_samples;i++){
                for (int ch{};ch < frame->m_frame->ch_layout.nb_channels;ch++) {
                    out_pcm_file.write(reinterpret_cast<const char*>(frame->m_frame->data[ch] + data_size * i), data_size);
                }
            }

            src_pts += frame->m_frame->nb_samples;
        }

        delete [] s16_pcm_buffer;
        in_pcm_file.close();
        out_pcm_file.close();

    } catch (const std::exception &e) {
        std::cerr << e.what()  << "\n";
        return -1;
    }

    return 0;
}

#endif