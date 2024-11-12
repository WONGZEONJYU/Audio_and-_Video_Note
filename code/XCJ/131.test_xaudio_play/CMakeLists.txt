cmake_minimum_required(VERSION 3.29)
project(131_test_xaudio_play)

set(CMAKE_CXX_STANDARD 20)
add_definitions(-DHAVE_SDL2)

if (WIN32)
    add_definitions(-DWINDOWS)

    set(FFMPEG_HEADERS_PATH "C:/libffmpeg/include")
    set(FFMPEG_LIB_PATH "C:/libffmpeg/bin")

    set(SDL_HEADERS_PATH "C:/SDL2-2.30.8/include")
    set(SDL_LIB_PATH "C:/SDL2-2.30.8/lib/x64")

    set(XLIB_HEADERS_PATH "C:/xlib/include")
    set(XLIB_LIBS_PATH "C:/xlib/lib")

    file(GLOB FFMPEG_LIBS "${FFMPEG_LIB_PATH}/*.lib")
    file(GLOB SDL_LIBS "${SDL_LIB_PATH}/*.lib")

    file(GLOB XLIB "${XLIB_LIBS_PATH}/*.lib")

elseif (CMAKE_SYSTEM_NAME STREQUAL "Darwin" OR CMAKE_SYSTEM_NAME STREQUAL "Mac OS X")
    add_definitions(-DMACOS)
    set(FFMPEG_HEADERS_PATH "/usr/local/ffmpeg/include")
    set(FFMPEG_LIB_PATH "/usr/local/ffmpeg/lib")

    set(SDL_HEADERS_PATH "/opt/homebrew/Cellar/sdl2/2.30.8/include/SDL2")
    set(SDL_LIB_PATH "/opt/homebrew/Cellar/sdl2/2.30.8/lib")

    set(XLIB_HEADERS_PATH "/Users/wong/Library/xlib/include")
    set(XLIB_LIBS_PATH "/Users/wong/Library/xlib/lib")

    file(GLOB FFMPEG_LIBS "${FFMPEG_LIB_PATH}/*.dylib")
    file(GLOB SDL_LIBS "${SDL_LIB_PATH}/*.dylib")
    file(GLOB XLIB "${XLIB_LIBS_PATH}/*.dylib")

endif ()

include_directories(${SDL_HEADERS_PATH})
include_directories(${XLIB_HEADERS_PATH})

file(GLOB SRC_FILE ${CMAKE_SOURCE_DIR}/*.*)

add_executable(${PROJECT_NAME} ${SRC_FILE})
target_link_libraries(${PROJECT_NAME} ${SDL_LIBS} ${XLIB})
