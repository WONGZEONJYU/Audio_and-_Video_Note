cmake_minimum_required(VERSION 3.30)
project(133_test_xplay)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

if (WIN32)
    add_definitions(-DWINDOWS)
    set(CMAKE_PREFIX_PATH "C:/Qt6/6.8.0/msvc2022_64")
    set(FFMPEG_HEADERS_PATH "C:/libffmpeg/include")
    set(FFMPEG_LIB_PATH "C:/libffmpeg/bin")

    set(SDL_HEADERS_PATH "C:/SDL2-2.30.8/include")
    set(SDL_LIB_PATH "C:/SDL2-2.30.8/lib/x64")

    set(Vulkan_INCLUDE_DIR "C:/VulkanSDK/1.3.290.0/Include")
    set(Vulkan_LIBRARY "C:/VulkanSDK/1.3.290.0/Lib/vulkan-1.lib")

    set(XLIB_HEADERS_PATH "C:/xlib/include")
    set(XLIB_LIBS_PATH "C:/xlib/lib")

    file(GLOB FFMPEG_LIBS "${FFMPEG_LIB_PATH}/*.lib")
    file(GLOB SDL_LIBS "${SDL_LIB_PATH}/*.lib")
    file(GLOB XLIBS "${XLIB_LIBS_PATH}/*.lib")

elseif (CMAKE_SYSTEM_NAME STREQUAL "Darwin" OR CMAKE_SYSTEM_NAME STREQUAL "Mac OS X")
    add_definitions(-DMACOS)

    set(CMAKE_PREFIX_PATH "/Users/wong/Qt/6.8.0/macos")

    set(FFMPEG_HEADERS_PATH "/usr/local/ffmpeg/include")
    set(FFMPEG_LIB_PATH "/usr/local/ffmpeg/lib")

    set(SDL_HEADERS_PATH "/opt/homebrew/Cellar/sdl2/2.30.8/include/SDL2")
    set(SDL_LIB_PATH "/opt/homebrew/Cellar/sdl2/2.30.8/lib")

    set(Vulkan_INCLUDE_DIR "/Users/wong/VulkanSDK/1.3.283.0/macOS/include")
    set(Vulkan_LIBRARY "/Users/wong/VulkanSDK/1.3.283.0/macOS/lib/libvulkan.dylib")

    set(XLIB_HEADERS_PATH "/Users/wong/Library/xlib/include")
    set(XLIB_LIBS_PATH "/Users/wong/Library/xlib/lib")

    file(GLOB FFMPEG_LIBS "${FFMPEG_LIB_PATH}/*.dylib")
    file(GLOB SDL_LIBS "${SDL_LIB_PATH}/*.dylib")
    file(GLOB XLIB "${XLIB_LIBS_PATH}/*.dylib")

endif ()

include_directories(${FFMPEG_HEADERS_PATH})
include_directories(${SDL_HEADERS_PATH})
include_directories(${XLIB_HEADERS_PATH})

set(QT_LIBS Core Gui Widgets)

find_package(Qt6 COMPONENTS
    ${QT_LIBS}
  REQUIRED)

file(GLOB SOURCE_FILE "${CMAKE_SOURCE_DIR}/*.cpp" "${CMAKE_SOURCE_DIR}/*.hpp")
add_executable(${PROJECT_NAME} ${SOURCE_FILE})

target_link_libraries(${PROJECT_NAME}
        ${FFMPEG_LIBS}
        ${SDL_LIBS}
        Qt::Core
        Qt::Gui
        Qt::Widgets
        ${XLIBS}
)

#以下代码是针对Windows平台用于拷贝动态库到运行目录下
if (WIN32 AND NOT DEFINED CMAKE_TOOLCHAIN_FILE)

    set(DEBUG_SUFFIX)
    if (MSVC AND CMAKE_BUILD_TYPE MATCHES "Debug")
        set(DEBUG_SUFFIX "d")
    endif()
    set(QT_INSTALL_PATH "${CMAKE_PREFIX_PATH}")
    if(NOT EXISTS "${QT_INSTALL_PATH}/bin")
        set(QT_INSTALL_PATH "${QT_INSTALL_PATH}/..")
        if(NOT EXISTS "${QT_INSTALL_PATH}/bin")
            set(QT_INSTALL_PATH "${QT_INSTALL_PATH}/..")
        endif()
    endif()
    if(EXISTS "${QT_INSTALL_PATH}/plugins/platforms/qwindows${DEBUG_SUFFIX}.dll")
        add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E make_directory
                "$<TARGET_FILE_DIR:${PROJECT_NAME}>/plugins/platforms/")
        add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy
                "${QT_INSTALL_PATH}/plugins/platforms/qwindows${DEBUG_SUFFIX}.dll"
                "$<TARGET_FILE_DIR:${PROJECT_NAME}>/plugins/platforms/")
    endif()
    foreach(QT_LIB ${QT_LIBS} )
        add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy
                "${QT_INSTALL_PATH}/bin/Qt6${QT_LIB}${DEBUG_SUFFIX}.dll"
                "$<TARGET_FILE_DIR:${PROJECT_NAME}>")
    endforeach(QT_LIB)

endif()
