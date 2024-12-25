#pragma once
#include <string>
#include <unordered_map>

namespace S_TYPE
{
    std::string encodingDecodingSInstruction(std::string operation, std::string o1, std::string o2, std::string o3, int line_number);

    struct SInstruction
    {
        std::string opcode;
        std::string func3;
    };

    inline std::unordered_map<std::string, SInstruction> mappings_S = {
        {"sb", {.opcode = "0100011", .func3 = "000"}},
        {"sh", {.opcode = "0100011", .func3 = "001"}},
        {"sw", {.opcode = "0100011", .func3 = "010"}},
        {"sd", {.opcode = "0100011", .func3 = "011"}}};
}
