cmake_minimum_required(VERSION 3.29)
project(xlib)

add_definitions(-DUNICODE -D_UNICODE)

set(CMAKE_CXX_STANDARD 20)

add_definitions(-D__STDC_CONSTANT_MACROS)
add_definitions(-DHAVE_FFMPEG)
add_definitions(-DHAVE_SDL2)
add_definitions(-DHAVE_OPENGL)

find_package(OpenGL REQUIRED)
file(GLOB SOURCE_FILE "${CMAKE_SOURCE_DIR}/*.cpp" "${CMAKE_SOURCE_DIR}/*.hpp")
add_library(${PROJECT_NAME} SHARED ${SOURCE_FILE})

#set(LIB_OUT_PATH "${CMAKE_BINARY_DIR}/${PROJECT_NAME}/lib")

if (WIN32)
    add_definitions(-DWINDOWS)
    # 使用 MSVC 时指定字符集
    if (MSVC)
        target_compile_options(${PROJECT_NAME} PRIVATE "/utf-8")
    endif()

    set(LIB_OUT_ROOT_PATH "C:/xlib")
    set(LIB_OUT_PATH "${LIB_OUT_ROOT_PATH}/lib")

    set(FFMPEG_HEADERS_PATH "C:/libffmpeg/include")
    set(FFMPEG_LIB_PATH "C:/libffmpeg/bin")

    set(SDL_HEADERS_PATH "C:/SDL2-2.30.9/include")
    set(SDL_LIB_PATH "C:/SDL2-2.30.9/lib/x64")

    set(Vulkan_INCLUDE_DIR "C:/VulkanSDK/1.3.290.0/Include")
    set(Vulkan_LIBRARY "C:/VulkanSDK/1.3.290.0/Lib/vulkan-1.lib")

    file(GLOB FFMPEG_LIBS "${FFMPEG_LIB_PATH}/*.lib")
    file(GLOB SDL_LIBS "${SDL_LIB_PATH}/*.lib")

    target_compile_definitions(${PROJECT_NAME} PRIVATE XLIB_EXPORTS)

elseif (CMAKE_SYSTEM_NAME STREQUAL "Darwin" OR CMAKE_SYSTEM_NAME STREQUAL "Mac OS X")
    add_definitions(-DMACOS)

    set(LIB_OUT_ROOT_PATH "/Users/wong/Library/xlib")
    set(LIB_OUT_PATH "${LIB_OUT_ROOT_PATH}/lib")

    set(FFMPEG_HEADERS_PATH "/usr/local/ffmpeg/include")
    set(FFMPEG_LIB_PATH "/usr/local/ffmpeg/lib")

    set(SDL_HEADERS_PATH "/opt/homebrew/Cellar/sdl2/2.30.9/include/SDL2")
    set(SDL_LIB_PATH "/opt/homebrew/Cellar/sdl2/2.30.9/lib")

    set(Vulkan_INCLUDE_DIR "/Users/wong/VulkanSDK/1.3.283.0/macOS/include")
    set(Vulkan_LIBRARY "/Users/wong/VulkanSDK/1.3.283.0/macOS/lib/libvulkan.dylib")

    file(GLOB FFMPEG_LIBS "${FFMPEG_LIB_PATH}/*.dylib")
    file(GLOB SDL_LIBS "${SDL_LIB_PATH}/*.dylib")

endif ()

set_target_properties(${PROJECT_NAME} PROPERTIES LIBRARY_OUTPUT_DIRECTORY ${LIB_OUT_PATH})
set_target_properties(${PROJECT_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${LIB_OUT_PATH})
set_target_properties(${PROJECT_NAME} PROPERTIES ARCHIVE_OUTPUT_DIRECTORY ${LIB_OUT_PATH})

include_directories(${FFMPEG_HEADERS_PATH})
include_directories(${SDL_HEADERS_PATH})
#link_directories(${FFMPEG_LIB_PATH})
#link_directories(${SDL_LIB_PATH})
find_package(Vulkan REQUIRED)
include_directories(${Vulkan_INCLUDE_DIR})

#set(TARGET_INCLUDE_PATH "${CMAKE_BINARY_DIR}/${PROJECT_NAME}/include")
set(TARGET_INCLUDE_PATH "${LIB_OUT_ROOT_PATH}/include")
if (NOT EXISTS ${TARGET_INCLUDE_PATH})
    file(MAKE_DIRECTORY ${TARGET_INCLUDE_PATH})
endif ()

#set(CMAKE_INSTALL_PREFIX "${CMAKE_BINARY_DIR}/${PROJECT_NAME}/")

# Install the library and header files
#install(TARGETS ${PROJECT_NAME}
#        LIBRARY DESTINATION lib
#        ARCHIVE DESTINATION lib
#        RUNTIME DESTINATION bin)
#
## Install header files
#install(DIRECTORY ${TARGET_INCLUDE_PATH} DESTINATION include)

# 查找所有 .hpp 文件
file(GLOB HEADER_FILES "${CMAKE_SOURCE_DIR}/*.hpp")
# 复制这些文件到指定目录
file(COPY ${HEADER_FILES} DESTINATION ${LIB_OUT_ROOT_PATH}/include/)

target_link_libraries(${PROJECT_NAME} ${FFMPEG_LIBS} ${SDL_LIBS} OpenGL::GL)
target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_SOURCE_DIR}})
