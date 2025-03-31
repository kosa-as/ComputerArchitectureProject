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

constexpr int base_address = 0x00000080;



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

// 将定义改为声明
extern std::unordered_map<std::string, InstructionType> stringToInstructionMap;

// 将 MIPSsim 前向声明添加到这里
class MIPSsim;

struct Instruction {
    uint32_t instruction;
    InstructionType type;
    std::string instruction_detail;
    std::string print_instruction_detail;
    int address;
    std::optional<int> _rs;
    std::optional<int> _rt;
    std::optional<int> _rd;
    std::optional<int> _immediate_value;
    std::optional<int> _offset;
    std::optional<int> _value;
    static std::unordered_map<int, std::string> category_1_map;
    static std::unordered_map<int, std::string> category_2_map;
    static std::unordered_map<int, std::string> category_3_map;
    void process_instruction();
    void process_category_1();
    void process_category_2();
    void process_category_3();
    void process_category_4();
};



using Func = std::function<int(Instruction&, int)>;
class MIPSsim {
    std::vector<Instruction> instructions;
    std::string input_filename;
    std::string output_filename;
    std::string simulation_filename;
    std::array<int, 32> registers{};
    std::vector<int> data;
    int base_data_address;
    Func select_instruction(Instruction& inst);
    int _add(Instruction& inst, int pc);
    int _sub(Instruction& inst, int pc);
    int _mul(Instruction& inst, int pc);
    int _and(Instruction& inst, int pc);
    int _or(Instruction& inst, int pc);
    int _xor(Instruction& inst, int pc);
    int _nor(Instruction& inst, int pc);
    int _addi(Instruction& inst, int pc);
    int _andi(Instruction& inst, int pc);
    int _ori(Instruction& inst, int pc);
    int _xori(Instruction& inst, int pc);
    int _j(Instruction& inst, int pc);
    int _beq(Instruction& inst, int pc);
    int _bgtz(Instruction& inst, int pc);
    int _sw(Instruction& inst, int pc);
    int _lw(Instruction& inst, int pc);
    int _break(Instruction& inst, int pc);
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

// 函数声明
InstructionType stringToInstruction(const std::string& str);



