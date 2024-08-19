cmake_minimum_required(VERSION 3.28)
project(XPlay2)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

if (WIN32 AND NOT DEFINED CMAKE_TOOLCHAIN_FILE)
    add_definitions(-DWINDOWS)
    #set(CMAKE_PREFIX_PATH "C:/Qt6/6.4.2/mingw_64/")
    set(CMAKE_PREFIX_PATH "C:/Qt6/6.7.2/mingw_64/")
    include_directories(c:/ffmepg-7.1/include)
    include_directories(c:/SDL2-2.28.5-VC/include)

    link_directories(c:/ffmepg-7.1/bin)
    link_directories(c:/SDL2-2.28.5-VC/lib/x64)

    set(Vulkan_INCLUDE_DIR "C:/VulkanSDK/1.3.290.0/Include")
    set(Vulkan_LIBRARY "C:/VulkanSDK/1.3.290.0/Lib/vulkan-1.lib")
    find_package(Vulkan REQUIRED)
    include_directories(${Vulkan_INCLUDE_DIR})

elseif (CMAKE_SYSTEM_NAME STREQUAL "Darwin" OR CMAKE_SYSTEM_NAME STREQUAL "Mac OS X")
    add_definitions(-DMACOS)
    #set(CMAKE_PREFIX_PATH "/Users/wong/Qt/6.4.2/macos")
    set(CMAKE_PREFIX_PATH "/Users/wong/Qt/6.7.2/macos")
    include_directories(/usr/local/ffmpeg/include)
    include_directories(/opt/homebrew/Cellar/sdl2/2.30.1/include/SDL2)

    link_directories(/usr/local/ffmpeg/lib)
    link_directories(/opt/homebrew/Cellar/sdl2/2.30.1/lib)

    set(Vulkan_INCLUDE_DIR "/Users/wong/VulkanSDK/1.3.283.0/macOS/include")
    set(Vulkan_LIBRARY "/Users/wong/VulkanSDK/1.3.283.0/macOS/lib/libvulkan.dylib")
    find_package(Vulkan REQUIRED)
    include_directories(${Vulkan_INCLUDE_DIR})

endif()

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

add_definitions(-D__STDC_CONSTANT_MACROS)
add_definitions(-DHAVE_FFMPEG)
add_definitions(-DHAVE_OPENGL)
include_directories(${PROJECT_SOURCE_DIR})
include_directories(${PROJECT_SOURCE_DIR}/src)

set(SRC_CODE
        src/XDemux.cpp
        src/XDemux.hpp
        src/XHelper.hpp
        src/XHelper.cpp
        src/XSwrContext.cpp
        src/XSwrContext.hpp
        src/XAVPacket.cpp
        src/XAVPacket.hpp
        src/XAVFrame.cpp
        src/XAVFrame.hpp
        src/XAVCodecParameters.cpp
        src/XAVCodecParameters.hpp
        src/XDecode.cpp
        src/XDecode.hpp
        src/XResample.cpp
        src/XResample.hpp
        src/XAudioPlay.cpp
        src/XAudioPlay.hpp
        src/QXAudioPlay.cpp
        src/QXAudioPlay.hpp
        src/XAudioThread.cpp
        src/XAudioThread.hpp
        src/IVideoCall.hpp
        src/XVideoThread.cpp
        src/XVideoThread.hpp
        src/XDecodeThread.hpp
        src/XDecodeThread.cpp
        src/XDemuxThread.cpp
        src/XDemuxThread.hpp
        XQSliderWidget.cpp
        XQSliderWidget.hpp
)

set(QT__LIB
    Core
    Gui
    Widgets
    OpenGL
    OpenGLWidgets
    Multimedia
    Network
)

find_package(OpenGL REQUIRED)
find_package(Qt6 COMPONENTS
        ${QT__LIB}
        REQUIRED)

add_executable(XPlay2
        main.cpp
        XPlay2Widget.cpp
        XPlay2Widget.hpp
        XPlay2Widget.ui
        XVideoWidget.cpp
        XVideoWidget.hpp
        ${SRC_CODE}
)
target_link_libraries(XPlay2
        Qt::Core
        Qt::Gui
        Qt::Widgets
        Qt6::OpenGL
        Qt6::OpenGLWidgets
        Qt6::Multimedia
        Qt6::Network
        OpenGL::GL
        ${FFMPEG_LIBS}
        ${Vulkan_LIBRARY}
)

if (WIN32 AND NOT DEFINED CMAKE_TOOLCHAIN_FILE)
    set(DEBUG_SUFFIX)
    if (MSVC AND CMAKE_BUILD_TYPE MATCHES "Debug")
        set(DEBUG_SUFFIX "d")
    endif ()
    set(QT_INSTALL_PATH "${CMAKE_PREFIX_PATH}")
    if (NOT EXISTS "${QT_INSTALL_PATH}/bin")
        set(QT_INSTALL_PATH "${QT_INSTALL_PATH}/..")
        if (NOT EXISTS "${QT_INSTALL_PATH}/bin")
            set(QT_INSTALL_PATH "${QT_INSTALL_PATH}/..")
        endif ()
    endif ()
    if (EXISTS "${QT_INSTALL_PATH}/plugins/platforms/qwindows${DEBUG_SUFFIX}.dll")
        add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E make_directory
                "$<TARGET_FILE_DIR:${PROJECT_NAME}>/plugins/platforms/")
        add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy
                "${QT_INSTALL_PATH}/plugins/platforms/qwindows${DEBUG_SUFFIX}.dll"
                "$<TARGET_FILE_DIR:${PROJECT_NAME}>/plugins/platforms/")
    endif ()
    foreach (QT_LIB ${QT__LIB})
        add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy
                "${QT_INSTALL_PATH}/bin/Qt6${QT_LIB}${DEBUG_SUFFIX}.dll"
                "$<TARGET_FILE_DIR:${PROJECT_NAME}>")
    endforeach (QT_LIB)
endif ()
