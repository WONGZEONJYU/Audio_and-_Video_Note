#include <iostream>
#include <vector>
#include <SDL.h>

#undef main

static inline constexpr auto Width{800},Height{600};

int main(const int argc,const char **argv) {

    SDL_Init(SDL_INIT_AUDIO | SDL_INIT_VIDEO);

    auto Window{SDL_CreateWindow("102.SDL_RGB",
                                 SDL_WINDOWPOS_CENTERED,
                                 SDL_WINDOWPOS_CENTERED,
                                 Width,
                                 Height,
                                 SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE)};
    /*创建窗口*/
    if (!Window) {
        std::cerr << __LINE__ << ": " << SDL_GetError() << "\n";
        return -1;
    }

    auto Renderer{SDL_CreateRenderer(Window,
                                     -1,
                                     SDL_RENDERER_ACCELERATED)};
    /*创建渲染器*/
    if (!Renderer) {
        SDL_DestroyWindow(Window);
        std::cerr << __LINE__ << ": " << SDL_GetError() << "\n";
        return -1;
    }

    auto Texture{SDL_CreateTexture(Renderer,
                                   SDL_PIXELFORMAT_ARGB8888,
                                   SDL_TEXTUREACCESS_STREAMING,
                                   Width,
                                   Height)};
    /*创建纹理*/
    if (!Texture) {
        SDL_DestroyRenderer(Renderer);
        SDL_DestroyWindow(Window);
        std::cerr << __LINE__ << ": " << SDL_GetError() << "\n";
        return -1;
    }

    std::vector<uint8_t> v(Width * Height * 4);
    uint32_t rgb[3]{0xff};
    int i{},count{255};
    SDL_Rect rect{0,0,Width,Height};

    while (true) {

        SDL_Event sdlEvent{};
        if (SDL_WaitEventTimeout(&sdlEvent,10) < 0){ //SDL事件
            SDL_DestroyRenderer(Renderer);
            SDL_DestroyWindow(Window);
            SDL_DestroyTexture(Texture);
            return -1;
        }else {
            if (sdlEvent.type == SDL_QUIT) {
                break;
            }
        }

        if (SDL_RenderClear(Renderer) < 0) { //清除渲染器
            std::cerr << __LINE__ << ": " << SDL_GetError() << "\n";
            SDL_DestroyRenderer(Renderer);
            SDL_DestroyWindow(Window);
            SDL_DestroyTexture(Texture);
            return -1;
        }

        if (!(--count)) {
            count = 255;
            rgb[i++] = 0;
            i %= std::size(rgb);
            rgb[i] = 0xff;
        }

        --rgb[i];

        v.clear();
        const auto dst_{v.data()};
        for (uint32_t h {}; h < Height; ++h) {
            const auto drift{ h * Width * 4 };
            for (uint32_t w {}; w < Width * 4; w += 4) {
                dst_[drift + w] = rgb[2];       //B
                dst_[drift + w + 1] = rgb[1];   //G
                dst_[drift + w + 2] = rgb[0];   //R
                dst_[drift + w + 3] = 0;        //A
            }
        }

        if (SDL_UpdateTexture(Texture, nullptr, dst_ ,Width * 4) < 0) { //更新纹理
            std::cerr << __LINE__ << ": " << SDL_GetError() << "\n";
            SDL_DestroyRenderer(Renderer);
            SDL_DestroyWindow(Window);
            SDL_DestroyTexture(Texture);
            return -1;
        }

        if (SDL_RenderCopy(Renderer,Texture, nullptr, &rect) < 0) { //纹理数据拷贝到渲染器
            std::cerr << __LINE__ << ": " << SDL_GetError() << "\n";
            SDL_DestroyRenderer(Renderer);
            SDL_DestroyWindow(Window);
            SDL_DestroyTexture(Texture);
            return -1;
        }

        SDL_RenderPresent(Renderer); //开始渲染
    }

    SDL_DestroyRenderer(Renderer);
    SDL_DestroyWindow(Window);
    SDL_DestroyTexture(Texture);
    return 0;
}
