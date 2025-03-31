cmake_minimum_required(VERSION 3.10.0)
project(proj1 VERSION 0.1.0 LANGUAGES C CXX)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# 添加头文件目录
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

# 添加源文件
add_executable(proj1 MIPSsim.cpp)

