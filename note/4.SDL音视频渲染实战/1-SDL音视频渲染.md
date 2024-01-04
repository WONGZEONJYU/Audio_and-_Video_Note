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

### 1.3.1 自己下载搭建

[[下载地址]](https://www.libsdl.org/download-2.0.php)

<img src="assets/image-20240104101849453.png" alt="image-20240104101849453" />

下载后打开安装包将 SDL2.framework 拖拽到 /Library/Frameworks这个目录下

### 1.3.2 命令搭建

> ```bash
> brew install SDL2
> #这是使用 Homebrew 默认的方式来安装 SDL2。这将会从 Homebrew 的二进制存储库中下载预编译的二进制文件,并将其安装到系统中。通常情况下,这种方式比较快速,因为它直接使用了预编译好的二进制文件,无需进行编译操作
> 
> brew install --build-from-source sdl2
> #这个命令指示 Homebrew 在安装SDL2时从源代码构建。这意味着 Homebrew 将下载 SDL2的源代码并在本地编译,然后将其安装在系统中。这种方式需要更多的时间和计算资源,但具有更大的灵活性,因为可以在编译过程中进行自定义设置和调整
> ```

## 1.4 新建开发工程(qt)

### 1.4.1 新建工程

<img src="assets/image-20240104111120569.png" alt="image-20240104111120569" /> 

路径任君选择 , 不能有中文路径

<img src="assets/image-20240104111215746.png" alt="image-20240104111215746" /> 

<img src="assets/image-20240104111313608.png" alt="image-20240104111313608" /> 

教程采用32bit开发 , 本人尝试使用64bit开发

### 1.4.2 拷贝SDL库到工程目录

<img src="assets/image-20240104111702843.png" alt="image-20240104111702843" /> 

 <img src="assets/image-20240104141717961.png" alt="image-20240104141717961" /> 

### 1.4.3 修改 `pro` 文件

> ```bash
> TEMPLATE = app
> CONFIG += console c++17
> CONFIG -= app_bundle
> CONFIG -= qt
> 
> SOURCES += \
>         main.cpp
> 
> INCLUDEPATH += $$PWD/SDL2-2.28.5-VC/include
> LIBS += $$PWD/SDL2-2.28.5-VC/lib/x64/SDL2.lib
> #把库文件路径与头文件路径加进去,库文件路径需要指定到具体某个库
> 
> CONFIG += shadow -build
> DESTDIR = $$PWD/bin
> #CONFIG += shadow 用于告诉 qmake 使用生成的中间文件目录作为构建目录
> #DESTDIR = $$PWD/bin 把输出目录调整到工程文件夹下的bin文件夹
> ```

### 1.4.4 main.cpp包含头文件并测试

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