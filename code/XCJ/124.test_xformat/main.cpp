extern "C"{
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
}

#include <iostream>
#include <thread>
#include "xdemux.hpp"
#include "xmux.hpp"
#include "xavpacket.hpp"

using namespace std::chrono;
using namespace std::this_thread;

int main(const int argc,const char *argv[]) {

    std::string usage{"124.test_xformat in_file out_file start_time(sec) end_time(sec)\n"};
    usage += "such as : 124.test_xformat in_file.mp4 out_file.mp4 10 20\n";
    std::cerr << usage;

    if (argc < 3){
        return -1;
    }

    const std::string in_file{argv[1]},out_file{argv[2]};
    int begin_sec{},end_sec{};

    if (argc > 3){
        begin_sec = std::atoi(argv[3]);
    }

    if (argc > 4){
        end_sec = std::atoi(argv[4]);
    }

////////////////////////////////////////////////////////打开媒体///////////////////////////////////////////////////////////////
    //constexpr auto url{GET_STR("v1080.mp4")};
    XDemux demux;
    auto demux_c{XDemux::Open(in_file)};
    demux.set_fmt_ctx(demux_c);

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////封装输出媒体文件/////////////////////////////////////////////////////////////////////////////
    //constexpr auto out_url{GET_STR(out.mp4)};
    XMux mux;
    auto mux_c{XMux::Open(out_file)};
    mux.set_fmt_ctx(mux_c);
    const auto mvs{mux_c->streams[mux.video_index()]};
    const auto mas{mux_c->streams[mux.audio_index()]};

    /**
     * 拷贝音视频流信息
     */
    if (demux.video_index() >= 0){
        demux.CopyParm(demux.video_index(),mvs->codecpar);
        mvs->time_base.den = demux.video_timebase().den;
        mvs->time_base.num = demux.video_timebase().num;
    }

    if (demux.audio_index() >= 0){
        demux.CopyParm(demux.audio_index(),mas->codecpar);
        mas->time_base.den = demux.audio_timebase().den;
        mas->time_base.num = demux.audio_timebase().num;
    }

    mux.WriteHead();

    int64_t video_begin_pts{},audio_begin_pts{},video_end_pts{};
    if (begin_sec > 0) {
        if (demux.video_index() >= 0 && demux.video_timebase().num > 0){
            // pts = sec / time_base
            // pts = sec / (num / den) = sec * (den / num)
            const auto t{static_cast<double >(demux.video_timebase().den) / static_cast<double >(demux.video_timebase().num)};
            video_begin_pts = static_cast<int64_t>(begin_sec * t);
            video_end_pts = static_cast<int64_t>(end_sec * t);
            demux.Seek(video_begin_pts,demux.video_index());
        }

        if (demux.audio_index() >= 0 && demux.audio_timebase().num > 0){
            const auto t{static_cast<double >(demux.audio_timebase().den) / static_cast<double >(demux.audio_timebase().num)};
            audio_begin_pts = static_cast<int64_t>(begin_sec * t);
        }
    }

#if 0
    //截取10 ~ 20秒之间的音视频 取多不取少
    //假定 9 11秒有关键帧 我们取第9秒
    auto begin_sec {10.0},end_sec{20.0};
    int64_t begin_pts{},end_pts{};
    //换算pts换成输入ic的pts,以视频流为准

    if (vs && vs->time_base.num > 0) {
        // pts = sec / time_base
        // pts = sec / (num / den) = sec * (den / num)
        const auto t{static_cast<double>(vs->time_base.den) / static_cast<double>(vs->time_base.num)}; // den分母 / num分子
        begin_pts = static_cast<int64_t>(begin_sec  * t);
        end_pts = static_cast<int64_t>(end_sec * t);
    }

    int64_t begin_audio_pts{};
    if (as && as->time_base.num > 0){
        const auto t{static_cast<double >(as->time_base.den) / static_cast<double >(as->time_base.num)};
        begin_audio_pts = static_cast<int64_t>(begin_sec * t);
    }

    /**
     * 向后关键帧
     */
    FF_ERR_OUT(av_seek_frame(ic,vs->index,begin_pts,
                             AVSEEK_FLAG_FRAME | AVSEEK_FLAG_BACKWARD),return -1);
#endif

    auto packet{new_XAVPacket()};
    int video_count{},audio_count{};
    double total_sec{};
    while (true) {
        if (!demux.Read(*packet)) {
            break;
        }

        if (video_end_pts > 0 &&
            packet->stream_index == demux.video_index() &&
            packet->pts > video_end_pts) {
            break;
        }

        if (demux.video_index() == packet->stream_index){
            mux.RescaleTime(*packet,video_begin_pts,demux.video_timebase());
            ++video_count;
            if (demux.video_timebase().den > 0){
                total_sec += static_cast<double >(packet->duration) * (static_cast<double >(demux.video_timebase().num) / static_cast<double >(demux.video_timebase().den));
            }
        }else if (demux.audio_index() == packet->stream_index){
            mux.RescaleTime(*packet,audio_begin_pts,demux.audio_timebase());
            ++audio_count;
        } else{}

#if 0
        auto in_stream{ic->streams[packet->stream_index]};
        AVStream *out_stream{};
        int64_t offset_pts{};

        if (packet->stream_index == vs->index){
            std::cerr << "video: packet.size: " << packet->size << "\t" <<
                      "packet.dts: " << packet->dts << "\t" <<
                      "packet.pts: " << packet->pts << "\n";

            //超过第20秒退出,只存10~20秒
            if (packet->pts > end_pts){
                packet->Reset();
                break;
            }

            //out_stream = ec->streams[packet->stream_index];
            //offset_pts = begin_pts;

        }else if (as && packet->stream_index == as->index){
            std::cerr << "audio: packet.size: " << packet->size << "\t" <<
                      "packet.dts: " << packet->dts << "\t" <<
                      "packet.pts: " << packet->pts << "\n";

            //out_stream = ec->streams[packet->stream_index];
            //offset_pts = begin_audio_pts;
        } else{}

        //写入前需重新计算pts dts duration
        //a * bq(输入的time_base) / cq(目标的time_base)

        packet->pts = av_rescale_q_rnd(packet->pts - offset_pts,in_stream->time_base,out_stream->time_base,
                                       static_cast<AVRounding>(AV_ROUND_NEAR_INF|AV_ROUND_PASS_MINMAX));

        packet->dts = av_rescale_q_rnd(packet->dts - offset_pts,in_stream->time_base,out_stream->time_base,
                                       static_cast<AVRounding>(AV_ROUND_NEAR_INF|AV_ROUND_PASS_MINMAX));

        packet->duration = av_rescale_q(packet->duration,in_stream->time_base,out_stream->time_base);
        packet->pos = -1;
        /**
         * 交错写入文件
         */
#endif
        mux.Write(*packet);
    }
    /**
     * 写入文件尾部信息
     */
    mux.WriteEnd();
    std::cerr << GET_STR(out_file:) << out_file << "\n";
    std::cerr << GET_STR(video_frames:) << video_count << "\n";
    std::cerr << GET_STR(audio_frames:) << audio_count << "\n";
    std::cerr << GET_STR(total_sec:) << total_sec << "\n";

    return 0;
}
