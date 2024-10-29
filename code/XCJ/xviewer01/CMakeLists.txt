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
##################################根据开发电脑进行调整路径###################################
    set(CMAKE_PREFIX_PATH "C:/Qt6/6.8.0/msvc2022_64")
    # 设定为 Unicode 编码
    add_definitions(-DUNICODE -D_UNICODE)
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)

    set(FFMPEG_HEADERS_PATH "C:/libffmpeg/include")
    set(FFMPEG_LIB_PATH "C:/libffmpeg/bin")

    set(SDL_HEADERS_PATH "C:/SDL2-2.30.8/include")
    set(SDL_LIB_PATH "C:/SDL2-2.30.8/lib/x64")

    set(Vulkan_INCLUDE_DIR "C:/VulkanSDK/1.3.290.0/Include")
    set(Vulkan_LIBRARY "C:/VulkanSDK/1.3.290.0/Lib/vulkan-1.lib")

    set(XLIB_HEADERS_PATH "C:/xlib/include")
    set(XLIB_LIBS_PATH "C:/xlib/lib")

    ##################################根据开发电脑进行调整路径###################################
    file(GLOB FFMPEG_LIBS "${FFMPEG_LIB_PATH}/*.lib")
    file(GLOB SDL_LIBS "${SDL_LIB_PATH}/*.lib")
    file(GLOB XLIB "${XLIB_LIBS_PATH}/*.lib")

    set(APP_ICON_RESOURCE "${CMAKE_SOURCE_DIR}/resources/win.rc")

elseif (CMAKE_SYSTEM_NAME STREQUAL "Darwin" OR CMAKE_SYSTEM_NAME STREQUAL "Mac OS X")
    add_definitions(-DMACOS)
##################################根据开发电脑进行调整路径###################################
    set(CMAKE_PREFIX_PATH "/Users/wong/Qt/6.8.0/macos")

    set(FFMPEG_HEADERS_PATH "/usr/local/ffmpeg/include")
    set(FFMPEG_LIB_PATH "/usr/local/ffmpeg/lib")

    set(SDL_HEADERS_PATH "/opt/homebrew/Cellar/sdl2/2.30.8/include/SDL2")
    set(SDL_LIB_PATH "/opt/homebrew/Cellar/sdl2/2.30.8/lib")

    set(Vulkan_INCLUDE_DIR "/Users/wong/VulkanSDK/1.3.283.0/macOS/include")
    set(Vulkan_LIBRARY "/Users/wong/VulkanSDK/1.3.283.0/macOS/lib/libvulkan.dylib")

    set(XLIB_HEADERS_PATH "/Users/wong/Library/xlib/include")
    set(XLIB_LIBS_PATH "/Users/wong/Library/xlib/lib")
##################################根据开发电脑进行调整路径###################################
    file(GLOB FFMPEG_LIBS "${FFMPEG_LIB_PATH}/*.dylib")
    file(GLOB SDL_LIBS "${SDL_LIB_PATH}/*.dylib")
    file(GLOB XLIB "${XLIB_LIBS_PATH}/*.dylib")
endif ()

include_directories(${FFMPEG_HEADERS_PATH})
include_directories(${SDL_HEADERS_PATH})
include_directories(${Vulkan_INCLUDE_DIR})
include_directories(${XLIB_HEADERS_PATH})
include_directories(${CMAKE_SOURCE_DIR})
include_directories(${CMAKE_SOURCE_DIR}/ui)

#link_directories(${FFMPEG_LIB_PATH})
#link_directories(${SDL_LIB_PATH})
#link_directories(${XLIB_LIBS_PATH})

find_package(Vulkan REQUIRED)

find_package(Qt6 COMPONENTS
        Core
        Gui
        Widgets
        REQUIRED)

#######添加应用程序系统级图标#######
set(QRC_SOURCE_FILE "${CMAKE_SOURCE_DIR}/resources/resources.qrc")
qt_add_resources(${QRC_SOURCE_FILE})
#######添加应用程序系统级图标#######

set(Search_Range
        "${CMAKE_SOURCE_DIR}/*.cpp"
        "${CMAKE_SOURCE_DIR}/*.hpp"
        "${CMAKE_SOURCE_DIR}/*.c"
        "${CMAKE_SOURCE_DIR}/*.h"
        "${CMAKE_SOURCE_DIR}/ui/*.cpp"
        "${CMAKE_SOURCE_DIR}/ui/*.hpp"
        "${CMAKE_SOURCE_DIR}/ui/*.ui"
        "${CMAKE_SOURCE_DIR}/ui/*.h")

file(GLOB SOURCE_FILE ${Search_Range})

add_executable(${PROJECT_NAME} ${SOURCE_FILE} ${QRC_SOURCE_FILE} ${APP_ICON_RESOURCE})

target_link_libraries(
        ${PROJECT_NAME}
        ${SDL_LIBS}
        ${FFMPEG_LIBS}
        ${XLIB}
        #xlib
        Qt::Core
        Qt::Gui
        Qt::Widgets
)
