# 计算机体系机构Project1说明

## 项目概述

本项目实现了一个简单的 MIPS 指令集模拟器，能够读取二进制指令文件，进行反汇编并模拟执行指令，输出执行过程中的寄存器和内存状态。本项目代码已上传至github，地址为：https://github.com/kosa-as/ComputerArchitectureProject/tree/proj1

## 开发环境

- 操作系统：Ubuntu 24.04
- 编译器：GCC/G++ 支持 C++20 标准
- 构建工具：CMake (最低版本 3.10.0)
- 依赖库：C++ 标准库

## 项目结构

```README.md
proj1/
├── CMakeLists.txt        # CMake 构建配置文件
├── MIPSsim.h             # 头文件，包含类和结构体定义
├── MIPSsim.cpp           # 源文件，包含实现代码
├── main.cpp              # 主函数入口
├── sample.txt            # 输入的二进制指令文件
├── mydisassembly.txt     # 输出的反汇编结果
└── mysimulation.txt      # 输出的模拟执行结果
```

## 功能特性

1. **指令解析**：将二进制指令解析为对应的 MIPS 汇编指令
2. **指令分类**：支持四类指令格式
   - Category 1: J, BEQ, BGTZ, BREAK, SW, LW
   - Category 2: ADD, SUB, MUL, AND, OR, XOR, NOR
   - Category 3: ADDI, ANDI, ORI, XORI
   - Category 4: DATA (数据段)
3. **模拟执行**：模拟执行指令并更新寄存器和内存状态
4. **状态输出**：每个周期输出当前指令、寄存器和内存状态

## 部署
```bash
git clone https://github.com/kosa-as/ComputerArchitectureProject.git
cd ComputerArchitectureProject
git checkout proj1
```

## 编译与运行

### 编译

```bash
mkdir build
cd build
cmake ..
make
```

### 运行

```bash
./proj1
diff -w -B ../mydisassembly.txt ../disassembly.txt
diff -w -B ../mysimulation.txt ../simulation.txt
```

默认情况下，程序会读取 `../sample.txt` 文件，并输出到 `../mydisassembly.txt` 和 `../mysimulation.txt`。在执行`diff`命令之后，没有输出，符合预期。

## 代码实现

### 主要类和结构体

1. **Instruction 结构体**：表示一条 MIPS 指令
   - 包含指令类型、二进制表示、地址等信息
   - 提供指令解析方法

2. **MIPSsim 类**：MIPS 模拟器的核心类
   - 管理指令列表、寄存器和内存
   - 提供指令执行方法
   - 处理文件输入输出

### 关键算法

1. **指令解析**：根据指令的前3位确定指令类别，然后根据操作码确定具体指令
2. **指令执行**：使用函数指针或 lambda 表达式实现不同指令的执行逻辑
3. **内存管理**：使用向量存储数据段，支持加载和存储操作

### 设计亮点

1. **函数式编程**：使用 `std::function` 和 lambda 表达式实现指令分发
2. **现代 C++ 特性**：使用 C++20 标准，包括 `std::optional`、模板等特性
3. **错误处理**：使用异常机制处理文件读写和指令执行错误

