#pragma once
#include <string>
#include <unordered_map>

namespace U_TYPE
{
    std::string encodingDecodingUInstruction(std::string operation, std::string o1, std::string o2, std::string o3, int line_number);

    struct UInstruction
    {
        std::string opcode;
    };

    std::string offsetValueU(long long int number);

    inline std::unordered_map<std::string, UInstruction> mappings_U = {
        {"lui", {.opcode = "0110111"}},
        {"auipc", {.opcode = "0010111"}},
    };
}
