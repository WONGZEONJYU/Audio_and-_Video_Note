#include <iostream>
#include <string>
#include <SDL.h>

using namespace std;

#undef main
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

    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
