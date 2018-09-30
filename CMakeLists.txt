cmake_minimum_required(VERSION 3.1)
project(MicroProfile)

include(${CMAKE_CURRENT_LIST_DIR}/Microprofile.cmake)

include_directories(${CMAKE_CURRENT_LIST_DIR})

set(DEBUG_C_FLAGS "-g -O0 -Wno-invalid-offsetof -Wall -Werror")
add_definitions(-DMICROPROFILE_USE_CONFIG)

option(USE_WEBSERVER "Use OpenGL Loader" ON)
option(USE_UI "Use OpenGL Loader" OFF)
option(USE_GPUTIMERS "Use OpenGL Loader" OFF)

configure_file(${CMAKE_CURRENT_LIST_DIR}/microprofile.config.h.in ${CMAKE_CURRENT_LIST_DIR}/microprofile.config.h)

set(CMAKE_C_FLAGS                "${CMAKE_C_FLAGS} ${DEBUG_C_FLAGS}")
set(CMAKE_CXX_FLAGS              "${CMAKE_CXX_FLAGS} ${DEBUG_C_FLAGS}")

add_library(
    Microprofile
    STATIC
        ${SOURCES_MICROPROFILE}
)
