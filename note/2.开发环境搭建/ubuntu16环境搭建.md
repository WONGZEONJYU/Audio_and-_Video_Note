# 1.创建目录

在home目录下创建
ffmpeg_sources : 用于下载源文件
ffmpeg_build : 存储编译后的库文件
bin：存储二进制文件 (ffmpeg，ffplay，ffprobe，X264，X265等)  

> ```bash
> cd ~
> mkdir fmpeg_sources ffmpeg_build bin  
> ```

# 2. 安装依赖

> ```bash
> sudo apt-get update
> sudo apt-get -y install \
> autoconf \
> automake \
> build-essential \
> cmake \
> git-core \
> libass-dev \
> libfreetype6-dev \
> libsdl2-dev \
> libtool \
> libva-dev \
> libvdpau-dev \
> libvorbis-dev \
> libxcb1-dev \
> libxcb-shm0-dev \
> libxcb-xfixes0-dev \
> pkg-config \
> texinfo \
> wget \
> zlib1g-dev
> ```

# 3. 编译与安装

## 3.1 NASM

部分库使用到汇编程序,使用源码进行安装

> ```bash
> cd ~/ffmpeg_sources && \
> wget https://www.nasm.us/pub/nasm/releasebuilds/2.14.02/nasm-2.14.02.tar.bz2 && \
> tar xjvf nasm-2.14.02.tar.bz2 && \
> cd nasm-2.14.02 && \
> ./autogen.sh && \
> PATH="$HOME/bin:$PATH" ./configure --prefix="$HOME/ffmpeg_build" --bindir="$HOME/bin" && \
> make && \
> sudo make install
> ```

## 3.2 Yasm

部分库使用到该汇编库,使用源码进行安装 : 

> ```bash
> cd ~/ffmpeg_sources && \
> wget -O yasm-1.3.0.tar.gz https://www.tortall.net/projects/yasm/releases/yasm-1.3.0.tar.gz && \
> tar xzvf yasm-1.3.0.tar.gz && \
> cd yasm-1.3.0 && \
> ./configure --prefix="$HOME/ffmpeg_build" --bindir="$HOME/bin" && \
> make && \
> sudo make install
> ```

## 3.3 libx264

H.264 视频编码器 , 更多信息和使用范例参考H.264 Encoding Guide
要求编译 ffmpeg 时配置 : `--enable-gpl --enable-libx264`
使用源码进行编译 : 

> ```bash
> cd ~/ffmpeg_sources && \
> git -C x264 pull 2> /dev/null || git clone --depth 1 https://gitee.com/mirrors_addons/x264.git && \
> cd x264 && \
> PATH="$HOME/bin:$PATH" PKG_CONFIG_PATH="$HOME/ffmpeg_build/lib/pkgconfig" \
> ./configure --prefix="$HOME/ffmpeg_build" --bindir="$HOME/bin" --enable-static --enable-pic && \
> PATH="$HOME/bin:$PATH" && \
> make && \
> sudo make install
> ```

## 3.4 libx265

H.265/HEVC 视频编码器， 更多信息和使用范例参考H.265 Encoding Guide。
要求编译ffmpeg时配置 : `--enable-gpl --enable-libx265`
使用源码进行编译 : 

> ```bash
> sudo apt-get install mercurial libnuma-dev && \
> cd ~/ffmpeg_sources && \
> if cd x265 2> /dev/null; then git pull && cd ..; else git clone https://gitee.com/mirrors_videolan/x26
> 5.git; fi && \
> cd x265/build/linux && \
> PATH="$HOME/bin:$PATH" cmake -G "Unix Makefiles" -DCMAKE_INSTALL_PREFIX="$HOME/ffmpeg_build" \
> -DENABLE_SHARED=off ../../source && \
> PATH="$HOME/bin:$PATH" make && \
> sudo make install
> ```

## 3.5 libvpx

VP8/VP9视频编解码器 , 更多信息和使用范例参考VP9 Video Encoding Guide 。
要求编译ffmpeg时配置 :  `--enable-libvpx`
使用源码进行编译 : 

> ```bash
> cd ~/ffmpeg_sources && \
> git -C libvpx pull 2> /dev/null || git clone --depth 1 https://github.com/webmproject/libvpx.git && \
> cd libvpx && \
> PATH="$HOME/bin:$PATH" && \
> ./configure --prefix="$HOME/ffmpeg_build" --disable-examples --disable-unit-tests \
>  --enable-vp9-highbitdepth --as=yasm --enable-pic && \
> PATH="$HOME/bin:$PATH" && \
> make && \
> sudo make install
> ```

## 3.6 libfdk-aac

AAC音频编码器 , 更多信息和使用范例参考AAC Audio Encoding Guide。
要求编译 ffmpeg 时配置 : `--enable-libfdk-aac` ( 如果你已经配置了 `--enable-gpl` 则需要加上 `--enablenonfree` )
使用源码进行编译 : 

> ```bash
> cd ~/ffmpeg_sources && \
> git -C fdk-aac pull 2> /dev/null || git clone --depth 1 https://github.com/mstorsjo/fdk-aac && \
> cd fdk-aac && \
> autoreconf -fiv && \
> ./configure CFLAGS=-fPIC --prefix="$HOME/ffmpeg_build" && \
> make && \
> sudo make install
> ```

## 3.7 libmp3lame

MP3音频编码器 , 要求编译ffmpeg时配置 : `--enable-libmp3lame`

使用源码进行编译 : 

> ```bash
> cd ~/ffmpeg_sources && \
> git clone --depth 1 https://gitee.com/hqiu/lame.git && \
> cd lame && \
> PATH="$HOME/bin:$PATH" && \
> ./configure --prefix="$HOME/ffmpeg_build" --bindir="$HOME/bin" --enable-nasm --with-pic && \
> PATH="$HOME/bin:$PATH" make && \
> sudo make install
> ```

## 3.8 libopus

Opus音频编解码器 , 要求编译ffmpeg时配置 : `--enable-libopus`
使用源码进行编译 : 

> ```bash
> cd ~/ffmpeg_sources && \
> git -C opus pull 2> /dev/null || git clone --depth 1 https://github.com/xiph/opus.git && \
> cd opus && \
> ./autogen.sh && \
> ./configure --prefix="$HOME/ffmpeg_build" -with-pic && \
> make && \
> sudo make install
> ```

## 3.9 FFmpeg

> ```bash
> cd ~/ffmpeg_sources && \
> wget -O ffmpeg-4.2.1.tar.bz2 https://ffmpeg.org/releases/ffmpeg-4.2.1.tar.bz2 && \
> tar xjvf ffmpeg-4.2.1.tar.bz2 && \
> cd ffmpeg-4.2.1 && \
> PATH="$HOME/bin:$PATH" PKG_CONFIG_PATH="$HOME/ffmpeg_build/lib/pkgconfig" CFLAGS="-O3 -fPIC"
> ```

新建一个 `build_ffmpeg.sh` : 

> ```bash
> ./configure \
>     --prefix="$HOME/ffmpeg_build" \
>     --pkg-config-flags="--static" \
>     --extra-cflags="-I$HOME/ffmpeg_build/include" \
>     --extra-ldflags="-L$HOME/ffmpeg_build/lib" \
>     --extra-libs="-lpthread -lm" \
>     --bindir="$HOME/bin" \
>     --enable-gpl \
>     --enable-libass \
>     --enable-libfdk-aac \
>     --enable-libfreetype \
>     --enable-libmp3lame \
>     --enable-libopus \
>     --enable-libvorbis \
>     --enable-libvpx \
>     --enable-libx264 \
>     --enable-libx265 \
>     --enable-pic \
>     --enable-shared \
>     --enable-nonfree && \
>     PATH="$HOME/bin:$PATH" && \
> make && \
> sudo make install && \
> hash -r
> ```

然后重新登录系统或者在当前shell会话执行如下命令以识别新安装ffmpeg的位置 : 
`source ~/.profile`

# 4. 使用

现在 , 您可以打开一个终端 , 输入ffmpeg命令 , 它应该执行新的ffmpeg。
如果你需要多个用户能同时使用你新编译的ffmpeg , 则可以移动或者拷贝ffmpeg二进制文件从~/bin
到/usr/local/bin。测试ffplay是否可以使用 (需要在图形方式进行测试)
`ffplay rtmp://media3.scctv.net/live/scctv_800`
如果能够正常播放则说明 ffplay能够编译成功使用。播放的时候要等等画面。

# 5. 文档  

> ```tex
> 你可以使用 man ffmpeg 以本地的方式访问文档:
> echo"MANPATH_MAP $HOME/bin $HOME/ffmpeg_build/share/man" >> ~/.manpath
> 你可能必须注销系统然后重新登录man ffmpeg才生效。
> HTML 格式的文档位于 ~/ffmpeg_build/share/doc/ffmpeg.
> 你也可以参考在线文档 online FFmpeg documentation 
> ```

# 6. 支持FFmpeg代码 Debug

刚才的工程可以运行 , 但不能debug , 解决此问题 , 首先认定一点 , 生成的可执行程序中 , ffmpeg 不包
含调试信息 , 调试信息在 ffmpeg_g 中 , debug 要选择 ffmpeg_g

另外 , ./config选项也是确定包含调试信息的核心 , 需要在config中添加 : 

* `-–enable-debug=3` : 开启debug调试
* `-–disable-asm` : 禁用 asm 优化
* `--disable-optimizations` : 禁用优化 , 以便调试时按函数顺序执行。
* `–-disable-stripping` : 禁用剥离可执行程序和共享库 , 即调试时可以进入到某个函数进行单独调试。

采用以下命令重新 `config` : 

1. 先clean

> ```bash
> make clean
> ```

2. 再重新config 

> ```bash
> ./configure \
>     --prefix="$HOME/ffmpeg_build" \
>     --pkg-config-flags="--static" \
>     --extra-cflags="-I$HOME/ffmpeg_build/include" \
>     --extra-ldflags="-L$HOME/ffmpeg_build/lib" \
>     --extra-libs="-lpthread -lm" \
>     --bindir="$HOME/bin" \
>     --enable-gpl \
>     --enable-libass \
>     --enable-libfdk-aac \
>     --enable-libfreetype \
>     --enable-libmp3lame \
>     --enable-libopus \
>     --enable-libvorbis \
>     --enable-libvpx \
>     --enable-libx264 \
>     --enable-libx265 \
>     --enable-pic \
>     --enable-shared \
>     --enable-nonfree \
>     --enable-debug=3 \
>     --disable-optimizations \
>     --disable-asm \
>     --disable-stripping && \
>     PATH="$HOME/bin:$PATH" make && \
>     make install && \
>     hash -r
> ```

# 7. 注意事项

> ```tex
> 在使用 ffplay 播放生成 h264 格式的视频时,播放速度会加快,解决方式:不要使用 FFmpeg 转码生成纯 h264 格式的视频,要使用一种容器包含 h264 视频,即生成一种音视频流格式,也就是不要生成纯粹的 h264 码流,而是生成诸如 mkv 等格式的文件
> ```



