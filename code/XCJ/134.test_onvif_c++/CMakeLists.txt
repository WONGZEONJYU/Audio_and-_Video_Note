cmake_minimum_required(VERSION 3.30)
project(test_onvif_c++)

add_compile_definitions(-DWITH_NONAMESPACES)
add_compile_definitions(-DWITH_OPENSSL)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_C_STANDARD 17)

include_directories("${CMAKE_SOURCE_DIR}/")
include_directories("${CMAKE_SOURCE_DIR}/gsoap/")
include_directories("${CMAKE_SOURCE_DIR}/gsoap/plugin/")

if (WIN32)

elseif (CMAKE_SYSTEM_NAME STREQUAL "Darwin" OR CMAKE_SYSTEM_NAME STREQUAL "Mac OS X")
    set(OPENSSL_LIB_PATH "/Users/wong/Library/openssl/lib/lib")
    set(OPENSSL_HEADERS_PATH "/Users/wong/Library/openssl/lib/include")

    file(GLOB OPENSSL_LIBS "${OPENSSL_LIB_PATH}/*.dylib")
    list(FILTER OPENSSL_LIBS EXCLUDE REGEX ".*.3.dylib")
    include_directories(${OPENSSL_HEADERS_PATH})
endif()

add_executable(${PROJECT_NAME}
        main.cpp
        soapC.cpp
        soapClient.cpp
        soapH.h
        soapStub.h
        xonvif.cpp
        xonvif.hpp
        ${CMAKE_SOURCE_DIR}/gsoap/dom.cpp
        ${CMAKE_SOURCE_DIR}/gsoap/stdsoap2.cpp
        ${CMAKE_SOURCE_DIR}/gsoap/stdsoap2.h
        ${CMAKE_SOURCE_DIR}/gsoap/custom/struct_timeval.cpp
        ${CMAKE_SOURCE_DIR}/gsoap/custom/struct_timeval.h
        ${CMAKE_SOURCE_DIR}/gsoap/plugin/smdevp.cpp
        ${CMAKE_SOURCE_DIR}/gsoap/plugin/smdevp.h
        ${CMAKE_SOURCE_DIR}/gsoap/plugin/wsaapi.cpp
        ${CMAKE_SOURCE_DIR}/gsoap/plugin/wsaapi.h
        ${CMAKE_SOURCE_DIR}/gsoap/plugin/wsseapi.cpp
        ${CMAKE_SOURCE_DIR}/gsoap/plugin/wsseapi.h
        ${CMAKE_SOURCE_DIR}/gsoap/plugin/mecevp.cpp
        ${CMAKE_SOURCE_DIR}/gsoap/plugin/mecevp.h
        ${CMAKE_SOURCE_DIR}/gsoap/plugin/threads.cpp
        ${CMAKE_SOURCE_DIR}/gsoap/plugin/threads.h
)

target_link_libraries(${PROJECT_NAME} ${OPENSSL_LIBS})
message(STATUS "msg : ${OPENSSL_LIBS}")