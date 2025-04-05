/* On my honor, I have neither given nor received unauthorized aid on this assignment */
#include <string>
#include <vector>
#include <fstream>
#include <stdexcept>
#include <iostream>
#include <cstdint>
#include <unordered_map>
#include <array>
#include <bitset>
#include <optional>
#include <functional>
#include <iomanip>

constexpr uint32_t base_address = 0x00000080;

/*
    @param value: 寄存器编号
    @param args: 寄存器编号
    @return: 寄存器编号对应的名称拼接的字符串
    例如：int_to_string_reg(0) = "R0"
          int_to_string_reg(0, 1) = "R0, R1"
          int_to_string_reg(0, 1, 2) = "R0, R1, R2"
    这里使用了折叠表达式的模板编程，要求C++17及以上
*/
template<typename T0, typename ...T> 
std::string int_to_string_reg(T0 value, T... args) {
    if constexpr (sizeof...(args) == 0) {
        return "R" + std::to_string(value);
    } else {
        return "R" + std::to_string(value) + ", " + int_to_string_reg(args...);
    }
}
enum class InstructionType : unsigned int{
    ADD,
    SUB,
    MUL,
    AND,
    OR,
    XOR,
    NOR,
    ADDI,
    ANDI,
    ORI,
    XORI,
    J,
    BEQ,
    BGTZ,
    SW,
    LW,
    BREAK,
    DATA,
    UNKNOWN
};

/*
    @param instruction: 指令的二进制表示
    @param type: 指令类型
    @param instruction_detail: 指令的详细信息(地址加指令)
    @param print_instruction_detail: 打印到txt的指令的详细信息
    @param address: 指令的地址
    @param _rs: 指令的源寄存器(可选)
    @param _rt: 指令的目标寄存器(可选)
    @param _rd: 指令的目的寄存器(可选)
    @param _immediate_value: 指令的立即数(可选)
    @param _offset: 指令的偏移量(可选)
    @param _value: data部分的值(可选)
*/
struct Instruction {
    uint32_t instruction;
    InstructionType type;
    std::string instruction_detail;
    std::string print_instruction_detail;
    uint32_t address;
    std::optional<uint32_t> _rs;
    std::optional<uint32_t> _rt;
    std::optional<uint32_t> _rd;
    std::optional<uint32_t> _immediate_value;
    std::optional<uint32_t> _offset;
    std::optional<int> _value;
    static std::unordered_map<unsigned int, std::string> category_1_map;
    static std::unordered_map<unsigned int, std::string> category_2_map;
    static std::unordered_map<unsigned int, std::string> category_3_map;
    void process_instruction();
    void process_category_1();
    void process_category_2();
    void process_category_3();
    void process_category_4();
};


// 这里声明了函数指针的类型，用于存储指令的执行函数。std::function是C++11标准库中的一个模板类，用于存储和调用函数对象。
using Func = std::function<uint32_t(Instruction&, uint32_t)>;

/*
    @param instructions: 指令列表
    @param input_filename: 输入文件名
    @param output_filename: 输出文件名
    @param simulation_filename: 模拟执行文件名
    @param registers: 寄存器数组
    @param data: 数据列表
    @param base_data_address: 数据段的起始地址
    @param select_instruction: 选择指令的函数
    @param is_break: 是否是break指令,区别数据段还是指令段
*/
class MIPSsim {
    std::vector<Instruction> instructions;
    std::string input_filename;
    std::string output_filename;
    std::string simulation_filename;
    std::array<int, 32> registers{};
    std::vector<int> data;
    uint32_t base_data_address;
    Func select_instruction(Instruction& inst);
    uint32_t _add(Instruction& inst, uint32_t pc);
    uint32_t _sub(Instruction& inst, uint32_t pc);
    uint32_t _mul(Instruction& inst, uint32_t pc);
    uint32_t _and(Instruction& inst, uint32_t pc);
    uint32_t _or(Instruction& inst, uint32_t pc);
    uint32_t _xor(Instruction& inst, uint32_t pc);
    uint32_t _nor(Instruction& inst, uint32_t pc);
    uint32_t _addi(Instruction& inst, uint32_t pc);
    uint32_t _andi(Instruction& inst, uint32_t pc);
    uint32_t _ori(Instruction& inst, uint32_t pc);
    uint32_t _xori(Instruction& inst, uint32_t pc);
    uint32_t _j(Instruction& inst, uint32_t pc);
    uint32_t _beq(Instruction& inst, uint32_t pc);
    uint32_t _bgtz(Instruction& inst, uint32_t pc);
    uint32_t _sw(Instruction& inst, uint32_t pc);
    uint32_t _lw(Instruction& inst, uint32_t pc);
    uint32_t _break(Instruction& inst, uint32_t pc);
    void print_registers();
    void print_data();
    public:
        static bool is_break;
        MIPSsim()=default;
        ~MIPSsim()=default;
        void excute(); 
        void read_from_file();
        void write_to_output_file();
        void write_to_simulation_file(int cycle, const std::string& instruction_detail);
        void set_input_filename(const std::string& filename);
        void set_output_filename(const std::string& filename);
        void set_simulation_filename(const std::string& filename);
};

InstructionType stringToInstruction(const std::string& str);

std::unordered_map<std::string, InstructionType> stringToInstructionMap = {
    {"ADD", InstructionType::ADD},
    {"SUB", InstructionType::SUB},
    {"MUL", InstructionType::MUL},
    {"AND", InstructionType::AND},
    {"OR", InstructionType::OR},
    {"XOR", InstructionType::XOR},
    {"NOR", InstructionType::NOR},
    {"ADDI", InstructionType::ADDI},
    {"ANDI", InstructionType::ANDI},
    {"ORI", InstructionType::ORI},
    {"XORI", InstructionType::XORI},
    {"J", InstructionType::J},
    {"BEQ", InstructionType::BEQ},
    {"BGTZ", InstructionType::BGTZ},
    {"SW", InstructionType::SW},
    {"LW", InstructionType::LW},
    {"BREAK", InstructionType::BREAK},
    {"DATA", InstructionType::DATA},
    {"UNKNOWN", InstructionType::UNKNOWN}
};

InstructionType stringToInstruction(const std::string& str){
    auto it = stringToInstructionMap.find(str);
    if (it != stringToInstructionMap.end()) {
        return it->second;
    }
    return InstructionType::UNKNOWN;  // 找不到返回 UNKNOWN
}

void MIPSsim::read_from_file() {
    std::ifstream infile(input_filename);
    std::string line;
    
    if(!infile.is_open()) {
        throw std::runtime_error("无法打开文件: " + input_filename);
    }
    int count = 0;
    while(std::getline(infile, line)) {
        if(line.empty()) continue;
        Instruction inst;
        inst.address = base_address + count * 4;
        inst.instruction = static_cast<uint32_t>(std::stoul(line, nullptr, 2));
        if(!is_break) {
            inst.process_instruction();
        }else{
            inst.process_category_4();
            if(inst._value.has_value()) {
                data.push_back(inst._value.value());
            }
        }
        count++;
        instructions.push_back(inst);
    }
    base_data_address = base_address + 4 * (instructions.size() - data.size());
    infile.close();
}

void MIPSsim::write_to_output_file() {
    std::ofstream outfile(output_filename);
    for(auto& inst : instructions) {
        outfile << inst.print_instruction_detail;
        if(&inst != &instructions.back()) {
            outfile << std::endl;
        }
    }
    outfile.close();
}

void MIPSsim::write_to_simulation_file(int cycle, const std::string& instruction_detail) {
    // 使用追加模式打开文件
    std::ofstream simulation_file(simulation_filename, std::ios::app);
    
    simulation_file << "--------------------"<< std::endl;
    simulation_file << "Cycle:" << cycle <<" "<< instruction_detail << std::endl;
    simulation_file << "Registers" << std::endl;
    for(int i = 0; i < 32; i += 8) {
        simulation_file << "R" << std::setfill('0') << std::setw(2) << i << ":\t";
        for(int j = 0; j < 8; j++) {
            simulation_file << registers[i + j] << "\t";
        }
        simulation_file << std::endl;
    }
    simulation_file << std::endl;
    simulation_file << "Data" << std::endl;

    int data_size = data.size();
    int row = (data_size + 7) / 8;  // 对data_size/8向上取整
    for(int i = 0; i < row; i++) {
        simulation_file << i*32 + base_data_address << ":\t";
        for(int j = i*8; j < (i+1)*8 && j < data_size; j++) {
            simulation_file << data[j] << "\t";
        }
        simulation_file << std::endl;
    }
    simulation_file.flush();
    simulation_file.close();
}

void MIPSsim::set_simulation_filename(const std::string& filename) {
    simulation_filename = filename;
}

void MIPSsim::set_input_filename(const std::string& filename) {
    input_filename = filename;
}

void MIPSsim::set_output_filename(const std::string& filename) {
    output_filename = filename;
}

/*
    @param inst: 指令
    @return: 指令的执行函数。由于_add这些函数声明在对象中，使用lamda函数来捕获对象来获取对于的操作
*/
Func MIPSsim::select_instruction(Instruction& inst) {
    switch(inst.type) {
        case InstructionType::ADD:
            return [this](Instruction& i, int pc) { return this->_add(i, pc); };
        case InstructionType::SUB:
            return [this](Instruction& i, int pc) { return this->_sub(i, pc); };
        case InstructionType::MUL:
            return [this](Instruction& i, int pc) { return this->_mul(i, pc); };
        case InstructionType::AND:
            return [this](Instruction& i, int pc) { return this->_and(i, pc); };
        case InstructionType::OR:
            return [this](Instruction& i, int pc) { return this->_or(i, pc); };
        case InstructionType::XOR:
            return [this](Instruction& i, int pc) { return this->_xor(i, pc); };
        case InstructionType::NOR:
            return [this](Instruction& i, int pc) { return this->_nor(i, pc); };
        case InstructionType::ADDI:
            return [this](Instruction& i, int pc) { return this->_addi(i, pc); };
        case InstructionType::ANDI:
            return [this](Instruction& i, int pc) { return this->_andi(i, pc); };
        case InstructionType::ORI:
            return [this](Instruction& i, int pc) { return this->_ori(i, pc); };
        case InstructionType::XORI:
            return [this](Instruction& i, int pc) { return this->_xori(i, pc); };
        case InstructionType::J:
            return [this](Instruction& i, int pc) { return this->_j(i, pc); };
        case InstructionType::BEQ:
            return [this](Instruction& i, int pc) { return this->_beq(i, pc); };
        case InstructionType::BGTZ:
            return [this](Instruction& i, int pc) { return this->_bgtz(i, pc); };
        case InstructionType::SW:
            return [this](Instruction& i, int pc) { return this->_sw(i, pc); };
        case InstructionType::LW:
            return [this](Instruction& i, int pc) { return this->_lw(i, pc); };
        case InstructionType::BREAK:
            return [this](Instruction& i, int pc) { return this->_break(i, pc); };
        default:
            return nullptr;
    }
}
void MIPSsim::print_registers() {
    std::cout << "Registers" << std::endl;
    for(int i = 0; i < 32; i += 8) {
        std::cout << "R" << std::setfill('0') << std::setw(2) << i << ":\t";
        for(int j = 0; j < 8; j++) {
            std::cout << registers[i + j] << "\t";
        }
        std::cout << std::endl;
    }
}

void MIPSsim::print_data() {
    std::cout << "Data" << std::endl;
    int data_size = data.size();
    int row = (data_size + 7) / 8;
    for(int i = 0; i < row; i++) {
        std::cout << i*32 + base_data_address << ":\t";
        for(int j = i*8; j < (i+1)*8 && j < data_size; j++) {
            std::cout << data[j] << "\t";
        }
        std::cout << std::endl;
    }
}
void MIPSsim::excute() {
    // 在开始执行前清空文件
    std::ofstream clear_file(simulation_filename, std::ios::trunc);
    clear_file.close();
    uint32_t pc = 0;
    int count = 1;
    // -1 是break指令的需求
    int length = instructions.size() - data.size();
    while(pc < length) {
       Func func = select_instruction(instructions[pc]);
       if (func == nullptr) {
           std::cerr << "未知指令类型: " << static_cast<int>(instructions[pc].type) << std::endl;
           pc+1;
           continue;
       }
       uint32_t next_pc = func(instructions[pc], pc);

       write_to_simulation_file(count, instructions[pc].instruction_detail);
    //    print_registers();
    //    print_data();
       pc = next_pc;
       count++;
    }
    
}

std::unordered_map<unsigned int, std::string> Instruction::category_1_map = {
    {0b000, "J"},
    {0b010, "BEQ"},
    {0b100, "BGTZ"},
    {0b101, "BREAK"},
    {0b110, "SW"},
    {0b111, "LW"}
};

std::unordered_map<unsigned int, std::string> Instruction::category_2_map = {
    {0b000, "ADD"},
    {0b001, "SUB"},
    {0b010, "MUL"},
    {0b011, "AND"},
    {0b100, "OR"},
    {0b101, "XOR"},
    {0b110, "NOR"},
};

std::unordered_map<unsigned int, std::string> Instruction::category_3_map = {
    {0b000, "ADDI"},
    {0b001, "ANDI"},
    {0b010, "ORI"},
    {0b011, "XORI"},
};

bool MIPSsim::is_break = false;


void Instruction::process_instruction() {
    // 获取前3位
    std::string prefix = std::bitset<32>(instruction).to_string() + "      ";
    uint32_t category = (instruction >> 29) & 0x7;
    switch(category) {
        case 0b000:
            process_category_1();
            break;
        case 0b110:
            process_category_2();
            break;
        case 0b111:
            process_category_3();
            break;
        default:
            process_category_4();
            break;
    }
    instruction_detail = std::to_string(address) + " " + instruction_detail;
    print_instruction_detail = prefix + instruction_detail;
    
}

void Instruction::process_category_1() {
    uint32_t opcode = (instruction >> 26) & 0x7;
    if (Instruction::category_1_map.find(opcode) != Instruction::category_1_map.end()) {
        instruction_detail = Instruction::category_1_map[opcode];
    } else {
        instruction_detail = "UNKNOWN";
    }
    type = stringToInstruction(instruction_detail);
    if(type == InstructionType::J) {
        //取延迟槽的指令的前四位
        uint32_t high_bits = ((address+4) & 0xF0000000);
        //取出后面的26位，在左移2位，获得jump的地址
        uint32_t immediate_value = high_bits | ((instruction & 0x3FFFFFF)<<2);
        _immediate_value = immediate_value;
        instruction_detail += " #" + std::to_string(immediate_value);
    }
    else if(type == InstructionType::BEQ) {
        //相等时分支，后16位左移2位，获得offset
        uint32_t rs = (instruction >> 21) & 0x1F;
        uint32_t rt = (instruction >> 16) & 0x1F;
        uint32_t offset = (instruction & 0xFFFF) << 2;
        _rs = rs;
        _rt = rt;
        _offset = offset;
        instruction_detail += " " + int_to_string_reg(rs, rt) + ", #" + std::to_string(offset);
    }
    else if(type == InstructionType::BGTZ) {
        //大于0时分支，后16位左移2位，获得offset，同时没有Rt
        uint32_t rs = (instruction >> 21) & 0x1F;
        uint32_t offset = (instruction & 0xFFFF) << 2;
        _rs = rs;
        _offset = offset;
        instruction_detail += " " + int_to_string_reg(rs) + ", #" + std::to_string(offset);
    }
    else if(type == InstructionType::SW) {
        //存储，后16位获得offset
        uint32_t rs = (instruction >> 21) & 0x1F;
        uint32_t rt = (instruction >> 16) & 0x1F;
        uint32_t offset = (instruction & 0xFFFF);
        _rs = rs;
        _rt = rt;
        _offset = offset;
        instruction_detail += " " + int_to_string_reg(rt) + ", " + std::to_string(offset) + "("+ int_to_string_reg(rs) +")";
    }
    else if(type == InstructionType::LW) {
        //加载，后16位获得offset
        uint32_t rs = (instruction >> 21) & 0x1F;
        uint32_t rt = (instruction >> 16) & 0x1F;
        uint32_t offset = (instruction & 0xFFFF);
        _rs = rs;
        _rt = rt;
        _offset = offset;
        instruction_detail += " " + int_to_string_reg(rt) + ", " + std::to_string(offset) + "("+ int_to_string_reg(rs) +")";
    }else{
        //break
        MIPSsim::is_break = true;
    }

}
void Instruction::process_category_2() {
    uint32_t opcode = (instruction >> 16) & 0x7;
    if (Instruction::category_2_map.find(opcode) != Instruction::category_2_map.end()) {
        instruction_detail = Instruction::category_2_map[opcode];
    } else {
        instruction_detail = "UNKNOWN";
    }
    type = stringToInstruction(instruction_detail);
    uint32_t rd = (instruction >> 11) & 0x1F;
    uint32_t rs = (instruction >> 19) & 0x1F;
    uint32_t rt = (instruction >> 24) & 0x1F;
    _rd = rd;
    _rs = rs;
    _rt = rt;
    instruction_detail += " " + int_to_string_reg(rd, rt, rs);

}
    
void Instruction::process_category_3() {
    int opcode = (instruction >> 16) & 0x7;
    if (Instruction::category_3_map.find(opcode) != Instruction::category_3_map.end()) {
        instruction_detail = Instruction::category_3_map[opcode];
    } else {
        instruction_detail = "UNKNOWN";
    }
    type = stringToInstruction(instruction_detail);
    uint32_t rs = (instruction >> 19) & 0x1F;
    uint32_t rt = (instruction >> 24) & 0x1F;
    uint32_t immediate_value = (instruction) & 0xFF;
    _rs = rs;
    _rt = rt;
    _immediate_value = immediate_value;
    instruction_detail += " " + int_to_string_reg(rs, rt) + ", #" + std::to_string(immediate_value);
}
void Instruction::process_category_4() {
    type = InstructionType::DATA;
    _value = static_cast<int>(instruction);
    instruction_detail = std::to_string(address) + " " + std::to_string(static_cast<int>(instruction));
    print_instruction_detail = std::bitset<32>(instruction).to_string() + "      "+ instruction_detail;
}

uint32_t MIPSsim::_add(Instruction& inst, uint32_t pc) {
    if(inst._rd.has_value() && inst._rs.has_value() && inst._rt.has_value()) {
        uint32_t rd = inst._rd.value();
        uint32_t rs = inst._rs.value();
        uint32_t rt = inst._rt.value();
        registers[rd] = registers[rs] + registers[rt];
        return pc+1;
    }else{
        std::throw_with_nested(std::runtime_error("寄存器错误"));
    }
    return 0;
}

uint32_t MIPSsim::_sub(Instruction& inst, uint32_t pc) {
    if(inst._rd.has_value() && inst._rs.has_value() && inst._rt.has_value()) {
        uint32_t rd = inst._rd.value();
        uint32_t rs = inst._rs.value();
        uint32_t rt = inst._rt.value();
        registers[rd] = registers[rs] - registers[rt];
        return pc+1;
    }else{
        std::throw_with_nested(std::runtime_error("寄存器错误"));
    }
    return 0;
}

uint32_t MIPSsim::_mul(Instruction& inst, uint32_t pc) {
    if(inst._rd.has_value() && inst._rs.has_value() && inst._rt.has_value()) {
        uint32_t rd = inst._rd.value();
        uint32_t rs = inst._rs.value();
        uint32_t rt = inst._rt.value();
        registers[rd] = registers[rs] * registers[rt];
        return pc+1;
    }else{
        std::throw_with_nested(std::runtime_error("寄存器错误"));
    }
    return 0;
}

uint32_t MIPSsim::_and(Instruction& inst, uint32_t pc) {
    if(inst._rd.has_value() && inst._rs.has_value() && inst._rt.has_value()) {
        uint32_t rd = inst._rd.value();
        uint32_t rs = inst._rs.value();
        uint32_t rt = inst._rt.value();
        registers[rd] = registers[rs] & registers[rt];
        return pc+1;
    }else{
        std::throw_with_nested(std::runtime_error("寄存器错误"));
    }
    return 0;
}

uint32_t MIPSsim::_or(Instruction& inst, uint32_t pc) {
    if(inst._rd.has_value() && inst._rs.has_value() && inst._rt.has_value()) {
        uint32_t rd = inst._rd.value();
        uint32_t rs = inst._rs.value();
        uint32_t rt = inst._rt.value();
        registers[rd] = registers[rs] | registers[rt];
        return pc+1;
    }else{
        std::throw_with_nested(std::runtime_error("寄存器错误"));
    }
    return 0;
}

uint32_t MIPSsim::_xor(Instruction& inst, uint32_t pc) {
    if(inst._rd.has_value() && inst._rs.has_value() && inst._rt.has_value()) {
        uint32_t rd = inst._rd.value();
        uint32_t rs = inst._rs.value();
        uint32_t rt = inst._rt.value();
        registers[rd] = registers[rs] ^ registers[rt];
        return pc+1;
    }else{
        std::throw_with_nested(std::runtime_error("寄存器错误"));
    }
    return 0;
}

uint32_t MIPSsim::_nor(Instruction& inst, uint32_t pc) {
    if(inst._rd.has_value() && inst._rs.has_value() && inst._rt.has_value()) {
        uint32_t rd = inst._rd.value();
        uint32_t rs = inst._rs.value();
        uint32_t rt = inst._rt.value();
        registers[rd] = ~(registers[rs] | registers[rt]);
        return pc+1;
    }else{
        std::throw_with_nested(std::runtime_error("寄存器错误"));
    }
    return 0;
}

uint32_t MIPSsim::_addi(Instruction& inst, uint32_t pc) {
    if(inst._rt.has_value() && inst._rs.has_value() && inst._immediate_value.has_value()) {
        uint32_t rt = inst._rt.value();
        uint32_t rs = inst._rs.value();
        uint32_t immediate_value = inst._immediate_value.value();
        registers[rs] = registers[rt] + immediate_value;
        return pc+1;
    }else{
        std::throw_with_nested(std::runtime_error("寄存器错误"));
    }
    return 0;
}

uint32_t MIPSsim::_andi(Instruction& inst, uint32_t pc) {
    if(inst._rt.has_value() && inst._rs.has_value() && inst._immediate_value.has_value()) {
        uint32_t rt = inst._rt.value();
        uint32_t rs = inst._rs.value();
        uint32_t immediate_value = inst._immediate_value.value();
        registers[rs] = registers[rt] & immediate_value;
        return pc+1;
    }else{
        std::throw_with_nested(std::runtime_error("寄存器错误"));
    }
    return 0;
}

uint32_t MIPSsim::_ori(Instruction& inst, uint32_t pc) {
    if(inst._rt.has_value() && inst._rs.has_value() && inst._immediate_value.has_value()) {
        uint32_t rt = inst._rt.value();
        uint32_t rs = inst._rs.value();
        uint32_t immediate_value = inst._immediate_value.value();
        registers[rs] = registers[rt] | immediate_value;
        return pc+1;
    }else{
        std::throw_with_nested(std::runtime_error("寄存器错误"));
    }
    return 0;
}

uint32_t MIPSsim::_xori(Instruction& inst, uint32_t pc) {
    if(inst._rt.has_value() && inst._rs.has_value() && inst._immediate_value.has_value()) {
        uint32_t rt = inst._rt.value();
        uint32_t rs = inst._rs.value();
        uint32_t immediate_value = inst._immediate_value.value();
        registers[rs] = registers[rt] ^ immediate_value;
        return pc+1;
    }else{
        std::throw_with_nested(std::runtime_error("寄存器错误"));
    }
    return 0;
}

uint32_t MIPSsim::_j(Instruction& inst, uint32_t pc) {
    if(inst._immediate_value.has_value()) {
        uint32_t immediate_value = inst._immediate_value.value();
        return (immediate_value - base_address) / 4;
    }else{
        std::throw_with_nested(std::runtime_error("立即数错误"));
    }
    return 0;
}

uint32_t MIPSsim::_beq(Instruction& inst, uint32_t pc) {
    if(inst._rs.has_value() && inst._rt.has_value() && inst._offset.has_value()) {
        uint32_t rs = inst._rs.value();
        uint32_t rt = inst._rt.value();
        uint32_t offset = inst._offset.value();
        if(registers[rs] == registers[rt]) {
            pc += offset / 4;
        }
        return pc+1;
    }else{
        std::throw_with_nested(std::runtime_error("寄存器错误"));
    }
    return 0;
}

uint32_t MIPSsim::_bgtz(Instruction& inst, uint32_t pc) {
    if(inst._rs.has_value() && inst._offset.has_value()) {
        uint32_t rs = inst._rs.value();
        uint32_t offset = inst._offset.value();
        if(registers[rs] > 0) {
            pc += offset / 4;
        }
        return pc+1;
    }else{
        std::throw_with_nested(std::runtime_error("寄存器错误"));
    }
    return 0;
}

uint32_t MIPSsim::_sw(Instruction& inst, uint32_t pc) {
    if(inst._rs.has_value() && inst._rt.has_value() && inst._offset.has_value()) {
        uint32_t rs = inst._rs.value();
        uint32_t rt = inst._rt.value();
        uint32_t offset = inst._offset.value();
        data[(registers[rs] + offset - base_data_address)/4] = registers[rt];
        return pc+1;
    }else{
        std::throw_with_nested(std::runtime_error("寄存器错误"));
    }
    return 0;
}

uint32_t MIPSsim::_lw(Instruction& inst, uint32_t pc) {   
    if(inst._rt.has_value() && inst._rs.has_value() && inst._offset.has_value()) {
        uint32_t rt = inst._rt.value();
        uint32_t rs = inst._rs.value();
        uint32_t offset = inst._offset.value();
        registers[rt] = data[(registers[rs] + offset - base_data_address)/4];
        return pc+1;
    }else{
        std::throw_with_nested(std::runtime_error("寄存器错误"));
    }
    return 0;
}   

uint32_t MIPSsim::_break(Instruction& inst, uint32_t pc) {
    return pc+1;
}

int main(int argc, char* argv[]){
    MIPSsim mips;
    if(argc != 2){
        throw std::invalid_argument("usage: ./MIPSsim <input_file>");
    }
    mips.set_input_filename(argv[1]);
    mips.set_output_filename("generated_disassembly.txt");
    mips.set_simulation_filename("generated_simulation.txt");
    mips.read_from_file();
    mips.write_to_output_file();
    mips.excute();
    return 0;
}
