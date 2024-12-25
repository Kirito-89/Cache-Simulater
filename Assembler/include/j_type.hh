#pragma once
#include <string>
#include <unordered_map>

namespace J_TYPE
{

    std::string offsetValueJInstruction(long long int number);

    std::string encodingDecodingJInstruction(std::string operation, std::string o1, std::string o2, std::string o3, int line_number, std::unordered_map<std::string, int> &labels);

    struct JInstruction
    {
        std::string opcode;
    };

    inline std::unordered_map<std::string, JInstruction> mappings_J = {
        {"jal", {.opcode = "1101111"}},
    };
}
