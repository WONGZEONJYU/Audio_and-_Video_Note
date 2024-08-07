cmake_minimum_required(VERSION 3.28.1)
project(player)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

if (WIN32 AND NOT DEFINED CMAKE_TOOLCHAIN_FILE)
    add_definitions(-DWINDOWS)
    set(CMAKE_PREFIX_PATH "C:/Qt6/6.4.2/mingw_64/")
    include_directories(c:/ffmepg-7.1/include)
    include_directories(c:/SDL2-2.28.5-VC/include)

    link_directories(c:/ffmepg-7.1/bin)
    link_directories(c:/SDL2-2.28.5-VC/lib/x64)
elseif (CMAKE_SYSTEM_NAME STREQUAL "Darwin" OR CMAKE_SYSTEM_NAME STREQUAL "Mac OS X")
    add_definitions(-DMACOS)
    set(CMAKE_PREFIX_PATH "/Users/wong/Qt/6.4.2/macos")
    include_directories(/usr/local/ffmpeg/include)
    include_directories(/opt/homebrew/Cellar/sdl2/2.30.1/include/SDL2)

    link_directories(/usr/local/ffmpeg/lib)
    link_directories(/opt/homebrew/Cellar/sdl2/2.30.1/lib)
endif()

include_directories(${PROJECT_SOURCE_DIR})
include_directories(${PROJECT_SOURCE_DIR}/src)

find_package(Qt6 COMPONENTS
        Core
        Gui
        Widgets
        REQUIRED)

add_executable(player main.cpp
        mainwindow.cpp
        mainwindow.hpp
        mainwindow.ui
        ctrlbar.cpp
        ctrlbar.hpp
        ctrlbar.ui
        titlebar.cpp
        titlebar.hpp
        titlebar.ui
        playlistbox.cpp
        playlistbox.hpp
        playlistbox.ui
        displaybox.cpp
        displaybox.hpp
        displaybox.ui
        src/MessageQueue.cpp
        src/MessageQueue.hpp
        src/IjkMediaPlayer.cpp
        src/IjkMediaPlayer.hpp
        src/MessageAbstract.hpp
        src/FFPlay.cpp
        src/FFPlay.hpp
        src/ff_ffmsg.h
        src/IjkMediaPlayer_State.h
        src/ff_ffplay_def.hpp
        src/ff_ffplay_def.cpp
        src/ShareAVPacket.cpp
        src/ShareAVPacket.hpp
        src/ShareAVFrame.cpp
        src/ShareAVFrame.hpp
        src/AVHelper.h
        src/AVHelper.cpp
        src/DecoderAbstract.cpp
        src/DecoderAbstract.hpp
        src/VideoDecoder.cpp
        src/VideoDecoder.hpp
        src/AudioDecoder.cpp
        src/AudioDecoder.hpp
        src/SwrContext_t.cpp
        src/SwrContext_t.hpp
        src/FFPlay_th.cpp
        src/FFPlay_cb.cpp
        src/Sws_Context.cpp
        src/Sws_Context.hpp
        src/VImage.hpp
)

set(FFMPEG_LIBS avcodec avdevice avfilter avformat avutil postproc swresample swscale SDL2)

target_link_libraries(player
        Qt6::Core
        Qt6::Gui
        Qt6::Widgets
        ${FFMPEG_LIBS}
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
    foreach (QT_LIB Core Gui Widgets)
        add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy
                "${QT_INSTALL_PATH}/bin/Qt6${QT_LIB}${DEBUG_SUFFIX}.dll"
                "$<TARGET_FILE_DIR:${PROJECT_NAME}>")
    endforeach (QT_LIB)
endif ()
