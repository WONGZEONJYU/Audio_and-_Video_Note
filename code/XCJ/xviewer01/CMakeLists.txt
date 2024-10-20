cmake_minimum_required(VERSION 3.29)
project(xviewer01)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

add_definitions(-DHAVE_FFMPEG)
add_definitions(-D__STDC_CONSTANT_MACROS)
add_definitions(-DHAVE_SDL2)

if (WIN32 AND NOT DEFINED CMAKE_TOOLCHAIN_FILE)
    add_definitions(-DWINDOWS)
    include_directories(C:/libffmepg/include)
    include_directories(C:/SDL2-2.30.8/include)

    link_directories(C:/libffmepg/bin)
    link_directories(C:/SDL2-2.30.8/lib/x64)
    set(Vulkan_INCLUDE_DIR "C:/VulkanSDK/1.3.290.0/Include")
    set(Vulkan_LIBRARY "C:/VulkanSDK/1.3.290.0/Lib/vulkan-1.lib")
    find_package(Vulkan REQUIRED)
    include_directories(${Vulkan_INCLUDE_DIR})

elseif (CMAKE_SYSTEM_NAME STREQUAL "Darwin" OR CMAKE_SYSTEM_NAME STREQUAL "Mac OS X")

    add_definitions(-DMACOS)
    set(CMAKE_PREFIX_PATH "/Users/wong/Qt/6.8.0/macos")
    include_directories(/opt/homebrew/opt/llvm/include)
    include_directories(/usr/local/ffmpeg/include)
    include_directories(/opt/homebrew/Cellar/sdl2/2.30.8/include/SDL2)
    include_directories(/Users/wong/Library/xlib/include)

    link_directories(/usr/local/ffmpeg/lib)
    link_directories(/opt/homebrew/Cellar/sdl2/2.30.8/lib)
    link_directories(/Users/wong/Library/xlib/lib)

    set(Vulkan_INCLUDE_DIR "/Users/wong/VulkanSDK/1.3.283.0/macOS/include")
    set(Vulkan_LIBRARY "/Users/wong/VulkanSDK/1.3.283.0/macOS/lib/libvulkan.dylib")
    find_package(Vulkan REQUIRED)
    include_directories(${Vulkan_INCLUDE_DIR})

endif ()

set(FFMPEG_LIBS
        avcodec
        avdevice
        avfilter
        avformat
        avutil
        postproc
        swresample
        swscale
)

set(SDL_LIBS SDL2)
set(XLIB xlib)

find_package(Qt6 COMPONENTS
        Core
        Gui
        Widgets
        REQUIRED)

add_executable(${PROJECT_NAME} main.cpp
        ui/xviewer.cpp
        ui/xviewer.hpp
        ui/xviewer.ui)

target_link_libraries(${PROJECT_NAME} ${SDL_LIBS} ${FFMPEG_LIBS} ${XLIB}
        Qt::Core
        Qt::Gui
        Qt::Widgets

)
