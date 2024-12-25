#pragma once
#include <string>
#include <unordered_map>

namespace B_TYPE
{

    std::string offsetValue(long long int number);

    std::string encodingDecodingBInstruction(std::string operation, std::string o1, std::string o2, std::string o3, int line_number, std::unordered_map<std::string, int> &labels);

    struct BInstruction
    {
        std::string opcode;
        std::string func3;
    };

    inline std::unordered_map<std::string, BInstruction> mappings_B = {
        {"beq", {.opcode = "1100011", .func3 = "000"}},
        {"bne", {.opcode = "1100011", .func3 = "001"}},
        {"blt", {.opcode = "1100011", .func3 = "100"}},
        {"bge", {.opcode = "1100011", .func3 = "101"}},
        {"bltu", {.opcode = "1100011", .func3 = "110"}},
        {"bgeu", {.opcode = "1100011", .func3 = "111"}}};

}