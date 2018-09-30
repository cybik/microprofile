cmake_minimum_required(VERSION 3.1)
project(MicroProfile)

include(${CMAKE_CURRENT_LIST_DIR}/Microprofile.cmake)

include_directories(${CMAKE_CURRENT_LIST_DIR})

set(DEBUG_C_FLAGS "-g -O0 -Wno-invalid-offsetof -std=c++11 -Wno-c++11-extensions")

add_definitions(-D_MICROPROFILE_CMAKE)
option(MICROPROFILE_WEBSERVER "Microprofile - Webserver" ON)
option(MICROPROFILE_UI "Microprofile - UI" OFF)

option(MICROPROFILE_GPU_TIMERS "Microprofile - GPU timers" OFF)
option(MICROPROFILE_GPU_TIMERS_OGL "Microprofile - GPU timers - OpenGL" OFF)
option(MICROPROFILE_GPU_TIMERS_D3D11 "Microprofile - GPU timers - D3D11" OFF)
option(MICROPROFILE_GPU_TIMERS_D3D12 "Microprofile - GPU timers - D3D12" OFF)
option(MICROPROFILE_GPU_TIMERS_VK "Microprofile - GPU timers - Vulkan" OFF)

configure_file(${CMAKE_CURRENT_LIST_DIR}/microprofile.config.h.in ${CMAKE_CURRENT_LIST_DIR}/microprofile.config.h)

set(CMAKE_C_FLAGS                "${CMAKE_C_FLAGS} ${DEBUG_C_FLAGS}")
set(CMAKE_CXX_FLAGS              "${CMAKE_CXX_FLAGS} ${DEBUG_C_FLAGS}")

add_library(
    Microprofile
    STATIC
        ${SOURCES_MICROPROFILE}
        ${HEADERS_MICROPROFILE}
)
