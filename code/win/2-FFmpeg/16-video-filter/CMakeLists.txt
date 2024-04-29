cmake_minimum_required(VERSION 3.28)
project(16_video_filter)

set(CMAKE_CXX_STANDARD 20)

add_definitions(-D__STDC_CONSTANT_MACROS)

if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    # Windows操作系统下的特定配置
    message("Windows operating system detected")
    # 设置Windows下的编译选项
    add_definitions(-DWINDOWS)
    # 链接Windows下的特定库
#    include_directories(${PROJECT_SOURCE_DIR}/../ffmepg-6.1/include)
#
#    link_directories(${PROJECT_SOURCE_DIR}/../ffmepg-6.1/bin)

    include_directories(c:/ffmepg-6.1/include)

    link_directories(c:/ffmepg-6.1/bin)

elseif (CMAKE_SYSTEM_NAME STREQUAL "Darwin" OR CMAKE_SYSTEM_NAME STREQUAL "Mac OS X")
    # macOS操作系统下的特定配置
    message("macOS operating system detected")
    # 设置macOS下的编译选项
    add_definitions(-DMACOS)
    # 链接macOS下的特定库
    include_directories(/usr/local/ffmpeg/include)

    link_directories(/usr/local/ffmpeg/lib)
endif ()

set(FFMPEG_LIBS avcodec avdevice avfilter avformat avutil postproc swresample swscale)

set(SRC_FILE main.cpp
        AVFilter_Demo.cpp
        AVFilter_Demo.hpp
        AVHelper.cpp
        AVHelper.h
        ShareAVFrame.cpp
        ShareAVFrame.hpp
        ShareAVPacket.cpp
        ShareAVPacket.hpp)

add_executable(${PROJECT_NAME} ${SRC_FILE})

target_link_libraries(${PROJECT_NAME} ${FFMPEG_LIBS})
