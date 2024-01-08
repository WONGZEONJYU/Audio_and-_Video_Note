#include <iostream>
#include <string>
#include <filesystem>
#include <memory_resource>
#include <SDL.h>

#undef main

using namespace std;

//自定义消息类型
constexpr auto REFRESH_EVENT  {SDL_USEREVENT + 1};     // 请求画面刷新事件
constexpr auto QUIT_EVENT  {SDL_USEREVENT + 2};    // 退出事件

//定义分辨率
// YUV像素分辨率
constexpr uint32_t YUV_WIDTH {320};
constexpr uint32_t YUV_HEIGHT  {240};
//定义YUV格式
constexpr auto YUV_FORMAT {SDL_PIXELFORMAT_IYUV};

int s_thread_exit {};  // 退出标志 = 1则退出

int refresh_video_timer(void *data)
{
    (void)data;
    while (!s_thread_exit){
        SDL_Event event;
        event.type = REFRESH_EVENT;
        SDL_PushEvent(&event);
        SDL_Delay(50);
    }

    s_thread_exit = 0;

    //push quit event
    SDL_Event event;
    event.type = QUIT_EVENT;
    SDL_PushEvent(&event);
    return 0;
}

int main()
{
    //初始化 SDL
    if(SDL_Init(SDL_INIT_VIDEO)){
        throw string("Could not initialize SDL - ") + SDL_GetError();
    }

    // 分辨率
    // 1. YUV的分辨率
    constexpr auto video_width {YUV_WIDTH},
                        video_height {YUV_HEIGHT};

    // 2.显示窗口的分辨率
    auto win_width {YUV_WIDTH},win_height {YUV_WIDTH};
    //创建窗口
    auto window { SDL_CreateWindow("Simplest YUV Player",
                           SDL_WINDOWPOS_UNDEFINED,
                           SDL_WINDOWPOS_UNDEFINED,
                           video_width, video_height,
                           SDL_WINDOW_OPENGL|SDL_WINDOW_RESIZABLE)};
    if(!window){
        throw string("SDL: could not create window, err : ") + SDL_GetError() + "\n";
    }

    // 基于窗口创建渲染器
    auto renderer {SDL_CreateRenderer(window, -1, 0)};
    if(!renderer){
        SDL_DestroyWindow(window);
        throw string("SDL: could not create Renderer, err : ") + SDL_GetError() + "\n";
    }

    // 基于渲染器创建纹理
    auto texture { SDL_CreateTexture(renderer,
                          YUV_FORMAT, // YUV420P，即是SDL_PIXELFORMAT_IYUV
                          SDL_TEXTUREACCESS_STREAMING,
                          video_width,
                          video_height)};

    if(!texture){
        SDL_DestroyWindow(window);
        SDL_DestroyRenderer(renderer);
        throw string("SDL: could not create Renderer, err : ") + SDL_GetError() + "\n";
    }

    // 分配空间
    std::pmr::pool_options opt{};
    std::pmr::synchronized_pool_resource mpool(opt);

    // 我们测试的文件是YUV420P格式
    constexpr auto y_frame_len {video_width * video_height},
    u_frame_len {y_frame_len / 4},v_frame_len {y_frame_len / 4},
    yuv_frame_len {y_frame_len + u_frame_len + v_frame_len};

    void *video_buf{};
    try {
        video_buf = mpool.allocate(yuv_frame_len);
    } catch (const string &e) {
        SDL_DestroyWindow(window);
        SDL_DestroyRenderer(renderer);
        SDL_DestroyTexture(texture);
        throw e + " Failed to alloce yuv frame space!\n";
    }

    // 打开YUV文件
    constexpr auto yuv_path {"yuv420p_320x240.yuv"};
    auto video_fd { fopen(yuv_path, "rb")};
    if(!video_fd ){
        SDL_DestroyWindow(window);
        SDL_DestroyRenderer(renderer);
        SDL_DestroyTexture(texture);
        mpool.release();
        cerr << "Failed to open yuv file\n";
        throw "Failed to open yuv file\n";
    }

    // 创建请求刷新线程
    auto timer_thread { SDL_CreateThread(refresh_video_timer, nullptr, nullptr)};

    for(;;){

        // 收取SDL系统里面的事件
        SDL_Event event{};
        SDL_WaitEvent(&event);

        if(event.type == REFRESH_EVENT){ // 画面刷新事件

            auto video_buff_len {fread(video_buf, 1, yuv_frame_len, video_fd)}; /*一次读取一帧*/
            if(video_buff_len <= 0){
                cerr << "Failed to read data from yuv file!\n";
                goto _FAIL;
            }
            // 设置纹理的数据 video_width = 320, plane
            /*采用planar格式存储,4个Y,对应一个U和一个V*/
            SDL_UpdateTexture(texture, nullptr, video_buf, video_width);

            // 显示区域,可以通过修改w和h进行缩放,支持拉伸操作
#if 1
            const auto w_ratio {win_width * 1.0 /video_width},
            h_ratio {win_height * 1.0 /video_height};
            SDL_Rect rect{.x = 0,.y = 0,
                        .w = static_cast<int>(video_width * w_ratio),
                        .h = static_cast<int>(video_height * h_ratio)};

            //            rect.w = video_width * 0.5;
            //            rect.h = video_height * 0.5;
#else
            /* 保持视频320x240分辨率 , 拉动窗口视频始终在窗口的正中心 */
            SDL_Rect rect{.x = static_cast<int>((win_width - video_width)/2) ,
                        .y = static_cast<int>((win_height - video_height)/2) ,
                        .w = video_width ,
                        .h = video_height };
#endif
            // 清除当前显示
            SDL_RenderClear(renderer);
            // 将纹理的数据拷贝给渲染器
            SDL_RenderCopy(renderer, texture, nullptr, &rect);
            // 显示
            SDL_RenderPresent(renderer);
        }else if(SDL_WINDOWEVENT == event.type){
            //If Resize
            auto w{reinterpret_cast<int*>(&win_width)},
                h{reinterpret_cast<int*>(&win_height)};
            SDL_GetWindowSize(window, w, h);/*支持拉伸操作*/
            cout << "SDL_WINDOWEVENT win_width : " << *w << ", win_height: " << *h << "\n";
        }else if(SDL_QUIT == event.type){ //退出事件
            s_thread_exit = 1;
        }else if(QUIT_EVENT == event.type){
            break;
        }else{}
    }

_FAIL:
    s_thread_exit = 1;      // 保证线程能够退出
    // 释放资源
    if(timer_thread){
        SDL_WaitThread(timer_thread, nullptr); // 等待线程退出
    }

    mpool.release();

    if(video_fd){
        fclose(video_fd);
    }

    if(texture){
        SDL_DestroyTexture(texture);
    }

    if(renderer){
        SDL_DestroyRenderer(renderer);
    }

    if(window){
        SDL_DestroyWindow(window);
    }

    SDL_Quit();

    return 0;
}
