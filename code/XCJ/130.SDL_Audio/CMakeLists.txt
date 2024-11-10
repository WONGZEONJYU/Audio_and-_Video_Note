cmake_minimum_required(VERSION 3.29)
project(130_SDL_Audio)

set(CMAKE_CXX_STANDARD 20)

if (WIN32)
set(SDL_HEADERS_PATH "C:/SDL2-2.30.8/include")
set(SDL_LIB_PATH "C:/SDL2-2.30.8/lib/x64")

elseif (CMAKE_SYSTEM_NAME STREQUAL "Darwin" OR CMAKE_SYSTEM_NAME STREQUAL "Mac OS X")
set(SDL_HEADERS_PATH "/opt/homebrew/Cellar/sdl2/2.30.8/include/SDL2")
set(SDL_LIB_PATH "/opt/homebrew/Cellar/sdl2/2.30.8/lib")

endif()

include_directories(${SDL_HEADERS_PATH})
link_directories(${SDL_LIB_PATH})

add_executable(${PROJECT_NAME} main.cpp)
target_link_libraries(${PROJECT_NAME} SDL2)
