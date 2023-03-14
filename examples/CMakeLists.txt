cmake_minimum_required(VERSION 3.23)
project(gitlike VERSION 1.0.0 DESCRIPTION "Sample build program")
add_executable(gitlike gitlike.cxx)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(args REQUIRED CONFIG)

target_link_libraries(gitlike taywee::args)
