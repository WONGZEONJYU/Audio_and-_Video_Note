#include <iostream>
#include <fstream>
#include <SDL.h>

#undef main
static void AudioCallback(void * const userdata, Uint8 * const stream,
                        const int len){
    std::fill_n(stream,len,0);
    const auto ifs{static_cast<std::ifstream*>(userdata)};
    ifs->read(reinterpret_cast<char*>(stream),len);
    if (ifs->gcount() <= 0){
        SDL_PauseAudio(1);
        std::cerr << "end\n";
    }
}

int main(const int argc,const char *argv[]) {

    (void )argc,(void )argv;
    SDL_Init(SDL_INIT_AUDIO);

    std::ifstream ifs("test_pcm.pcm",std::ios::binary);
    if (!ifs){
        std::cerr << "open pcm file failed\n";
        return -1;
    }

    SDL_AudioSpec spec{
        .freq = 44100,
        .format = AUDIO_S16SYS,
        .channels = 2,
        .silence = 0,
        .samples = 1152,
        .callback = AudioCallback,
        .userdata = &ifs,
    };

    if(SDL_OpenAudio(&spec,{}) < 0){
        std::cerr << "open audio failed\n";
        return -1;
    }

    SDL_PauseAudio(0);
    (void )getchar();
    SDL_QuitSubSystem(SDL_INIT_AUDIO);
    return 0;
}
