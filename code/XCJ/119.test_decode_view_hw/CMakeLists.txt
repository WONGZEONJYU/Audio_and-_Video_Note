cmake_minimum_required(VERSION 3.29)
project(119.test_decode_view_hw)

set(CMAKE_CXX_STANDARD 26)

if (WIN32 AND NOT DEFINED CMAKE_TOOLCHAIN_FILE)
    add_definitions(-DWINDOWS)
    #set(CMAKE_PREFIX_PATH "C:/Qt6/6.4.2/mingw_64/")
    set(CMAKE_PREFIX_PATH "C:/Qt6/6.8.0/mingw_64/")
    include_directories(c:/ffmepg-7.1/include)
    include_directories(c:/SDL2-2.28.5-VC/include)

    link_directories(c:/ffmepg-7.1/bin)
    link_directories(c:/SDL2-2.28.5-VC/lib/x64)
    set(MM_FILE)
    set(Vulkan_INCLUDE_DIR "C:/VulkanSDK/1.3.290.0/Include")
    set(Vulkan_LIBRARY "C:/VulkanSDK/1.3.290.0/Lib/vulkan-1.lib")
    find_package(Vulkan REQUIRED)
    include_directories(${Vulkan_INCLUDE_DIR})

elseif (CMAKE_SYSTEM_NAME STREQUAL "Darwin" OR CMAKE_SYSTEM_NAME STREQUAL "Mac OS X")
    add_definitions(-DMACOS)

    #set(CMAKE_PREFIX_PATH "/Users/wong/Qt/6.4.2/macos")
    set(CMAKE_PREFIX_PATH "/Users/wong/Qt/6.7.2/macos")
    include_directories(/usr/local/ffmpeg/include)
    include_directories(/opt/homebrew/Cellar/sdl2/2.30.8/include/SDL2)

    link_directories(/usr/local/ffmpeg/lib)
    link_directories(/opt/homebrew/Cellar/sdl2/2.30.8/lib)

    set(Vulkan_INCLUDE_DIR "/Users/wong/VulkanSDK/1.3.283.0/macOS/include")
    set(Vulkan_LIBRARY "/Users/wong/VulkanSDK/1.3.283.0/macOS/lib/libvulkan.dylib")
    find_package(Vulkan REQUIRED)
    include_directories(${Vulkan_INCLUDE_DIR})

endif()

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
        xavframe.cpp
        xavpacket.cpp
        xhelper.cpp
        xsdl.cpp
        xvideo_view.cpp)

target_link_libraries(${PROJECT_NAME} ${FFMPEG_LIBS})
add_definitions(-DHAVE_FFMPEG)
add_definitions(-D__STDC_CONSTANT_MACROS)
add_definitions(-DHAVE_SDL2)
