cmake_minimum_required(VERSION 3.29)
project(xlib)

set(CMAKE_CXX_STANDARD 20)

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
    include_directories(/opt/homebrew/opt/llvm/include)
    include_directories(/usr/local/ffmpeg/include)
    include_directories(/opt/homebrew/Cellar/sdl2/2.30.8/include/SDL2)

    link_directories(/usr/local/ffmpeg/lib)
    link_directories(/opt/homebrew/Cellar/sdl2/2.30.8/lib)

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
        SDL2
)

set(SOURCE_FILE
xhelper.cpp
xhelper.hpp
xavframe.cpp
xavframe.hpp
xavpacket.cpp
xavpacket.hpp
xcodec.cpp
xcodec.hpp
xdecode.cpp
xdecode.hpp
xencode.cpp
xencode.hpp
xformat.cpp
xformat.hpp
xdemux.cpp
xdemux.hpp
xmux.cpp
xmux.hpp
xtools.cpp
xtools.hpp
xdemuxtask.cpp
xdemuxtask.hpp
xcodec_parameters.cpp
xcodec_parameters.hpp
xdecodetask.cpp
xdecodetask.h
xsdl.cpp
xsdl.hpp
xvideo_view.cpp
xvideo_view.hpp
xmuxtask.cpp
xmuxtask.hpp
xsonic.cpp
xprivate_sonic.cpp
xsonic.hpp
xswscale.cpp
xswscale.hpp
xswrsample.cpp
xswrsample.hpp)

set(static_lib ${PROJECT_NAME}_static)
add_library(${static_lib} STATIC ${SOURCE_FILE})
set_target_properties(${static_lib} PROPERTIES OUTPUT_NAME ${PROJECT_NAME})
set_target_properties(${static_lib} PROPERTIES
        ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/${PROJECT_NAME}/lib/")

set(shared_lib ${PROJECT_NAME}_shared)
add_library(${shared_lib} SHARED ${SOURCE_FILE})
set_target_properties(${shared_lib} PROPERTIES OUTPUT_NAME ${PROJECT_NAME})
set_target_properties(${shared_lib} PROPERTIES
        LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/${PROJECT_NAME}/lib/")

set(CMAKE_INSTALL_PREFIX "${CMAKE_BINARY_DIR}/${PROJECT_NAME}/" )

target_include_directories(${static_lib} PUBLIC include)


file(MAKE_DIRECTORY "${CMAKE_BINARY_DIR}/${PROJECT_NAME}/include")

# Install the library and header files
install(TARGETS ${static_lib}
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib
        RUNTIME DESTINATION bin
)

# Install header files
install(DIRECTORY ${CMAKE_BINARY_DIR}/${PROJECT_NAME}/include/ DESTINATION include)

# 查找所有 .hpp 文件
file(GLOB HEADER_FILES "${CMAKE_SOURCE_DIR}/*.hpp")

# 复制这些文件到指定目录
file(COPY ${HEADER_FILES} DESTINATION ${CMAKE_BINARY_DIR}/${PROJECT_NAME}/include/)

target_link_libraries(${static_lib} ${FFMPEG_LIBS})
target_link_libraries(${shared_lib} ${FFMPEG_LIBS})
