#include <iostream>
#include <string>
#include <SDL2/SDL.h>

#undef main

using namespace std;

int main(int argc,const char* argv[])
{
    (void)argc,(void)argv;

    SDL_Init(SDL_INIT_VIDEO);
    auto window {SDL_CreateWindow("basic window",
                                 SDL_WINDOWPOS_CENTERED,
                                 SDL_WINDOWPOS_CENTERED,
                                 1280,800,
                                 SDL_WINDOW_RESIZABLE | SDL_WINDOW_OPENGL)};

    if(!window){
        throw (string("err") + SDL_GetError());
    }

    for (;;) {
        SDL_Event e{};
        SDL_WaitEventTimeout(&e,300);
        if(e.type == SDL_QUIT){
            break;
        }
    }

    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}

