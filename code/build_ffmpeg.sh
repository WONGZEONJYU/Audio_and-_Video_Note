export LDFLAGS="-L/opt/homebrew/lib"
export CPPFLAGS="-I/opt/homebrew/include"

./configure \
    --prefix=/usr/local/ffmpeg \
    --enable-gpl \
    --enable-version3 \
    --enable-nonfree \
    --enable-postproc \
    --disable-libass \
    --enable-libfdk-aac \
    --enable-libfreetype \
    --enable-libmp3lame  \
    --enable-libopencore-amrnb \
    --enable-libopencore-amrwb \
    --enable-libopenjpeg    \
    --enable-libopus \
    --enable-libspeex \
    --enable-libtheora \
    --enable-libvorbis \
    --enable-libvpx \
    --enable-libx264 \
    --enable-libxvid \
    --enable-static \
    --enable-shared \
    --enable-openssl
