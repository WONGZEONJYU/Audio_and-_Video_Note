cmake_minimum_required(VERSION 3.28)
project(05_flv_parser_cplus)

set(CMAKE_CXX_STANDARD 17)

if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    # Windows操作系统下的特定配置
    message("Windows operating system detected")
    # 设置Windows下的编译选项
    add_definitions(-DWINDOWS)
    # 链接Windows下的特定库
    include_directories(${PROJECT_SOURCE_DIR}/../ffmepg-6.1/include)

    link_directories(${PROJECT_SOURCE_DIR}/../ffmepg-6.1/lib)

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

set(SRC_FILE FlvMetaData.cpp FlvParser.cpp Videojj.cpp vadbg.cpp main.cpp)

add_executable(${PROJECT_NAME} ${SRC_FILE})

target_link_libraries(${PROJECT_NAME} ${FFMPEG_LIBS})

