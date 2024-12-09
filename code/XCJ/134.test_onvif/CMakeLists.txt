cmake_minimum_required(VERSION 3.30)
project(134_test_onvif)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_C_STANDARD 23)

include_directories("${CMAKE_SOURCE_DIR}/")
include_directories("${CMAKE_SOURCE_DIR}/gsoap/")

add_executable(${PROJECT_NAME} main.c
        ${CMAKE_SOURCE_DIR}/gsoap/stdsoap2.c
        ${CMAKE_SOURCE_DIR}/gsoap/stdsoap2.h
        ${CMAKE_SOURCE_DIR}/gsoap/custom/struct_timeval.c
        ${CMAKE_SOURCE_DIR}/gsoap/custom/struct_timeval.h
        soapC.c
        soapClient.c
        soapH.h
        soapStub.h)
