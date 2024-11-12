#include <iostream>
#include <fstream>
#include "xaudio_play.hpp"

int main() {
    const auto a{xAudio()};
    constexpr XAudioSpec spec;
    a->Open(spec);

    std::ifstream ifs("44100_16bit_2ch.pcm",std::ios::binary);
    if (!ifs) {
        std::cerr << "Error opening file\n";
        return -1;
    }

    a->set_volume(128);
    uint8_t buf[1024]{};
    a->set_speed(2.5);
    while (true) {
        ifs.read(reinterpret_cast<char*>(buf), sizeof(buf));
        const auto read_size{ifs.gcount()};
        if (ifs.eof()) {
            break;
        }
        a->Push(buf, read_size);
    }

    std::cerr << "play finish\n";
    getchar();
    return 0;
}
