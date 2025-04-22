# MIPSsim - MIPS处理器模拟器

## 项目简介

MIPSsim是一个模拟MIPS处理器执行指令的工具。本模拟器实现了MIPS指令集的一个子集，并模拟了五级流水线的执行过程，包括指令获取/译码、发射、执行、访存和写回阶段。

## 功能特点

- 支持基本MIPS指令（ADD, SUB, MUL, AND, OR, XOR, NOR等）
- 支持立即数指令（ADDI, ANDI, ORI, XORI）
- 支持分支指令（J, BEQ, BGTZ）
- 支持内存访问指令（LW, SW）
- 处理各种流水线冒险（结构冒险、数据冒险等）
- 生成详细的模拟执行报告

## 系统要求
- 操作系统：Ubuntu 24.04
- C++20或更高版本
- CMake 3.10或更高版本

## 部署指南

### 1. 克隆项目

```bash
git clone https://github.com/kosa-as/ComputerArchitectureProject.git
git checkout proj2
cd ComputerArchitectureProject
```

### 2. 编译项目

```bash
mkdir build
cd build
cmake ..
make
```

### 3. 运行模拟器

```bash
./MIPSsim <input_file>
```

### 4. 查看结果

执行完成后，会在当前目录下生成以下文件：

- `generated_disassembly.txt`：生成的汇编代码
- `generated_simulation.txt`：详细的模拟执行报告

