cmake_minimum_required(VERSION 3.28)
project(01_ffplay_cpp)

set(CMAKE_CXX_STANDARD 20)

include_directories(${PROJECT_SOURCE_DIR})

if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    # Windows操作系统下的特定配置
    message("Windows operating system detected")
    # 设置Windows下的编译选项
    add_definitions(-DWINDOWS)
    # 链接Windows下的特定库

    # include_directories(${PROJECT_SOURCE_DIR}/../ffmepg-6.1/include)

    # link_directories(${PROJECT_SOURCE_DIR}/../ffmepg-6.1/bin)

    include_directories(c:/ffmepg-7.1/include)
    include_directories(c:/SDL2-2.28.5-VC/include)

    link_directories(c:/ffmepg-7.1/bin)
    link_directories(c:/SDL2-2.28.5-VC/lib/x64)

elseif (CMAKE_SYSTEM_NAME STREQUAL "Darwin" OR CMAKE_SYSTEM_NAME STREQUAL "Mac OS X")
    # macOS操作系统下的特定配置
    message("macOS operating system detected")
    # 设置macOS下的编译选项
    add_definitions(-DMACOS)
    # 链接macOS下的特定库
    include_directories(/usr/local/ffmpeg/include)
    include_directories(/opt/homebrew/Cellar/sdl2/2.30.1/include/SDL2)

    link_directories(/usr/local/ffmpeg/lib)
    link_directories(/opt/homebrew/Cellar/sdl2/2.30.1/lib)

endif ()

include_directories("/")

set(FFMPEG_LIBS avcodec avdevice avfilter avformat avutil postproc swresample swscale SDL2)

set(SRC_FILE ffplay.cpp cmdutils.cpp ffplay_renderer.cpp opt_common.cpp)

add_executable(${PROJECT_NAME} ${SRC_FILE})

target_link_libraries(${PROJECT_NAME} ${FFMPEG_LIBS})
