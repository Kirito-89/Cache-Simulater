#pragma once
#include <string>
#include <unordered_map>

namespace R_TYPE
{
    std::string encodingDecodingRInstruction(std::string operation, std::string o1, std::string o2, std::string o3, int line_number);

    struct RInstruction
    {
        std::string opcode;
        std::string func3;
        std::string func7;
    };

    inline std::unordered_map<std::string, RInstruction> mappings_R = {
        {"add", {"0110011", "000", "0000000"}},
        {"sub", {"0110011", "000", "0100000"}},
        {"xor", {"0110011", "100", "0000000"}},
        {"or", {"0110011", "110", "0000000"}},
        {"and", {"0110011", "111", "0000000"}},
        {"sll", {"0110011", "001", "0000000"}},
        {"srl", {"0110011", "101", "0000000"}},
        {"sra", {"0110011", "101", "0100000"}},
        {"slt", {"0110011", "010", "0000000"}},
        {"sltu", {"0110011", "011", "0000000"}}};
}
