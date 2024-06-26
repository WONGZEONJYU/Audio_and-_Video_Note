# 1.SDL简介

[[官网]](https://www.libsdl.org/)	[[文档]](http://wiki.libsdl.org/Introduction  )

SDL（Simple DirectMedia Layer）是一套开放源代码的跨平台多媒体开发库 , 使用C语言写成。 SDL提供了数种控制图像、声音、输出入的函数 , 让开发者只要用相同或是相似的代码就可以开发出跨多个平台 (Linux、 Windows、 Mac OS X等) 的应用软件。目前SDL多用于开发游戏、模拟器、 媒体播放器等多媒体应用领域

对于我们课程而言 : SDL主要用来辅助学习FFmpeg , 所以我们只会关注我们用到的知识点

<img src="assets/image-20240104094407419.png" alt="image-20240104094407419" /> 

## 1.1 Windows环境搭建

[[下载地址]](https://www.libsdl.org/download-2.0.php)

<img src="assets/image-20240104094551340.png" alt="image-20240104094551340" /> 

 版本会持续更新 , 下载对应的版本就OK

## 1.2 Linux环境搭建

### 1.2.1 下载SDL源码库 , SDL2-x.x.x.tar.gz

[[下载地址]](https://www.libsdl.org/download-2.0.php)

<img src="assets/image-20240104094754989.png" alt="image-20240104094754989" /> 

### 1.2.2 解压

> ```bash
> tar -vxf SDL2-x.x.x.tar.gz
> ```

### 1.2.3 配置 & 编译

> ```bash
> ./configure
> make
> sudo make install
> ```

### 1.2.4 遇到错误

> ```tex
> 如果出现Could not initialize SDL - No available video device
> (Did you set the DISPLAY variable?)错误
> 说明系统中没有安装x11的库文件，因此编译出来的SDL库实际上不能用。
> 下载安装
> ```

> ```bash
> sudo apt-get install libx11-dev
> sudo apt-get install xorg-dev
> ```

## 1.3 Mac 搭建

### 1.3.1 下载搭建

[[下载地址]](https://www.libsdl.org/download-2.0.php)

<img src="assets/image-20240104101849453.png" alt="image-20240104101849453" />

下载后打开安装包将 SDL2.framework 拖拽到 /Library/Frameworks这个目录下

> ```tex
> 这种是直接是使用预编译好的二进制文件,无需进行编译操作,与下的第一条命令搭建是一样的
> ```

### 1.3.2 命令搭建

> ```bash
> brew install SDL2
> #这是使用 Homebrew 默认的方式来安装 SDL2。这将会从 Homebrew 的二进制存储库中下载预编译的二进制文件,并将其安装到系统中。通常情况下,这种方式比较快速,因为它直接使用了预编译好的二进制文件,无需进行编译操作
> 
> brew install --build-from-source sdl2
> #这个命令指示 Homebrew 在安装SDL2时从源代码构建。这意味着 Homebrew 将下载 SDL2的源代码并在本地编译,然后将其安装在系统中。这种方式需要更多的时间和计算资源,但具有更大的灵活性,因为可以在编译过程中进行自定义设置和调整
> ```

## 1.4 新建开发工程(qt)

### 1.4.1 windows 下

#### 1.4.1.1 新建工程

<img src="assets/image-20240104111120569.png" alt="image-20240104111120569" /> 

路径任君选择 , 不能有中文路径

<img src="assets/image-20240104111215746.png" alt="image-20240104111215746" /> 

<img src="assets/image-20240104111313608.png" alt="image-20240104111313608" /> 

教程采用32bit开发 , 本人尝试使用64bit开发

#### 1.4.1.2 拷贝SDL库到工程目录

<img src="assets/image-20240104111702843.png" alt="image-20240104111702843" /> 

 <img src="assets/image-20240104141717961.png" alt="image-20240104141717961" /> 

#### 1.4.1.3 修改 `pro` 文件

> ```bash
> TEMPLATE = app
> CONFIG += console c++17
> CONFIG -= app_bundle
> CONFIG -= qt
> 
> SOURCES += \
>         main.cpp
> 
> INCLUDEPATH += $$PWD/../SDL2-2.28.5-VC/include
> LIBS += $$PWD/../SDL2-2.28.5-VC/lib/x64/SDL2.lib
> #把库文件路径与头文件路径加进去,库文件路径需要指定到具体某个库
> 
> CONFIG += shadow -build
> DESTDIR = $$PWD/bin
> #CONFIG += shadow 用于告诉 qmake 使用生成的中间文件目录作为构建目录
> #DESTDIR = $$PWD/bin 把输出目录调整到工程文件夹下的bin文件夹
> ```

#### 1.4.1.4 main.cpp包含头文件并测试

> ```c++
> #include <iostream>
> #include <SDL.h>
> 
> using namespace std;
> 
> #undef main //必须加上此宏定义,由于在SDL_main.h定义了#define main SDL_main
> int main(int argc,const char* argv[])
> {
>     SDL_version ver{};
>     SDL_GetVersion(&ver);
> 
>     cout << dec << static_cast<int>(ver.major) << "." <<
>             static_cast<int>(ver.minor) << "." <<
>             static_cast<int>(ver.patch) << "\n";
> 
>     return 0;
> }
> ```

<img src="assets/image-20240104133811110.png" alt="image-20240104133811110" /> 

### 1.4.2 linux 下

### 1.4.3 Macos 下

# 2. SDL 编程

## 2.1 SDL子系统

SDL将功能分成下列数个子系统 (subsystem) : 

* 👉 **SDL_INIT_TIMER : 定时器**
* 👉 **SDL_INIT_AUDIO : 音频**
* 👉 **SDL_INIT_VIDEO : 视频**
* SDL_INIT_JOYSTICK : 摇杆
* SDL_INIT_HAPTIC : 触摸屏
* SDL_INIT_GAMECONTROLLER : 游戏控制器
* 👉 **SDL_INIT_EVENTS : 事件**
* SDL_INIT_EVERYTHING : 包含上述所有选项 

## 2.2 Window显示 

### 2.2.1 SDL视频显示函数简介

* `SDL_Init()` : 初始化SDL系统
* `SDL_CreateWindow()` : 创建窗口SDL_Window
* `SDL_CreateRenderer()` : 创建渲染器SDL_Renderer
* `SDL_CreateTexture()` : 创建纹理SDL_Texture
* `SDL_UpdateTexture()` : 设置纹理的数据
* `SDL_RenderCopy()` : 将纹理的数据拷贝给渲染器
* `SDL_RenderPresent()` : 显示
* `SDL_Delay()` : 工具函数 , 用于延时
* `SDL_Quit()` : 退出SDL系统  

#### 2.2.1.1 编程实验

[[01-sdl-basic参考链接]](/code/win/1-SDL/01-sdl-basic)

> ```c++
> #include <iostream>
> #include <string>
> #include <SDL.h>
> 
> #undef main	/*必须在#include <SDL.h>下*/
> using namespace std;
> 
> int main(int argc,const char* argv[])
> {
>     (void)argc,(void)argv;
> 
>     SDL_Init(SDL_INIT_VIDEO);
>     auto window {SDL_CreateWindow("basic window",
>                                  SDL_WINDOWPOS_CENTERED,
>                                  SDL_WINDOWPOS_CENTERED,
>                                  1280,800,
>                                  SDL_WINDOW_RESIZABLE | SDL_WINDOW_OPENGL)};
> 
>     if(!window){
>         throw (string("err") + SDL_GetError());
>     }
> 	SDL_Delay(10000);
>     SDL_DestroyWindow(window);
>     SDL_Quit();
>     return 0;
> }
> ```

<img src="assets/image-20240104204925735.png" alt="image-20240104204925735" /> 

### 2.2.2 SDL数据结构简介

* `SDL_Window` 代表了一个 “窗口”
* `SDL_Renderer` 代表了一个 “渲染器”
* `SDL_Texture` 代表了一个 “纹理”
* `SDL_Rect` 一个简单的矩形结构  

> ```tex
> 存储RGB和存储纹理的区别:
> 比如一个从左到右由红色渐变到蓝色的矩形,用存储RGB的话就需要把矩形中每个点的具体颜色值存储下来;而纹理只是一些描述信息,比如记录了矩形的大小、起始颜色、终止颜色等信息,显卡可以通过这些信息推算出矩形块的详细信息。
> 所以相对于存储RGB而已,存储纹理占用的内存要少的多。
> ```

<img src="assets/image-20240105152547759.png" alt="image-20240105152547759" /> 

#### 2.2.2.1 编程实验

[[02-sdl-window参考链接]](/code/win/1-SDL/02-sdl-window)

1. pro文件

> ```bash
> TEMPLATE = app
> CONFIG += console c++20
> CONFIG -= app_bundle
> CONFIG -= qt
> 
> SOURCES += \
>         main.cpp
> 
> INCLUDEPATH += $$PWD/../SDL2-2.28.5-VC/include
> LIBS += $$PWD/../SDL2-2.28.5-VC/lib/x64/SDL2.lib
> 
> CONFIG += shadow -build
> DESTDIR = $$PWD/bin
> ```

2. main.cpp

> ```c++
> #include <iostream>
> #include <string>
> #include <SDL.h>
> #undef main
> 
> using namespace std;
> 
> int main()
> {
>        SDL_Init(SDL_INIT_VIDEO);
> 
>        auto window {SDL_CreateWindow("2 Window",
>                                     SDL_WINDOWPOS_CENTERED,
>                                     SDL_WINDOWPOS_CENTERED,
>                                     1280,800,
>                                     SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE)};//创建窗口
>        if(!window){
>            throw (string("err: ") + SDL_GetError());
>        }
> 
>        auto renderer {SDL_CreateRenderer(window, -1, 0)}; //基于窗口创建渲染器
>        if(!renderer){
>            throw (string("err: ") + SDL_GetError());
>        }
> 
>        auto texture {SDL_CreateTexture(renderer,
>                                        SDL_PIXELFORMAT_RGBA8888,
>                                        SDL_TEXTUREACCESS_TARGET,
>                                        1280,800)}; //创建纹理
>        if(!texture){
>            throw (string("err: ") + SDL_GetError());
>        }
> 
>        int show_count{};
> 
>        for(;;){
> 
>            const SDL_Rect rect{.x = rand() % 1000,
>                            .y = rand() % 700,.w = 100,.h = 100};
> 
>            SDL_SetRenderTarget(renderer,texture); //设置渲染目标为纹理
>            SDL_SetRenderDrawColor(renderer,255,0,0,0); //设置纹理背景颜色(RGBA)
>            SDL_RenderClear(renderer); //清屏
> 
>            SDL_RenderDrawRect(renderer, &rect); //绘制一个长方形
>            SDL_SetRenderDrawColor(renderer, 0, 255, 255, 0); //设置长方形填充颜色(RGBA)
>            SDL_RenderFillRect(renderer, &rect); /*填充长方形颜色*/
> 
>            SDL_SetRenderTarget(renderer, nullptr); //恢复默认，渲染目标为窗口
>            SDL_RenderCopy(renderer, texture, nullptr, nullptr); //拷贝纹理到CPU
> 
>            SDL_RenderPresent(renderer); //输出到目标窗口上
>            SDL_Delay(300);
> 
>            if(++show_count >= 30){
>                break;
>            }
>        }
> 
>        SDL_DestroyTexture(texture);
>        SDL_DestroyRenderer(renderer);
>        SDL_DestroyWindow(window); //销毁窗口
>        SDL_Quit();
>        return 0;
> }
> ```

<img src="assets/image-20240105095202600.png" alt="image-20240105095202600" />

 <img src="assets/image-20240105095230643.png" alt="image-20240105095230643" /> 

## 2.3 SDL事件

* 函数
  * `SDL_WaitEvent()` : 等待一个事件(阻塞)
  * `SDL_WaitEventTimeout()` : 等待一个事件(阻塞,带超时)
  * `SDL_PollEvent()` : 轮询是否有事件 , 没有立即返回 (非阻塞)
  * `SDL_PumpEvents()` : 
    * 将硬件设备产生的事件放入事件队列 , 用于读取事件 , 在调用上述几个函数之前 , 必须调用 `SDL_PumpEvents` 搜集键盘等事件 ( **简单理解就是强制更新事件队列** )
    * 一般情况下 , 上述三个函数内部会自动调用 `SDL_PumpEvents()` , 用户无需显示调用本函数
  * `SDL_PushEvent()` : 发送一个事件
  * `SDL_PeepEvents()` : 从事件队列提取一个事件 ( 调用前建议先调用 `SDL_PumpEvents()` 来更新一下事件队列 )
* 数据结构
  * `SDL_Event` : 代表一个事件  

### 2.3.1 编程实验

[[03-sdl-event参考链接]](/code/win/1-SDL/03-sdl_event)

1. `pro` 文件

> ```bash
> TEMPLATE = app
> CONFIG += console c++20
> CONFIG -= app_bundle
> CONFIG -= qt
> 
> SOURCES += \
>         main.cpp
> 
> INCLUDEPATH += $$PWD/../SDL2-2.28.5-VC/include
> LIBS += $$PWD/../SDL2-2.28.5-VC/lib/x64/SDL2.lib
> 
> CONFIG += shadow -build
> DESTDIR = $$PWD/bin
> 
> ```

2. main.cpp

> ```c++
> #include <iostream>
> #include <string>
> #include <SDL.h>
> #undef main
> 
> using namespace std;
> 
> static inline constexpr auto FF_QUIT_EVENT {SDL_USEREVENT + 2}; //用户自定义事件
> 
> int main()
> {
>     SDL_Init( SDL_INIT_EVERYTHING  );               // Initialize SDL2
> 
>     // Create an application window with the following settings:
>     const auto window { SDL_CreateWindow(
>                 "An SDL2 window",                  // window title
>                 SDL_WINDOWPOS_UNDEFINED,           // initial x position
>                 SDL_WINDOWPOS_UNDEFINED,           // initial y position
>                 1280,                               // width, in pixels
>                 800,                               // height, in pixels
>                 SDL_WINDOW_SHOWN | SDL_WINDOW_BORDERLESS// flags - see below
>                 )};
> 
>     // Check that the window was successfully created
>     if (!window){
>         // In the case that the window could not be made...
>         throw (string("Could not create window: ") + SDL_GetError());
>     }
> 
>     /* We must call SDL_CreateRenderer in order for draw calls to affect this window. */
>     const auto renderer {SDL_CreateRenderer(window, -1, 0)};
>     if(!renderer){
>         throw (string("Could not create Renderer: ") + SDL_GetError());
>     }
> 
>     /* Select the color for drawing. It is set to red here. */
>     SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
> 
>     /* Clear the entire screen to our selected color. */
>     SDL_RenderClear(renderer);
> 
>     /* Up until now everything was drawn behind the scenes.
>        This will show the new, red contents of the window. */
>     SDL_RenderPresent(renderer);
> 
>     for (;;){
> 
>         bool b_exit {};
>         SDL_Event event{};
> 
> //        if(!SDL_PollEvent(&event)){
> //            continue;
> //        }
> 
>         SDL_WaitEvent(&event);
>         switch (event.type){
> 
>         case SDL_KEYDOWN:/* 键盘事件 */
>             switch (event.key.keysym.sym){
>             case SDLK_a:
>                 cout << "key down a\n";
>                 break;
>             case SDLK_s:
>                 cout << "key down s\n";
>                 break;
>             case SDLK_d:
>                 cout << "key down d\n";
>                 break;
>             case SDLK_w:
>                 cout << "key down w\n";
>                 break;
>             case SDLK_q:{
>                 cout << "key down q and push quit event\n";
>                 SDL_Event event_q{};
>                 event_q.type = FF_QUIT_EVENT;
>                 SDL_PushEvent(&event_q);
>                 break;
>             }
>             default:
>                 cout << "key down 0x" << hex << event.key.keysym.sym << "\n";
>                 break;
>             }
>             break;
>         case SDL_MOUSEBUTTONDOWN:			/* 鼠标按下事件 */
>             if (event.button.button == SDL_BUTTON_LEFT) {
>                 cout << "mouse down left\n";
>             }else if(event.button.button == SDL_BUTTON_RIGHT){
>                 cout << "mouse down right\n";
>             }else{
>                 cout << "mouse down " << event.button.button << "\n";
>             }
>             break;
>         case SDL_MOUSEMOTION:		/* 鼠标移动事件 */
>             cout << "mouse movie (" << event.button.x << "," << event.button.y << ")\n";
>             break;
>         case FF_QUIT_EVENT:
>             cout << "receive quit event\n";
>             b_exit = true;
>             break;
>         }
> 
>         if(b_exit){
>              break;
>         }
>     }
> 
>     //destory renderer
>     SDL_DestroyRenderer(renderer);
>     // Close and destroy the window
>     SDL_DestroyWindow(window);
>     // Clean up
>     SDL_Quit();
> 
>     return 0;
> }
> ```

<img src="assets/image-20240105134535778.png" alt="image-20240105134535778" /> 

<img src="assets/image-20240105134601217.png" alt="image-20240105134601217" /> 

## 2.4 SDL多线程

* SDL线程创建 : `SDL_CreateThread()`
* SDL线程等待 : `SDL_WaitThead()`
* SDL互斥锁创建/销毁 : `SDL_CreateMutex() / SDL_DestroyMutex()`
* SDL锁定互斥 : `SDL_LockMutex() / SDL_UnlockMutex()`
* SDL条件变量 (信号量) 创建/销毁 : `SDL_CreateCond() / SDL_DestoryCond()`
* SDL条件变量 (信号量) 等待/通知 : `SDL_CondWait() / SDL_CondSingal()`

### 2.4.1 编程实验

[[04-sdl-thread参考链接]](/code/win/1-SDL/04-sdl-thread)

1. `pro` 文件

> ```bash
> TEMPLATE = app
> CONFIG += console c++20
> CONFIG -= app_bundle
> CONFIG -= qt
> 
> SOURCES += \
>         main.cpp
> 
> INCLUDEPATH += $$PWD/../SDL2-2.28.5-VC/include
> LIBS += $$PWD/../SDL2-2.28.5-VC/lib/x64/SDL2.lib
> 
> CONFIG += shadow -build
> DESTDIR = $$PWD/bin
> ```

2. main.cpp

> ```c++
> #include <iostream>
> #include <string>
> #include <thread>
> #include <SDL.h>
> 
> #undef main
> using namespace std;
> using namespace chrono;
> using namespace this_thread;
> 
> SDL_mutex *s_lock {};
> SDL_cond *s_cond {};
> 
> int thread_work(void *arg)
> {
>        (void)arg;
>        SDL_LockMutex(s_lock);
>        cout << "                <============thread_work sleep\n";
>        sleep_for(10s); // 用来测试获取锁
>        cout << "                <============thread_work wait\n";
> 
>        // 释放s_lock资源,并等待signal。之所以释放s_lock是让别的线程能够获取到s_lock
>        SDL_CondWait(s_cond, s_lock); //另一个线程(1)发送signal和(2)释放lock后,这个函数退出
> 
>        cout << "                <===========thread_work receive signal, continue to do ~_~!!!\n";
>        cout << "                <===========thread_work end\n";
>        SDL_UnlockMutex(s_lock);
>        return 0;
> }
> 
> int main()
> {
>        s_lock = SDL_CreateMutex();
>        s_cond = SDL_CreateCond();
> 
>        auto t {SDL_CreateThread(thread_work,"thread_work",nullptr)};
> 
>        if(!t) {
>            throw string(SDL_GetError());
>        }
> 
>        for(int i {};i< 2;i++){
>            sleep_for(2s);
>            cout << "main execute =====>\n";
>        }
> 
>        cout << "main SDL_LockMutex(s_lock) before ====================>\n";
> 
>        SDL_LockMutex(s_lock);  // 获取锁,但是子线程还拿着锁
>        cout << "main ready send signal====================>\n";
>        cout << "main SDL_CondSignal(s_cond) before ====================>\n";
> 
>        SDL_CondSignal(s_cond); // 发送信号,唤醒等待的线程
>        cout << "main SDL_CondSignal(s_cond) after ====================>\n";
> 
>        sleep_for(10s);
>        SDL_UnlockMutex(s_lock);// 释放锁,让其他线程可以拿到锁
>        cout << "main SDL_UnlockMutex(s_lock) after ====================>\n";
> 
>        SDL_WaitThread(t, nullptr);
>        SDL_DestroyMutex(s_lock);
>        SDL_DestroyCond(s_cond);
> 
>        return 0;
> }
> ```

<img src="assets/image-20240105160048854.png" alt="image-20240105160048854" />  

## 2.5 YUV显示 : SDL视频显示的流程

<img src="assets/image-20240104150917857.png" alt="image-20240104150917857" /> 

### 2.5.1 编程实验

[[05-sdl-yuv参考链接]]()

1. `pro` 文件

> ```bash
> TEMPLATE = app
> CONFIG += console c++20
> CONFIG -= app_bundle
> CONFIG -= qt
> 
> SOURCES += \
>         main.cpp
> 
> INCLUDEPATH += $$PWD/../SDL2-2.28.5-VC/include
> LIBS += $$PWD/../SDL2-2.28.5-VC/lib/x64/SDL2.lib
> 
> CONFIG += shadow -build
> DESTDIR = $$PWD/bin
> 
> ```

2. main.cpp

> ```c++
> #include <iostream>
> #include <string>
> #include <fstream>
> #include <thread>
> #include <memory_resource>
> #include <SDL.h>
> 
> #undef main
> 
> using namespace std;
> using namespace chrono;
> using namespace this_thread;
> 
> int main()
> {
>     //自定义消息类型
>     constexpr auto REFRESH_EVENT  {SDL_USEREVENT + 1};     // 请求画面刷新事件
>     constexpr auto QUIT_EVENT  {SDL_USEREVENT + 2};    // 退出事件
> 
>     //定义分辨率
>     // YUV像素分辨率
>     constexpr uint32_t YUV_WIDTH {320},YUV_HEIGHT{240};
> 
>     //定义YUV格式
>     constexpr auto YUV_FORMAT {SDL_PIXELFORMAT_IYUV};
> 
>     //初始化 SDL
>     if(SDL_Init(SDL_INIT_VIDEO)){
>         throw string("Could not initialize SDL - ") + SDL_GetError();
>     }
> 
>     //分辨率
>     // 1. YUV的分辨率
>     constexpr auto video_width {YUV_WIDTH},
>                     video_height {YUV_HEIGHT};
> 
>     // 2.显示窗口的分辨率
>     auto win_width {YUV_WIDTH},win_height {YUV_WIDTH};
>     //创建窗口
>     auto window { SDL_CreateWindow("Simplest YUV Player",
>                            SDL_WINDOWPOS_UNDEFINED,
>                            SDL_WINDOWPOS_UNDEFINED,
>                            video_width, video_height,
>                            SDL_WINDOW_OPENGL|SDL_WINDOW_RESIZABLE)};
>     if(!window){
>         throw string("SDL: could not create window, err : ") + SDL_GetError() + "\n";
>     }
> 
>     //基于窗口创建渲染器
>     auto renderer {SDL_CreateRenderer(window, -1, 0)};
>     if(!renderer){
>         SDL_DestroyWindow(window);
>         throw string("SDL: could not create Renderer, err : ") + SDL_GetError() + "\n";
>     }
> 
>     //基于渲染器创建纹理
>     auto texture { SDL_CreateTexture(renderer,
>                           YUV_FORMAT, // YUV420P，即是SDL_PIXELFORMAT_IYUV
>                           SDL_TEXTUREACCESS_STREAMING,
>                           video_width,
>                           video_height)};
> 
>     if(!texture){
>         SDL_DestroyWindow(window);
>         SDL_DestroyRenderer(renderer);
>         throw string("SDL: could not create Renderer, err : ") + SDL_GetError() + "\n";
>     }
> 
>     // 分配空间
>     std::pmr::pool_options opt{};
>     std::pmr::synchronized_pool_resource mpool(opt);
> 
>     //测试的文件是YUV420P格式
>     constexpr auto y_frame_len {video_width * video_height},
>     u_frame_len {y_frame_len / 4},v_frame_len {y_frame_len / 4},
>     yuv_frame_len {y_frame_len + u_frame_len + v_frame_len};
> 
>     void *video_buf{};
>     try {
>         video_buf = mpool.allocate(yuv_frame_len);
>     } catch (const bad_alloc &e) {
>         SDL_DestroyWindow(window);
>         SDL_DestroyRenderer(renderer);
>         SDL_DestroyTexture(texture);
>         const string errmsg(e.what());
>         cerr << errmsg;
>         throw errmsg + " Failed to alloce yuv frame space!\n";
>     }
> 
>     // 打开YUV文件
>     constexpr auto yuv_path {"yuv420p_320x240.yuv"};
>     ifstream ifs(yuv_path,ios::binary); /*只读和二进制形式打开*/
>     if(!ifs){
>         SDL_DestroyWindow(window);
>         SDL_DestroyRenderer(renderer);
>         SDL_DestroyTexture(texture);
>         mpool.deallocate(video_buf,yuv_frame_len);
>         mpool.release();
>         cerr << "Failed to open yuv file\n";
>         throw "Failed to open yuv file\n";
>     }
> 
>     // 创建请求刷新线程
>     bool s_thread_exit {};  // 退出标志 = true则退出
>     thread timer_thread([&](){
> 
>         while (!s_thread_exit){
>             SDL_Event event{.type = REFRESH_EVENT};
>             SDL_PushEvent(&event);
>             sleep_for(50ms);
>         }
> 
>         s_thread_exit = false;
> 
>         //push quit event
>         SDL_Event event{.type = QUIT_EVENT};
>         SDL_PushEvent(&event);
>     });
> 
>     for(;;){
> 
>         // 收取SDL系统里面的事件
>         SDL_Event event{};
>         SDL_WaitEvent(&event);
> 
>         if(REFRESH_EVENT == event.type){ // 画面刷新事件
> 
>             //auto video_buff_len {fread(video_buf, 1, yuv_frame_len, video_fd)}; /*一次读取一帧*/
> 
>             try {
>                 ifs.read(static_cast<char*>(video_buf),yuv_frame_len);
>             } catch (const ios::failure &e) {
>                 cerr << e.what() << "\n";
>                 cerr << "Failed to read data from yuv file!\n";
>                 cerr << SDL_GetError();
>                 goto _FAIL;
>             }
> 
>             const auto video_buff_len {ifs.gcount()};
> 
>             if(!video_buff_len){
>                 cout << "Play finish\n" << flush;
>                 break;
>             }
>             // 设置纹理的数据 video_width = 320, plane
>             /*采用planar格式存储,4个Y,对应一个U和一个V*/
>             SDL_UpdateTexture(texture, nullptr, video_buf, video_width);
> 
>             // 显示区域,可以通过修改w和h进行缩放,支持拉伸操作
> #if 1
>             const auto w_ratio {win_width * 1.0 /video_width},
>             h_ratio {win_height * 1.0 /video_height};
>             SDL_Rect rect{.x = 0,.y = 0,
>                         .w = static_cast<int>(video_width * w_ratio),
>                         .h = static_cast<int>(video_height * h_ratio)};
> 
>             //            rect.w = video_width * 0.5;
>             //            rect.h = video_height * 0.5;
> #else
>             /* 保持视频320x240分辨率 , 拉动窗口视频始终在窗口的正中心 */
>             SDL_Rect rect{.x = static_cast<int>((win_width - video_width)/2) ,
>                         .y = static_cast<int>((win_height - video_height)/2) ,
>                         .w = video_width ,
>                         .h = video_height };
> #endif
>             // 清除当前显示
>             SDL_RenderClear(renderer);
>             // 将纹理的数据拷贝给渲染器
>             SDL_RenderCopy(renderer, texture, nullptr, &rect);
>             // 显示
>             SDL_RenderPresent(renderer);
>         }else if(SDL_WINDOWEVENT == event.type){
>             //If Resize
>             auto w{reinterpret_cast<int*>(&win_width)},
>                 h{reinterpret_cast<int*>(&win_height)};
>             SDL_GetWindowSize(window, w, h);/*支持拉伸操作*/
>             cout << "SDL_WINDOWEVENT win_width : " << *w << ", win_height: " << *h << "\n";
>         }else if(SDL_QUIT == event.type){ //退出事件
>             s_thread_exit = true;
>         }else if(QUIT_EVENT == event.type){
>             cout << "QUIT_EVENT\n";
>             break;
>         }else{}
>     }
> 
> _FAIL:
>     s_thread_exit = true;      // 保证线程能够退出
> 
>     timer_thread.join(); // 等待线程退出
> 
>     ifs.close();
>     // 释放资源
>     mpool.deallocate(video_buf,yuv_frame_len);
>     mpool.release();
> 
>     SDL_DestroyTexture(texture);
> 
>     SDL_DestroyRenderer(renderer);
> 
>     SDL_DestroyWindow(window);
> 
>     SDL_Quit();
> 
>     return 0;
> }
> 
> ```

## 2.6 SDL播放音频PCM

### 2.6.1 打开音频设备

> ```c++
> typedef struct SDL_AudioSpec {
>         int freq; // 音频采样率
>         SDL_AudioFormat format; // 音频数据格式
>         Uint8 channels; // 声道数: 1 单声道, 2 立体声
>         Uint8 silence; // 设置静音的值， 因为声音采样是有符号的， 所以0当然就是这个值
>         Uint16 samples; // 音频缓冲区中的采样个数，要求必须是2的n次
>         Uint16 padding; // 考虑到兼容性的一个参数
>         Uint32 size; // 音频缓冲区的大小，以字节为单位
>         SDL_AudioCallback callback; // 填充音频缓冲区的回调函数
>         void *userdata; // 用户自定义的数据
> } SDL_AudioSpec;
> 
> int SDLCALL SDL_OpenAudio(SDL_AudioSpec* desired,SDL_AudioSpec* obtained);
> // desired：期望的参数。
> // obtained：实际音频设备的参数,一般情况下设置为NULL即可
> ```

### 2.6.2 `SDL_AudioCallback`

> ```c++
> // userdata:SDL_AudioSpec结构中的用户自定义数据,一般情况下可以不用
> // stream:该指针指向需要填充的音频缓冲区
> // len:音频缓冲区的大小(以字节为单位) 1024*2*2=4096bytes (采样数 * 两个字节 * 两个通道)
> void (SDLCALL *SDL_AudioCallback) (void *userdata, Uint8 *stream, int len);
> 
> // 当pause_on设置为0的时候即可开始播放音频数据。设置为1的时候，将会播放静音的值。
> void SDLCALL SDL_PauseAudio(int pause_on)
> ```

### 2.6.3 编程实验

[[06-sdl-pcm参考链接]]()

1. `pro` 文件

> ```bash
> TEMPLATE = app
> CONFIG += console c++20
> CONFIG -= app_bundle
> CONFIG -= qt
> 
> SOURCES += \
>         main.cpp
> 
> INCLUDEPATH += $$PWD/../SDL2-2.28.5-VC/include
> LIBS += $$PWD/../SDL2-2.28.5-VC/lib/x64/SDL2.lib
> 
> CONFIG += shadow -build
> DESTDIR = $$PWD/bin
> ```

2. main.cpp

> ```c++
> #include <iostream>
> #include <string>
> #include <memory_resource>
> #include <fstream>
> #include <thread>
> #include <exception>
> #include <SDL.h>
> 
> #undef main
> using namespace std;
> using namespace chrono;
> using namespace this_thread;
> 
> /**
>  * SDL2播放PCM
>  * 本程序使用SDL2播放PCM音频采样数据。SDL实际上是对底层绘图
>  * API（Direct3D，OpenGL）的封装，使用起来明显简单于直接调用底层
>  * API。
>  * 测试的PCM数据采用采样率44.1k, 采用精度S16SYS, 通道数2
>  *
>  * 函数调用步骤如下:
>  *
>  * [初始化]
>  * SDL_Init(): 初始化SDL。
>  * SDL_OpenAudio(): 根据参数（存储于SDL_AudioSpec）打开音频设备。
>  * SDL_PauseAudio(): 播放音频数据。
>  *
>  * [循环播放数据]
>  * SDL_Delay(): 延时等待播放完成。
>  *
>  */
> 
> 
> 
> // 音频PCM数据缓存
> static Uint8 *s_audio_buf {};
> // 目前读取的位置
> static Uint8 *s_audio_pos {};
> // 缓存结束位置
> static Uint8 *s_audio_end {};
> 
> //音频设备回调函数(调用次数由读取多少数据决定,调用间隔由spec.samples * spec.channels * spec.format)
> /*比如,1024个采样点 , 16bit格式 , 2个通道 */
> void fill_audio_pcm(void *udata, Uint8 *stream, int len)
> {
>     (void)udata;
> 
>     SDL_memset(stream, 0, len);
> 
>     if(s_audio_pos >= s_audio_end) {    //数据读取完毕
>         return;
>     }
> 
>     // 数据够了就读预设长度，数据不够就只读部分（不够的时候剩多少就读取多少）
>     /*当remain_buffer_len 比 len 还大 , 就先拷贝len ,
>      *如果不足len , 直接拷贝剩余的长度remain_buffer_len
>     */
>     const auto remain_buffer_len {s_audio_end - s_audio_pos},
>                 length {(len < remain_buffer_len) ? len : remain_buffer_len};
> 
>     // 拷贝数据到stream并调整音量
>     SDL_MixAudio(stream, s_audio_pos, static_cast<Uint32>(length), SDL_MIX_MAXVOLUME/8);
> 
>     cout << "len = " << length << "\n";
>     s_audio_pos += length;  // 移动缓存指针
> }
> 
> // 提取PCM文件
> // ffmpeg -i input.mp4 -t 20 -codec:a pcm_s16le -ar 44100 -ac 2 -f s16le 44100_16bit_2ch.pcm
> // 测试PCM文件
> // ffplay -ar 44100 -ac 2 -f s16le 44100_16bit_2ch.pcm
> 
> int main()
> {
>     //SDL initialize
>     if(SDL_Init(SDL_INIT_AUDIO)){    // 支持AUDIO
>         throw string("Could not initialize SDL - ") + SDL_GetError() + "\n";
>     }
> 
>     //打开PCM文件
>     constexpr auto path{"44100_16bit_2ch.pcm"};
>     ifstream ifs(path,ios::binary);
>     if(!ifs){
>         const string errmsg(string("Failed to open pcm file!\n"));
>         cerr << errmsg;
>         throw errmsg;
>     }
> 
>     pmr::pool_options opt{};
>     pmr::synchronized_pool_resource mptool(opt);
> 
>     //s_audio_buf = (uint8_t *)malloc(PCM_BUFFER_SIZE);
>     //以1024个采样点一帧 2通道 16bit采样点为例(2字节),每次读取2帧数据
>     constexpr auto PCM_BUFFER_SIZE (1024*2*2*2);
>     try {
>         s_audio_buf = static_cast<Uint8*>( mptool.allocate(PCM_BUFFER_SIZE));
>     } catch (const std::bad_alloc & e) {
>         ifs.close();
>         cerr << e.what();
>         throw e.what();
>     }
> 
>     // 音频参数设置SDL_AudioSpec
>     SDL_AudioSpec spec{};
>     spec.freq = 44100;          // 采样频率
>     spec.format = AUDIO_S16SYS; // 采样点格式
>     spec.channels = 2;          // 2通道
>     spec.silence = 0;
>     spec.samples = 1024;       // 23.2ms -> 46.4ms 每次读取的采样数量，多久产生一次回调和 samples
>     spec.callback = fill_audio_pcm; // 回调函数
>     spec.userdata = nullptr;
> 
>     //打开音频设备
> 
>     if(SDL_OpenAudio(&spec, nullptr)) {
>         mptool.deallocate(s_audio_buf,PCM_BUFFER_SIZE);
>         mptool.release();
>         ifs.close();
>         const string errmsg(string("Failed to open audio device, ")  + SDL_GetError() + "\n");
>         cout << errmsg;
>         throw errmsg;
>     }
> 
>     //play audio
>     SDL_PauseAudio(0);
> 
>     long long data_count {};
> 
>     for(;;){
>         // 从文件读取PCM数据
>         try {
>             ifs.read(reinterpret_cast<char*>(s_audio_buf),PCM_BUFFER_SIZE);
>         } catch (const ios::failure &e) {
>             cerr << e.what() << "\n";
>             cerr << "Failed to read data from pcm file!\n";
>             cerr << SDL_GetError();
>             break;
>         }
> 
>         // 每次缓存的长度
>         const auto read_buffer_len {ifs.gcount()};
> 
>         if(read_buffer_len <= 0) {
>             break;
>         }
> 
>         data_count += read_buffer_len; // 统计读取的数据总字节数
>         cout << "now playing " << data_count << " bytes data.\n";
>         s_audio_end = s_audio_buf + read_buffer_len;    // 更新buffer的结束位置
>         s_audio_pos = s_audio_buf;  // 更新buffer的起始位置
>         //the main thread wait for a moment
>         while(s_audio_pos < s_audio_end){
>             sleep_for(10ms);  // 等待PCM数据消耗
>             /* delay_ms < spec.samples / spec.freq */
>         }
>     }
> 
>     cout << "play PCM finish\n";
>     // 关闭音频设备
>     SDL_CloseAudio();
> 
>     //release some resources
>     mptool.deallocate(s_audio_buf,PCM_BUFFER_SIZE);
>     mptool.release();
> 
>     ifs.close();
>     //quit SDL
>     SDL_Quit();
> 
>     return 0;
> }
> 
> ```

