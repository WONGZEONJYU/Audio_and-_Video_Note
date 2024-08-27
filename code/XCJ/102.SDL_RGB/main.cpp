#include <iostream>
#include <vector>
#include <thread>
#include <SDL.h>

#undef main

static inline constexpr auto Width{800},Height{600};

int main(const int argc,const char **argv) {

    SDL_Init(SDL_INIT_AUDIO | SDL_INIT_VIDEO);

    auto Window{SDL_CreateWindow("102.SDL_RGB", 0, 0, Width, Height, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE)};
    if (!Window) {
        std::cerr << __LINE__ << ": " << SDL_GetError() << "\n";
        return -1;
    }

    auto Renderer{SDL_CreateRenderer(Window,-1, SDL_RENDERER_ACCELERATED)}; //创建渲染器
    if (!Renderer) {
        std::cerr << __LINE__ << ": " << SDL_GetError() << "\n";
        return -1;
    }

    auto Texture{SDL_CreateTexture(Renderer,SDL_PIXELFORMAT_RGBA8888,SDL_TEXTUREACCESS_STREAMING,Width,Height)};
    if (!Texture){
        std::cerr << __LINE__ << ": " << SDL_GetError() << "\n";
        return -1;
    }

    if (SDL_RenderClear(Renderer) < 0){
        std::cerr << __LINE__ << ": " << SDL_GetError() << "\n";
        return -1;
    }

    std::vector<uint8_t> v(Width * 4 * Height);

    const auto dst_{v.data()};

    for (uint32_t h {}; h < Height; ++h) {
        const auto drift{ h * Width * 4 };
        for (uint32_t w {}; w < Width * 4; w += 4) {
            dst_[drift + w] = 0xff;
            dst_[drift + w + 1] = 0;
            dst_[drift + w + 2] = 0;
            dst_[drift + w + 3] = 0x7f;
        }
    }

    if (SDL_UpdateTexture(Texture, nullptr,dst_,Width * 4 ) < 0){
        std::cerr << __LINE__ << ": " << SDL_GetError() << "\n";
        return -1;
    }

    if(SDL_RenderCopy(Renderer,Texture, nullptr, nullptr) < 0){
        std::cerr << __LINE__ << ": " << SDL_GetError() << "\n";
        return -1;
    }

    SDL_RenderPresent(Renderer);

    getchar();

    SDL_DestroyRenderer(Renderer);

    return 0;
}
