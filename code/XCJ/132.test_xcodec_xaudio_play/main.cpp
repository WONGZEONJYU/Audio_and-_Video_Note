#include <iostream>
#include <xdemuxtask.hpp>
#include <xdecodetask.hpp>
#include <xaudio_play.hpp>
#include <xavframe.hpp>

using namespace std;

int main() {

    const auto a{xAudio()};
    XDemuxTask demux_task;
    XDecodeTask decode_task;
    demux_task.set_next(&decode_task);
    decode_task.set_frame_cache(true);
    if (!demux_task.Open("v1080.mp4")) {
        return -1;
    }

    const auto ap{demux_task.CopyAudioParm()};
    if (!ap) {
        return -1;
    }

    if (!decode_task.Open(ap)) {
        return -1;
    }

    decode_task.set_stream_index(demux_task.audio_index());

    if (!a->Open(ap)){
        return -1;
    }

    a->set_speed(1.0);

    demux_task.Start();
    decode_task.Start();

    while (true){
        if (const auto frame{decode_task.CopyFrame()}){
            a->Push(*frame);
        }
    }
    getchar();
    return 0;
}
