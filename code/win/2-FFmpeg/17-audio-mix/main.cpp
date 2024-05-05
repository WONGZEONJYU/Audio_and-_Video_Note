#include <iostream>
#include "Mixer.hpp"

int main(const int argc,const char **argv) {

    if(argc < 4){
        std::cerr << "usage in1.pcm in2.pcm out.pcm";
        return -1;
    }

    try {
        auto mixer{new_Mixer(argv)};
        mixer->exec();

    } catch (const std::exception &e) {
        std::cerr << e.what() << "\n";
        return -1;
    }

    return 0;
}
