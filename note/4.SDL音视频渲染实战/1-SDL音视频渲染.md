# 1.SDL简介

[[官网]](https://www.libsdl.org/)	[[文档]](http://wiki.libsdl.org/Introduction  )

SDL（Simple DirectMedia Layer）是一套开放源代码的跨平台多媒体开发库 , 使用C语言写成。 SDL提供了数种控制图像、声音、输出入的函数 , 让开发者只要用相同或是相似的代码就可以开发出跨多个平台 (Linux、 Windows、 Mac OS X等) 的应用软件。目前SDL多用于开发游戏、模拟器、 媒体播放器等多媒体应用领域

对于我们课程而言 : SDL主要用来辅助学习FFmpeg , 所以我们只会关注我们用到的知识点

<img src="assets/image-20240104094407419.png" alt="image-20240104094407419" /> 

# 2. Windows环境搭建

[[下载地址]](https://www.libsdl.org/download-2.0.php)

<img src="assets/image-20240104094551340.png" alt="image-20240104094551340" /> 

 版本会持续更新 , 下载对应的版本就OK

# 3. Linux环境搭建

## 3.1 下载SDL源码库 , SDL2-x.x.x.tar.gz

[[下载地址]](https://www.libsdl.org/download-2.0.php)

<img src="assets/image-20240104094754989.png" alt="image-20240104094754989" /> 

## 3.2 解压

> ```bash
> tar -vxf SDL2-x.x.x.tar.gz
> ```

## 3.3 配置 & 编译

> ```bash
> ./configure
> make
> sudo make install
> ```

## 3.4 遇到错误

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

# 4. Mac 搭建

## 4.1 自己下载搭建

[[下载地址]](https://www.libsdl.org/download-2.0.php)

<img src="assets/image-20240104101849453.png" alt="image-20240104101849453" />

下载后打开安装包将 SDL2.framework 拖拽到 /Library/Frameworks这个目录下

## 4.2 命令搭建

> ```bash
> brew install SDL2
> #这是使用 Homebrew 默认的方式来安装 SDL2。这将会从 Homebrew 的二进制存储库中下载预编译的二进制文件,并将其安装到系统中。通常情况下,这种方式比较快速,因为它直接使用了预编译好的二进制文件,无需进行编译操作
> 
> brew install --build-from-source sdl2
> #这个命令指示 Homebrew 在安装SDL2时从源代码构建。这意味着 Homebrew 将下载 SDL2的源代码并在本地编译,然后将其安装在系统中。这种方式需要更多的时间和计算资源,但具有更大的灵活性,因为可以在编译过程中进行自定义设置和调整
> ```

