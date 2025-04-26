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
#include <queue>
constexpr uint32_t base_address = 0x00000080;
constexpr int pre_issue_queue_length = 4;
constexpr int pre_alu_queue_length = 2;
constexpr int post_alu_queue_length = 1;
constexpr int pre_mem_queue_length = 1;
constexpr int post_mem_queue_length = 1;
constexpr int max_issue_instruction = 2;
/** 
*    @brief: 将寄存器编号转换为字符串
*    @param value: 寄存器编号
*    @param args: 寄存器编号
*    @return: 寄存器编号对应的名称拼接的字符串
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

/** 
*    @brief:相比与proj1，添加了cache_result,和get_result_register_index,get_source_register_index这两个封装了返回目的寄存器和源寄存器的函数
*    @param instruction: 指令的二进制表示
*    @param type: 指令类型
*    @param instruction_detail: 指令的详细信息(地址加指令)
*    @param print_instruction_detail: 打印到txt的指令的详细信息
*    @param address: 指令的地址
*    @param _rs: 指令的源寄存器(可选)
*    @param _rt: 指令的目标寄存器(可选)
*    @param _rd: 指令的目的寄存器(可选)
*    @param _immediate_value: 指令的立即数(可选)
*    @param _offset: 指令的偏移量(可选)
*    @param _value: data部分的值(可选)
*    @param cache_result: 指令的执行结果的暂时存储，写回时就要取从cache_result中取(可选)
*    @param category_1_map: 指令分类1的静态map
*    @param category_2_map: 指令分类2的静态map
*    @param category_3_map: 指令分类3的静态map
*/
struct Instruction {
    uint32_t instruction{};
    InstructionType type = InstructionType::UNKNOWN;
    std::string instruction_detail;
    std::string print_instruction_detail;
    uint32_t address{};
    std::optional<uint32_t> _rs;
    std::optional<uint32_t> _rt;
    std::optional<uint32_t> _rd;
    std::optional<uint32_t> _immediate_value;
    std::optional<uint32_t> _offset;
    std::optional<int> _value;
    //proj2新添加的，用来标识指令的状态,表示是否发射，是否读取，是否执行，是否写回
    std::optional<int> cache_result;//用来暂时存储指令的执行结果
    static std::unordered_map<unsigned int, std::string> category_1_map;
    static std::unordered_map<unsigned int, std::string> category_2_map;
    static std::unordered_map<unsigned int, std::string> category_3_map;
    void process_instruction();
    void process_category_1();
    void process_category_2();
    void process_category_3();
    void process_category_4();
/** 
*    @brief: 获取目的寄存器
*    使用optional封装，因为可能没有目的寄存器，且不同的指令返回的目的寄存器不同
*    @return: 目的寄存器
*/
    [[nodiscard]] inline std::optional<int> get_result_register_index() const {
        if(_rd.has_value() && 
        (type == InstructionType::ADD || type == InstructionType::SUB 
        || type == InstructionType::MUL || type == InstructionType::AND 
        || type == InstructionType::OR || type == InstructionType::XOR 
        || type == InstructionType::NOR)) {
            return _rd;
        }
        if(_rs.has_value() && 
        (type == InstructionType::ADDI || type == InstructionType::ANDI 
        || type == InstructionType::ORI || type == InstructionType::XORI )) {
            return _rs;
        }
        if(_rt.has_value() && type == InstructionType::LW) {
            return _rt;
        }
        return std::nullopt;
    }
/** 
*    @brief: 获取源寄存器
*    使用optional封装，因为可能没有源寄存器，且不同的指令返回的源寄存器不同
*    @return: 源寄存器
*/
    [[nodiscard]] inline std::tuple<std::optional<int>, std::optional<int>> get_source_register_index() const {
        if(type == InstructionType::ADD || type == InstructionType::SUB 
        || type == InstructionType::MUL || type == InstructionType::AND 
        || type == InstructionType::OR || type == InstructionType::XOR 
        || type == InstructionType::NOR || type == InstructionType::SW || type == InstructionType::BEQ) {
            return {_rs, _rt};
        }
        if(type == InstructionType::ADDI || type == InstructionType::ANDI 
        || type == InstructionType::ORI || type == InstructionType::XORI) {
            return {_rt, std::nullopt};
        }
        if(type == InstructionType::LW || type == InstructionType::BGTZ) {
            return {_rs, std::nullopt};
        }
        return {std::nullopt, std::nullopt};
    }
};


// 这里声明了函数指针的类型，用于存储指令的执行函数。std::function是C++11标准库中的一个模板类，用于存储和调用函数对象。
using Func = std::function<void(Instruction&)>;
using jump_func = std::function<void(Instruction& , int&)>;
/** 
*    @param instructions: 指令列表
*    @param input_filename: 输入文件名
*    @param output_filename: 输出文件名
*    @param simulation_filename: 模拟执行文件名
*    @param registers: 寄存器数组
*    @param data: 数据列表
*    @param base_data_address: 数据段的起始地址
*    @param select_instruction: 选择指令的函数
*    @param is_break: 是否是break指令,区别数据段还是指令段
*    proj2新添加的内容
*    @param register_result_status: 寄存器结果状态
*    @param register_result_status_next_cycle: 下一个周期寄存器结果状态暂存
*    @param waiting_instruction_pc: 等待指令的PC
*    @param executed_instruction_pc: 执行指令的PC
*    @param pre_issue_queue: 预发射队列
*    @param last_cycle_issue_queue: 上一个周期发射队列
*    @param pre_alu_queue: 预ALU队列
*    @param post_alu_queue: 后ALU队列
*    @param pre_mem_queue: 预访存队列
*    @param post_mem_queue: 后访存队列
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
    jump_func select_jump_instruction(Instruction& inst);
    void _add(Instruction& inst);
    void _sub(Instruction& inst);
    void _mul(Instruction& inst);
    void _and(Instruction& inst);
    void _or(Instruction& inst);
    void _xor(Instruction& inst);
    void _nor(Instruction& inst);
    void _addi(Instruction& inst);
    void _andi(Instruction& inst);
    void _ori(Instruction& inst);
    void _xori(Instruction& inst);
    void _sw(Instruction& inst);
    void _lw(Instruction& inst);
    void _j(Instruction& inst, int& pc);
    void _beq(Instruction& inst, int& pc);
    void _bgtz(Instruction& inst, int& pc);
    void _break(Instruction& inst, int& pc);
    //proj2新添加的内容
    /** 
    *    @brief: 固定大小的队列
    *    @param T: 队列的类型
    *    @param max_size: 队列的最大大小
    *    @param last_cycle_full: 上一个周期是否满了
    *    @param is_full: 是否满了
    *    @param get_max_size: 获取队列的最大大小
    *    @param begin: 获取队列的开始迭代器
    *    @param end: 获取队列的结束迭代器
    *    @param sort_instruction: 排序指令
    */
    template<typename T>
    class FixedSizeQueue : public std::queue<T> {
        size_t max_size;
        bool last_cycle_full;
        public:
            explicit FixedSizeQueue(size_t size) : max_size(size) {
                last_cycle_full = is_full();
            }
            
            bool is_full() const { return this->size() >= max_size; }
            size_t get_max_size() const { return max_size; }
            
            auto begin() { return this->c.begin(); }
            auto end() { return this->c.end(); }
            void sort_instruction() {//按照地址从低到高排序
                std::sort(this->begin(), this->end(), 
                          [](const Instruction& a, const Instruction& b){ 
                              return a.address < b.address; 
                          });
            }
            void set_last_cycle_full() {
                last_cycle_full = is_full();
            }
            bool get_last_cycle_full() const {
                return last_cycle_full;
            }
            void push(const T& value) {
                if (this->size() >= max_size) {
                    throw std::length_error("Queue is full, cannot push more elements");
                }
                std::queue<T>::push(value);
            }
            friend std::ostream& operator<<(std::ostream& os, const FixedSizeQueue<T>& queue) {
                if(queue.get_max_size() == 1) {
                    if(!queue.empty()) {
                        os<<"[" << queue.front().instruction_detail << "]\n";
                    }else{
                        os << std::endl;
                    }
                }
                else{
                    auto temp_queue = queue;
                    os << "\n\t";
                    for (size_t i = 0; i < queue.get_max_size(); ++i) {
                        os << "Entry " << i << ":";
                        if (!temp_queue.empty()) {
                            os << "[" << temp_queue.front().instruction_detail << "]";
                            temp_queue.pop();
                        }
                        os << "\n";
                        if(i < queue.get_max_size() - 1) {
                            os << "\t";
                        }
                    }
                }
                return os;
            }
    };

    FixedSizeQueue<Instruction> pre_issue_queue{pre_issue_queue_length};
    //上一个周期发射队列,由于fetch到branch instruction时，需要检查是否存在RAW hazard，所以需要保存上一个周期的发射队列。已经发射的可以通过register_result_status来判断
    FixedSizeQueue<Instruction> last_cycle_issue_queue{pre_issue_queue_length};
    FixedSizeQueue<Instruction> pre_alu_queue{pre_alu_queue_length};
    FixedSizeQueue<Instruction> post_alu_queue{post_alu_queue_length};
    FixedSizeQueue<Instruction> pre_mem_queue{pre_mem_queue_length};
    FixedSizeQueue<Instruction> post_mem_queue{post_mem_queue_length};
    std::array<int, 32> register_result_status;
    std::array<int, 32> register_result_status_next_cycle;
    bool check_issue_hazard(const Instruction& inst) const;
    int waiting_instruction_pc = -1;
    int executed_instruction_pc = -1;
    void write_back();
    void alu_unit();
    void mem_unit();
    void issue();
    void instruction_fetch(uint32_t& pc);
    bool is_branch_instruction(const Instruction& inst)const;
    //更新snapshot，snapshot是更新寄存器状态，队列是否为满以及保存上周期的pre-issue。通过更新来避免structure hazards
    void update_snapshot();
    public:
        static bool is_break;
        MIPSsim()=default;
        ~MIPSsim()=default;
        void excute(); 
        void read_from_file();
        void write_to_output_file();
        void write_to_simulation_file(int cycle);
        void set_input_filename(const std::string& filename);
        void set_output_filename(const std::string& filename);
        void set_simulation_filename(const std::string& filename);
};

static std::unordered_map<std::string, InstructionType> stringToInstructionMap = {
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
        throw std::logic_error("无法打开文件: " + input_filename);
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

void MIPSsim::write_to_simulation_file(int cycle) {
    // 使用追加模式打开文件
    std::ofstream simulation_file(simulation_filename, std::ios::app);
    
    simulation_file << "--------------------"<< std::endl;
    simulation_file << "Cycle:" << cycle << std::endl<<std::endl;
    simulation_file << "IF Unit:" << std::endl;
    simulation_file << "\tWaiting Instruction:";
    if(waiting_instruction_pc != -1) {
        simulation_file << "[" << instructions[waiting_instruction_pc].instruction_detail << "]"<< std::endl;
    }else{
        simulation_file << std::endl;
    }
    simulation_file << "\tExecuted Instruction:";
    if(executed_instruction_pc != -1) {
        simulation_file << "[" << instructions[executed_instruction_pc].instruction_detail << "]";
    }
    simulation_file << std::endl;
    simulation_file << "Pre-Issue Queue:" << pre_issue_queue ;
    simulation_file << "Pre-ALU Queue:" << pre_alu_queue ;
    simulation_file << "Pre-MEM Queue:" << pre_mem_queue ;
    simulation_file << "Post-MEM Queue:" << post_mem_queue ;
    simulation_file << "Post-ALU Queue:" << post_alu_queue <<std::endl;
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
            return [this](Instruction& i) { return this->_add(i); };
        case InstructionType::SUB:
            return [this](Instruction& i) { return this->_sub(i); };
        case InstructionType::MUL:
            return [this](Instruction& i) { return this->_mul(i); };
        case InstructionType::AND:
            return [this](Instruction& i) { return this->_and(i); };
        case InstructionType::OR:
            return [this](Instruction& i) { return this->_or(i); };
        case InstructionType::XOR:
            return [this](Instruction& i) { return this->_xor(i); };
        case InstructionType::NOR:
            return [this](Instruction& i) { return this->_nor(i); };
        case InstructionType::ADDI:
            return [this](Instruction& i) { return this->_addi(i); };
        case InstructionType::ANDI:
            return [this](Instruction& i) { return this->_andi(i); };
        case InstructionType::ORI:
            return [this](Instruction& i) { return this->_ori(i); };
        case InstructionType::XORI:
            return [this](Instruction& i) { return this->_xori(i); };
        case InstructionType::SW:
            return [this](Instruction& i) { return this->_sw(i); };
        case InstructionType::LW:
            return [this](Instruction& i) { return this->_lw(i); };
        default:
            return nullptr;
    }
}

jump_func MIPSsim::select_jump_instruction(Instruction& inst) {
    switch(inst.type) {
        case InstructionType::J:
            return [this](Instruction& i, int& pc) { return this->_j(i, pc); };
        case InstructionType::BEQ:
            return [this](Instruction& i, int& pc) { return this->_beq(i, pc); };
        case InstructionType::BGTZ:
            return [this](Instruction& i, int& pc) { return this->_bgtz(i, pc); };
        case InstructionType::BREAK:
            return [this](Instruction& i, int& pc) { return this->_break(i, pc); };
        default:
            return nullptr;
    }
}

//写回的时候，在post_mem_queue和post_alu_queue中，最多各执行一次
void MIPSsim::write_back() {
    if(!post_mem_queue.empty()) {
        auto inst = post_mem_queue.front();
        post_mem_queue.pop();
        registers[inst.get_result_register_index().value()] = inst.cache_result.value();
        register_result_status_next_cycle[inst.get_result_register_index().value()] = -1;
    }
    if(!post_alu_queue.empty()) {
        auto inst = post_alu_queue.front();
        post_alu_queue.pop();
        registers[inst.get_result_register_index().value()] = inst.cache_result.value();
        register_result_status_next_cycle[inst.get_result_register_index().value()] = -1;
    }
}
/*指令获取/译码单元行为说明
    指令获取/译码单元在每个周期最多可以顺序地获取并译码两条指令。在获取新指令之前，必须检查以下所有条件是否满足：
     - 如果在上一个周期末获取单元被暂停（stalled），那么当前周期不能获取任何指令。获取单元可能因为分支指令（branch instruction）而暂停。
     - 如果在上一个周期末，Pre-issue 队列中没有空位，那么当前周期也不能获取任何指令。
    通常，整个“获取-译码”操作可以在一个周期内完成。被译码的指令会在当前周期末放入 Pre-issue 队列中。
 */
void MIPSsim::instruction_fetch(uint32_t& pc) {
    if(pre_issue_queue.get_last_cycle_full()){
        return;
    }
    if(executed_instruction_pc != -1) {
        executed_instruction_pc = -1;
    }
    //第一次取指令
    if(is_branch_instruction(instructions[pc])) {
        int next_pc = pc;
        auto [rs, rt] = instructions[pc].get_source_register_index();
        // RAW hazard: If a branch instruction is fetched, the fetch unit will try to read all the necessary registers to calculate the target address.
        if((!rs.has_value() || register_result_status[rs.value()] == -1) 
        &&(!rt.has_value() || register_result_status[rt.value()] == -1)// issued instruction
        && [&](){//pre-issued instruction
            auto temp_queue = last_cycle_issue_queue;
            while(!temp_queue.empty()) {
                auto& instruction = temp_queue.front();
                auto rd = instruction.get_result_register_index();
                if(rd.has_value() && (rs.has_value() && rd.value() == rs.value() || rt.has_value() && rd.value() == rt.value())) {
                    return false;
                }
                temp_queue.pop();
            }
            return true;
        }()){
            select_jump_instruction(instructions[pc])(instructions[pc], next_pc);
            //If all the registers are ready (or  target  is  immediate),  it  will  update  PC  before  the  end  of  the  current  cycle.
            executed_instruction_pc = pc;
            waiting_instruction_pc = -1;
            pc = next_pc;
            return;
        }else{
            // Otherwise  the  unit  is stalled  until  the  required  registers  are  available.
            waiting_instruction_pc = pc;
            return;
        }
    }else{
        pre_issue_queue.push(instructions[pc]);
        pc++;
    }
    //第二次取指令
    if(is_branch_instruction(instructions[pc])) {
        int next_pc = pc;
        auto [rs, rt] = instructions[pc].get_source_register_index();
        // If a branch instruction is fetched, the fetch unit will try to read all the necessary registers to calculate the target address.
        if((!rs.has_value() || register_result_status[rs.value()] == -1) 
        &&(!rt.has_value() || register_result_status[rt.value()] == -1)
        &&[&](){//pre-issued instruction
            auto temp_queue = last_cycle_issue_queue;
            while(!temp_queue.empty()) {
                auto& instruction = temp_queue.front();
                auto rd = instruction.get_result_register_index();
                if(rd.has_value() && (rs.has_value() && rd.value() == rs.value() || rt.has_value() && rd.value() == rt.value())) {
                    return false;
                }
                temp_queue.pop();
            }
            //和第一次取指令不同，如果第二个指令fetch到了branch指令，那么需要检查是否存在RAW hazard，和第一条fetch的指令
            auto fetch_last_rd = instructions[pc-1].get_result_register_index();
            if(fetch_last_rd.has_value() && (rs.has_value() && fetch_last_rd.value() == rs.value() || rt.has_value() && fetch_last_rd.value() == rt.value())) {
                return false;
            }
            return true;
        }()) {
            select_jump_instruction(instructions[pc])(instructions[pc], next_pc);
            //If all the registers are ready (or  target  is  immediate),  it  will  update  PC  before  the  end  of  the  current  cycle.
            executed_instruction_pc = pc;
            waiting_instruction_pc = -1;
            pc = next_pc;
            return;
        }else{
            // Otherwise  the  unit  is stalled  until  the  required  registers  are  available.
            waiting_instruction_pc = pc;
            return;
        }
    }else{
        pre_issue_queue.push(instructions[pc]);
        pc++;
    }
}

void MIPSsim::update_snapshot() {
    register_result_status = register_result_status_next_cycle;
    last_cycle_issue_queue = pre_issue_queue;
    pre_issue_queue.set_last_cycle_full();
    pre_alu_queue.set_last_cycle_full();
    post_alu_queue.set_last_cycle_full();
    pre_mem_queue.set_last_cycle_full();
    post_mem_queue.set_last_cycle_full();
}
bool MIPSsim::check_issue_hazard(const Instruction& inst) const {
    //first check if there is a WAW hazard
    //No WAW hazards with active instructions (issued but not finished, or earlier not-issued instructions). 
    //issued but not finished
    //sw has no destination register
    if(inst.type == InstructionType::SW || register_result_status[inst.get_result_register_index().value()] == -1 ) {//写前面不用在判断optional是否空
        //earlier not-issued instruction
        auto temp_queue = pre_issue_queue;
        while(!temp_queue.empty()) {
        auto& instruction = temp_queue.front();
        if(instruction.get_result_register_index() == inst.get_result_register_index() 
           && instruction.address < inst.address) {
            return false;
            }
            temp_queue.pop();
        }
    }else{
        return false;
    }

    //check if there is a RAW hazard
    auto [rs, rt] = inst.get_source_register_index();
    auto tmp_queue = pre_issue_queue;
    while(!tmp_queue.empty()) {
        auto& instruction = tmp_queue.front();
        auto rd = instruction.get_result_register_index();
        if(instruction.address < inst.address && rd.has_value()&& rs.has_value() && rs.value() == rd.value()) {
            return false;
        }
        if(instruction.address < inst.address && rd.has_value()&& rt.has_value() && rt.value() == rd.value()) {
            return false;
        }
        tmp_queue.pop();
    }
    if(rs.has_value() && rt.has_value()) {
        if(register_result_status[rs.value()] != -1 ||
        register_result_status[rt.value()] != -1) {
            return false;
        }
    }
    if(rs.has_value() && !rt.has_value()) {
        if(register_result_status[rs.value()] != -1) {
            return false;
        }
    }
    
    //check if there is a WAR hazard
    // No WAR hazards with earlier not-issued instructions
    auto temp_queue = pre_issue_queue;
    auto result_register_index = inst.get_result_register_index();
    while(!temp_queue.empty()) {
        auto& instruction = temp_queue.front();
        auto [rs, rt] = instruction.get_source_register_index();
        if(instruction.address < inst.address && rs.has_value() && rs.value() == result_register_index.value()) {
            return false;
        }
        if(instruction.address < inst.address && rt.has_value() && rt.value() == result_register_index.value()) {
            return false;
        }
        temp_queue.pop();
    }
    return true;
}

void MIPSsim::issue() {
    /*
    No structural hazards (the corresponding queue, i.e., Pre-ALU has empty slots at the end of last cycle); 
    No WAW hazards with active instructions (issued but not finished, or earlier not-issued instructions). 
    If two instructions are issued in a cycle, you need to make sure that there are no WAW or WAR hazards between them. 
    No WAR hazards with earlier not-issued instructions; 
    For MEM instructions, all the source registers are ready at the end of last cycle. 
    The load instruction must wait until all the previous stores are issued. 
    The stores must be issued in order. 
    */
    int issue_count = 0;
    int count = 0;
    bool has_sw_instruction = false;
    bool sw_issued_success = true;
    Instruction inst_issued_first;
    if(pre_issue_queue.empty()) {//no instruction in pre_issue_queue, no issue
        return;
    }
    //Pre-Issue Queue has 4 entries; each one can store one instruction. The instructions are  sorted  by  their  program  order,
    

    while(issue_count < max_issue_instruction && count < pre_issue_queue_length) {
        //Pre-ALU has empty slots at the end of last cycle
        if(pre_alu_queue.get_last_cycle_full()) {//No structural hazards (the corresponding queue, i.e., Pre-ALU has empty slots at the end of last cycle); 
            break;
        }
        if(pre_issue_queue.empty()) {//No structural hazards (the corresponding queue, i.e., Pre-ALU has empty slots at the end of last cycle); 
            break;
        }
        auto inst = pre_issue_queue.front();
        pre_issue_queue.pop();
        if(inst.type == InstructionType::SW) {
            //The stores must be issued in order.
            if(!sw_issued_success) {
                pre_issue_queue.push(inst);
                count++;
                continue;
            }
            //For MEM instructions, all the source registers are ready at the end of last cycle.
            auto [rs, rt] = inst.get_source_register_index();
            if(rs.has_value() && rt.has_value() 
            && register_result_status[rs.value()] != -1 
            && register_result_status[rt.value()] != -1) {
                sw_issued_success = false;// sw not issued successfully, another sw behind this sw must wait
                pre_issue_queue.push(inst);
                count++;
                continue;
            }
            if(!has_sw_instruction) {//The load instruction must wait until all the previous stores are issued. 
                has_sw_instruction = true;
            }
        }
        if(inst.type == InstructionType::LW) {
            if(has_sw_instruction) { //The load instruction must wait until all the previous stores are issued. 
                pre_issue_queue.push(inst);
                count++;
                continue;
            }
            auto [rs, rt] = inst.get_source_register_index();
            if(rs.has_value() && register_result_status[rs.value()] != -1) {
                //For MEM instructions, all the source registers are ready at the end of last cycle.
                pre_issue_queue.push(inst);
                count++;
                continue;
            }
        }
        if(check_issue_hazard(inst))
        { 
            auto rd = inst.get_result_register_index();
            if(issue_count == 0) {
                inst_issued_first = inst;
            }else{
                //If two instructions are issued in a cycle, you need to make sure that there are no WAW or WAR hazards between them.
                auto [rs, rt] = inst_issued_first.get_source_register_index();
                auto rd_issued = inst_issued_first.get_result_register_index();
                if(rd.has_value() && register_result_status_next_cycle[rd.value()] != -1) {//WAW hazard
                    pre_issue_queue.push(inst);
                    count++;
                    continue;
                }
                if(rs.has_value() && rd_issued.has_value() && rs.value() == rd_issued.value()) {//RAW hazard
                    pre_issue_queue.push(inst);
                    count++;
                    continue;
                }
                if(rt.has_value() && rd_issued.has_value() && rt.value() == rd_issued.value()) {//RAW hazard
                    pre_issue_queue.push(inst);
                    count++;
                    continue;
                }
                if(rs.has_value() && rd.has_value() && rs.value() == rd.value()) {//WAR hazard
                    pre_issue_queue.push(inst);
                    count++;
                    continue;
                }
                if(rt.has_value() && rd.has_value() && rt.value() == rd.value()) {//WAR hazard
                    pre_issue_queue.push(inst);
                    count++;
                    continue;
                }
            }
            pre_alu_queue.push(inst);
            issue_count++;
            //issue_bookkeeping
            if(rd.has_value()) {
                register_result_status_next_cycle[rd.value()] = inst.address;
            }
        }else{// sw 由于某种原因没有发射成功
            if(inst.type == InstructionType::SW) {
                sw_issued_success = false;
            }
            pre_issue_queue.push(inst);
        }
        count++;
    }
    pre_issue_queue.sort_instruction();
}
void MIPSsim::alu_unit() {//The pre-alu queue is managed as FIFO (in-order) queue.
    if(pre_alu_queue.empty()) {
        return;
    }
    //进入pre_alu_queue的指令，一定可以直接取然后执行，所以只要取front
    auto inst = pre_alu_queue.front();
    pre_alu_queue.pop();
    Func func = select_instruction(inst);
    if(inst.type == InstructionType::SW || inst.type == InstructionType::LW) {
        func(inst);
        pre_mem_queue.push(inst);
    }
    else{
        func(inst);
        post_alu_queue.push(inst);
    }
}
void MIPSsim::mem_unit() {
    if(pre_mem_queue.empty()) {
        return;
    }
    auto inst = pre_mem_queue.front();
    pre_mem_queue.pop();
    //For SW instruction, MEM also takes one cycle to finish (write the data to memory). When an SW instruction finishes, nothing would be sent to Post-MEM queue.
    if(inst.type == InstructionType::SW && inst.cache_result.has_value() && inst._rt.has_value()) {
        data[inst.cache_result.value()] = registers[inst._rt.value()];
    }else{
        //When a LW instruction finishes, the instruction with destination register id and the data will be written to the Post-MEM queue
        if(inst.cache_result.has_value() ) {
            inst.cache_result = data[inst.cache_result.value()];
        }
        post_mem_queue.push(inst);
    }
}
bool MIPSsim::is_branch_instruction(const Instruction& inst)const{
    if(inst.type == InstructionType::BEQ || inst.type == InstructionType::BGTZ || inst.type == InstructionType::J || inst.type == InstructionType::BREAK) {
        return true;
    }
    if(inst.type == InstructionType::DATA || inst.type == InstructionType::UNKNOWN) {
        throw std::logic_error("DATA和UNKNOWN指令不能作为分支指令");
    }
    return false;
}

void MIPSsim::excute() {
    // 在开始执行前清空文件
    std::ofstream clear_file(simulation_filename, std::ios::trunc);
    
    clear_file.close();
    // 初始化寄存器状态表为-1，表示寄存器未被占用
    register_result_status.fill(-1);
    register_result_status_next_cycle.fill(-1);
    //这里开始重写proj2的执行逻辑
    uint32_t pc = 0;
    int count = 1;
    int length = instructions.size() - data.size();
    while(pc < length) {
        update_snapshot();
        write_back();
        mem_unit();
        alu_unit();
        issue();
        instruction_fetch(pc);
        write_to_simulation_file(count);
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
    print_instruction_detail = prefix + std::to_string(address) + " " + instruction_detail;
    
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

void MIPSsim::_add(Instruction& inst) {
    if(inst._rd.has_value() && inst._rs.has_value() && inst._rt.has_value()) {
        uint32_t rd = inst._rd.value();
        uint32_t rs = inst._rs.value();
        uint32_t rt = inst._rt.value();
        inst.cache_result = registers[rs] + registers[rt];
        // registers[rd] = inst.cache_result.value();
    }else{
        throw std::logic_error("寄存器错误");
    }

}

void MIPSsim::_sub(Instruction& inst) {
    if(inst._rd.has_value() && inst._rs.has_value() && inst._rt.has_value()) {
        uint32_t rd = inst._rd.value();
        uint32_t rs = inst._rs.value();
        uint32_t rt = inst._rt.value();
        inst.cache_result = registers[rs] - registers[rt];
        // registers[rd] = inst.cache_result.value();
    }else{
        throw std::logic_error("寄存器错误");
    }

}

void MIPSsim::_mul(Instruction& inst) {
    if(inst._rd.has_value() && inst._rs.has_value() && inst._rt.has_value()) {
        uint32_t rd = inst._rd.value();
        uint32_t rs = inst._rs.value();
        uint32_t rt = inst._rt.value();
        inst.cache_result = registers[rs] * registers[rt];
        // registers[rd] = inst.cache_result.value();
    }else{
        throw std::logic_error("寄存器错误");
    }

}

void MIPSsim::_and(Instruction& inst) {
    if(inst._rd.has_value() && inst._rs.has_value() && inst._rt.has_value()) {
        uint32_t rd = inst._rd.value();
        uint32_t rs = inst._rs.value();
        uint32_t rt = inst._rt.value();
        inst.cache_result = registers[rs] & registers[rt];
        // registers[rd] = inst.cache_result.value();
    }else{
        throw std::logic_error("寄存器错误");
    }

}

void MIPSsim::_or(Instruction& inst) {
    if(inst._rd.has_value() && inst._rs.has_value() && inst._rt.has_value()) {
        uint32_t rd = inst._rd.value();
        uint32_t rs = inst._rs.value();
        uint32_t rt = inst._rt.value();
        inst.cache_result = registers[rs] | registers[rt];
        // registers[rd] = inst.cache_result.value();
    }else{
        throw std::logic_error("寄存器错误");
    }

}

void MIPSsim::_xor(Instruction& inst) {
    if(inst._rd.has_value() && inst._rs.has_value() && inst._rt.has_value()) {
        uint32_t rd = inst._rd.value();
        uint32_t rs = inst._rs.value();
        uint32_t rt = inst._rt.value();
        inst.cache_result = registers[rs] ^ registers[rt];
        // registers[rd] = inst.cache_result.value();
    }else{
        throw std::logic_error("寄存器错误");
    }

}

void MIPSsim::_nor(Instruction& inst) {
    if(inst._rd.has_value() && inst._rs.has_value() && inst._rt.has_value()) {
        uint32_t rd = inst._rd.value();
        uint32_t rs = inst._rs.value();
        uint32_t rt = inst._rt.value();
        inst.cache_result = ~(registers[rs] | registers[rt]);
        // registers[rd] = inst.cache_result.value();
    }else{
        throw std::logic_error("寄存器错误");
    }

}

void MIPSsim::_addi(Instruction& inst) {
    if(inst._rt.has_value() && inst._rs.has_value() && inst._immediate_value.has_value()) {
        uint32_t rt = inst._rt.value();
        uint32_t rs = inst._rs.value();
        uint32_t immediate_value = inst._immediate_value.value();
        inst.cache_result = registers[rs] + static_cast<int>(immediate_value);
        // registers[rt] = inst.cache_result.value();
    }else{
        throw std::logic_error("寄存器错误");
    }

}

void MIPSsim::_andi(Instruction& inst) {
    if(inst._rt.has_value() && inst._rs.has_value() && inst._immediate_value.has_value()) {
        uint32_t rt = inst._rt.value();
        uint32_t rs = inst._rs.value();
        uint32_t immediate_value = inst._immediate_value.value();
        inst.cache_result = registers[rs] & immediate_value;
        // registers[rt] = inst.cache_result.value();
    }else{
        throw std::logic_error("寄存器错误");
    }

}

void MIPSsim::_ori(Instruction& inst) {
    if(inst._rt.has_value() && inst._rs.has_value() && inst._immediate_value.has_value()) {
        uint32_t rt = inst._rt.value();
        uint32_t rs = inst._rs.value();
        uint32_t immediate_value = inst._immediate_value.value();
        inst.cache_result = registers[rs] | immediate_value;
        // registers[rt] = inst.cache_result.value();
    }else{
        throw std::logic_error("寄存器错误");
    }

}

void MIPSsim::_xori(Instruction& inst) {
    if(inst._rt.has_value() && inst._rs.has_value() && inst._immediate_value.has_value()) {
        uint32_t rt = inst._rt.value();
        uint32_t rs = inst._rs.value();
        uint32_t immediate_value = inst._immediate_value.value();
        inst.cache_result = registers[rs] ^ immediate_value;
        // registers[rt] = inst.cache_result.value();
    }else{
        throw std::logic_error("寄存器错误");
    }

}

void MIPSsim::_sw(Instruction& inst) {
    if(inst._rs.has_value() && inst._rt.has_value() && inst._offset.has_value()) {
        uint32_t rs = inst._rs.value();
        uint32_t rt = inst._rt.value();
        uint32_t offset = inst._offset.value();
        inst.cache_result = (registers[rs] + offset - base_data_address)/4;
        // data[(registers[rs] + offset - base_data_address)/4] = registers[rt];
    }else{
        throw std::logic_error("寄存器错误");
    }

}

void MIPSsim::_lw(Instruction& inst) {   
    if(inst._rt.has_value() && inst._rs.has_value() && inst._offset.has_value()) {
        uint32_t rt = inst._rt.value();
        uint32_t rs = inst._rs.value();
        uint32_t offset = inst._offset.value();
        inst.cache_result = (registers[rs] + offset - base_data_address)/4;
        // registers[rt] = data[(registers[rs] + offset - base_data_address)/4];
    }else{
        throw std::logic_error("寄存器错误");
    }

}   

void MIPSsim::_j(Instruction& inst, int& pc) {
    if(inst._immediate_value.has_value()) {
        uint32_t immediate_value = inst._immediate_value.value();
        pc = (immediate_value - base_address) / 4;
    }else{
        throw std::logic_error("寄存器错误");
    }
}

void MIPSsim::_beq(Instruction& inst, int& pc) {
    if(inst._rs.has_value() && inst._rt.has_value() && inst._offset.has_value()) {
        uint32_t rs = inst._rs.value();
        uint32_t rt = inst._rt.value();
        uint32_t offset = inst._offset.value();
        if(registers[rs] == registers[rt]) {
            pc += offset / 4;
        }
        pc++;
    }else{
        throw std::logic_error("寄存器错误");
    }
}

void MIPSsim::_bgtz(Instruction& inst, int& pc) {
    if(inst._rs.has_value() && inst._offset.has_value()) {
        uint32_t rs = inst._rs.value();
        uint32_t offset = inst._offset.value();
        if(registers[rs] > 0) {
            pc += offset / 4;
        }
        pc++;
    }else{
        throw std::logic_error("寄存器错误");
    }
}

void MIPSsim::_break(Instruction& inst, int& pc) {
    pc = instructions.size() - data.size();
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
    // mips.write_to_output_file();
    mips.excute();

}
