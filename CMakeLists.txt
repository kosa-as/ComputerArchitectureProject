cmake_minimum_required(VERSION 3.28)
project(MIPSsim)

# 设置 C++ 标准为 C++17（代码使用了 std::optional 需要 C++17）
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 添加可执行文件
add_executable(MIPSsim MIPSsim.cpp)

