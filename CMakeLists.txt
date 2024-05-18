cmake_minimum_required(VERSION 3.9)
project(CryptoFace)

set(CMAKE_CXX_STANDARD 11)

#find_library(OpenCV)
find_package(OpenCV)

include_directories(${OpenCV_INCLUDE_DIRS})
set(CMAKE_CXX_STANDARD 11)

add_executable(CryptoFace main.cpp)

target_link_libraries(CryptoFace ${OpenCV_LIBS})