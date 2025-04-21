# 计算机体系结构项目

## 项目概述

本仓库包含计算机体系结构课程的项目实现代码。项目主要关注MIPS指令集的模拟实现，分为两个阶段（分支）：proj1和proj2。

## 分支说明

### proj1 分支

proj1实现了一个基础的MIPS指令集模拟器，能够：
- 读取二进制指令文件
- 进行指令反汇编
- 模拟执行MIPS指令


### proj2 分支

proj2在proj1的基础上进行了扩展，增加了更多高级功能：
- 添加了流水线执行的模拟
- 添加了数据冒险的检测与处理
- 模拟了`scoreboard`算法的实现

## 获取代码

```bash
# 克隆仓库
git clone https://github.com/kosa-as/ComputerArchitectureProject.git
cd ComputerArchitectureProject

# 切换到proj1分支
git checkout proj1

# 或切换到proj2分支
git checkout proj2
```

## 开发环境

- 操作系统：Ubuntu 24.04
- 编译器：GCC/G++ 支持 C++20 标准
- 构建工具：CMake (最低版本 3.10.0)
- 依赖库：C++ 标准库

## 编译与运行

请参考各分支中的README.md获取详细的编译和运行指南。

基本步骤：
```bash
mkdir build
cd build
cmake ..
make
```

## 说明文档

每个分支都有单独的README.md文件，详细说明了该分支的具体实现、功能特性和使用方法。请切换到相应分支查看详细文档。

## 项目贡献

欢迎提交问题和改进建议。如有任何疑问，请通过GitHub Issues进行反馈。 