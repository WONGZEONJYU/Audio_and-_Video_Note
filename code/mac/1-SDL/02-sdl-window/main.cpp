#include <iostream>
#include <string>
#include <SDL.h>
#undef main

using namespace std;

int main()
{
    SDL_Init(SDL_INIT_VIDEO);

    auto window {SDL_CreateWindow("2 Window",
                                 SDL_WINDOWPOS_CENTERED,
                                 SDL_WINDOWPOS_CENTERED,
                                 1280,800,
                                 SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE)};//创建窗口
    if(!window){
        throw (string("err: ") + SDL_GetError());
    }

    auto renderer {SDL_CreateRenderer(window, -1, 0)}; //基于窗口创建渲染器
    if(!renderer){
        throw (string("err: ") + SDL_GetError());
    }

    auto texture {SDL_CreateTexture(renderer,
                                    SDL_PIXELFORMAT_RGBA8888,
                                    SDL_TEXTUREACCESS_TARGET,
                                    1280,800)}; //创建纹理
    if(!texture){
        throw (string("err: ") + SDL_GetError());
    }

    int show_count{};

    for(;;){

        const SDL_Rect rect{.x = rand() % 1000,
                        .y = rand() % 700,.w = 100,.h = 100};

        SDL_SetRenderTarget(renderer,texture); //设置渲染目标为纹理
        SDL_SetRenderDrawColor(renderer,255,0,0,0); //纹理背景颜色(RGBA)
        SDL_RenderClear(renderer); //清屏

        SDL_RenderDrawRect(renderer, &rect); //绘制一个长方形
        SDL_SetRenderDrawColor(renderer, 0, 255, 255, 0); //设置长方形填充颜色(RGBA)
        SDL_RenderFillRect(renderer, &rect); /*填充长方形颜色*/

        SDL_SetRenderTarget(renderer, nullptr); //恢复默认，渲染目标为窗口
        SDL_RenderCopy(renderer, texture, nullptr, nullptr); //拷贝纹理到CPU

        SDL_RenderPresent(renderer); //输出到目标窗口上
        SDL_Delay(300);

        if(++show_count >= 30){
            break;
        }
    }

    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window); //销毁窗口
    SDL_Quit();
    return 0;
}
