#pragma once
#include <string>
#include <unordered_map>

namespace I_TYPE
{
    std::string encodingDecodingIInstruction(std::string operation, std::string o1, std::string o2, std::string o3, int line_number);

    struct IInstruction
    {
        std::string opcode;
        std::string func3;
    };

    inline std::unordered_map<std::string, IInstruction> mappings_I = {
        {"addi", {.opcode = "0010011", .func3 = "000"}},  // Add Immediate
        {"xori", {.opcode = "0010011", .func3 = "100"}},  // XOR Immediate
        {"ori", {.opcode = "0010011", .func3 = "110"}},   // OR Immediate
        {"andi", {.opcode = "0010011", .func3 = "111"}},  // AND Immediate
        {"slli", {.opcode = "0010011", .func3 = "001"}},  // Shift Left Logical Immediate
        {"srli", {.opcode = "0010011", .func3 = "101"}},  // Shift Right Logical Immediate
        {"srai", {.opcode = "0010011", .func3 = "101"}},  // Shift Right Arithmetic Immediate (differs by func7)
        {"slti", {.opcode = "0010011", .func3 = "010"}},  // Set Less Than Immediate
        {"sltiu", {.opcode = "0010011", .func3 = "011"}}, // Set Less Than Immediate Unsigned
        {"lb", {.opcode = "0000011", .func3 = "000"}},    // Load byte
        {"lh", {.opcode = "0000011", .func3 = "001"}},    // Load Halfword
        {"lw", {.opcode = "0000011", .func3 = "010"}},    // Load Word
        {"ld", {.opcode = "0000011", .func3 = "011"}},    // Load Doubleword
        {"lbu", {.opcode = "0000011", .func3 = "100"}},   // Load Byte Unsigned
        {"lhu", {.opcode = "0000011", .func3 = "101"}},   // Load Halfword Unsigned
        {"lwu", {.opcode = "0000011", .func3 = "110"}},   // Load Word Unsigned
        {"jalr", {.opcode = "1100111", .func3 = "000"}}   // Load Word Unsigned

    };
}
