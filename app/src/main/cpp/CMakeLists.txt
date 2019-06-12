cmake_minimum_required(VERSION 3.4.1)

file(GLOB SOURCE *.cpp)

set(LIBS-PATH ${CMAKE_SOURCE_DIR}/../../../libs/${ANDROID_ABI})
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -L${LIBS-PATH}")

include_directories(${CMAKE_SOURCE_DIR}/include)

add_library(
        player-lib
        SHARED
        ${SOURCE})

target_link_libraries(
        player-lib
        avfilter avformat avcodec avutil swscale swresample
        z
        OpenSLES
        android
        log)