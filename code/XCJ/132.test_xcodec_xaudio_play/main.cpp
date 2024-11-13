#include <iostream>
#include <xdemuxtask.hpp>
#include <xdecodetask.hpp>
#include <xaudio_play.hpp>

using namespace std;

int main() {

    const auto a{xAudio()};
    XDemuxTask demux_task;
    XDecodeTask decode_task;

    if (!demux_task.Open("")) {
        return -1;
    }

    const auto ap{demux_task.CopyAudioParm()};
    if (!ap) {
        return -1;
    }

    if (decode_task.Open(ap)) {
        return -1;
    }






    return 0;
}
