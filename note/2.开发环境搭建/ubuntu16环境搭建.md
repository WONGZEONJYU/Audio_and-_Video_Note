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