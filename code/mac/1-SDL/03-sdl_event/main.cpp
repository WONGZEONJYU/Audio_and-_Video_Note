#include <iostream>
#include <string>
#include <SDL2/SDL.h>
#undef main

using namespace std;

static inline constexpr auto FF_QUIT_EVENT {SDL_USEREVENT + 2}; //用户自定义事件

int main()
{
    SDL_Init( SDL_INIT_EVERYTHING  );               // Initialize SDL2

    // Create an application window with the following settings:
    const auto window { SDL_CreateWindow(
                "An SDL2 window",                  // window title
                SDL_WINDOWPOS_UNDEFINED,           // initial x position
                SDL_WINDOWPOS_UNDEFINED,           // initial y position
                1280,                               // width, in pixels
                800,                               // height, in pixels
                SDL_WINDOW_SHOWN | SDL_WINDOW_BORDERLESS// flags - see below
                )};

    // Check that the window was successfully created
    if (!window){
        // In the case that the window could not be made...
        throw (string("Could not create window: ") + SDL_GetError());
    }

    /* We must call SDL_CreateRenderer in order for draw calls to affect this window. */
    const auto renderer {SDL_CreateRenderer(window, -1, 0)};
    if(!renderer){
        throw (string("Could not create Renderer: ") + SDL_GetError());
    }

    /* Select the color for drawing. It is set to red here. */
    SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);

    /* Clear the entire screen to our selected color. */
    SDL_RenderClear(renderer);

    /* Up until now everything was drawn behind the scenes.
       This will show the new, red contents of the window. */
    SDL_RenderPresent(renderer);

    for (;;){

        bool b_exit {};
        SDL_Event event{};

//        if(!SDL_PollEvent(&event)){
//            continue;
//        }

        SDL_WaitEvent(&event);
        switch (event.type){

        case SDL_KEYDOWN:/* 键盘事件 */
            switch (event.key.keysym.sym){
            case SDLK_a:
                cout << "key down a\n";
                break;
            case SDLK_s:
                cout << "key down s\n";
                break;
            case SDLK_d:
                cout << "key down d\n";
                break;
            case SDLK_w:
                cout << "key down w\n";
                break;
            case SDLK_q:{
                cout << "key down q and push quit event\n";
                SDL_Event event_q{};
                event_q.type = FF_QUIT_EVENT;
                SDL_PushEvent(&event_q);
                break;
            }
            default:
                cout << "key down 0x" << hex << event.key.keysym.sym << "\n";
                break;
            }
            break;
        case SDL_MOUSEBUTTONDOWN:			/* 鼠标按下事件 */
            if (event.button.button == SDL_BUTTON_LEFT) {
                cout << "mouse down left\n";
            }else if(event.button.button == SDL_BUTTON_RIGHT){
                cout << "mouse down right\n";
            }else{
                cout << "mouse down " << event.button.button << "\n";
            }
            break;
        case SDL_MOUSEMOTION:		/* 鼠标移动事件 */
            cout << "mouse movie (" << event.button.x << "," << event.button.y << ")\n";
            break;
        case FF_QUIT_EVENT:
            cout << "receive quit event\n";
            b_exit = true;
            break;
        }

        if(b_exit){
             break;
        }
    }

    //destory renderer
    SDL_DestroyRenderer(renderer);
    // Close and destroy the window
    SDL_DestroyWindow(window);
    // Clean up
    SDL_Quit();

    return 0;
}
