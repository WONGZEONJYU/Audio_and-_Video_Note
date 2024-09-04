cmake_minimum_required(VERSION 3.29)
project(113_first_encode)

set(CMAKE_CXX_STANDARD 26)

include_directories(/usr/local/ffmpeg/include)
include_directories(/opt/homebrew/Cellar/sdl2/2.30.1/include/SDL2)

link_directories(/usr/local/ffmpeg/lib)
link_directories(/opt/homebrew/Cellar/sdl2/2.30.1/lib)

set(FFMPEG_LIBS
        avcodec
        avdevice
        avfilter
        avformat
        avutil
        postproc
        swresample
        swscale
        SDL2
)

add_executable(${PROJECT_NAME} main.cpp
        XHelper.cpp)

target_link_libraries(${PROJECT_NAME} ${FFMPEG_LIBS})
add_definitions(-DHAVE_FFMPEG)
