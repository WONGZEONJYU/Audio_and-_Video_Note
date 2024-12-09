cmake_minimum_required(VERSION 3.30)
project(test_onvif_c++)

add_compile_definitions(-DWITH_NONAMESPACES)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_C_STANDARD 17)

include_directories("${CMAKE_SOURCE_DIR}/")
include_directories("${CMAKE_SOURCE_DIR}/gsoap/")

add_executable(test_onvif main.cpp
        ${CMAKE_SOURCE_DIR}/gsoap/stdsoap2.cpp
        ${CMAKE_SOURCE_DIR}/gsoap/stdsoap2.h
        ${CMAKE_SOURCE_DIR}/gsoap/custom/struct_timeval.cpp
        ${CMAKE_SOURCE_DIR}/gsoap/custom/struct_timeval.h
        soapC.cpp
        soapClient.cpp
        soapH.h
        soapStub.h)
