#include "s_type.hh"
#include "base.hh"

std::string S_TYPE ::encodingDecodingSInstruction(std::string operation, std::string o1, std::string o2, std::string o3, int line_number)
{

    std::string rs2 = o1;
    size_t pos = o2.find('(');
    std::string imm = o2.substr(0, pos);

    std::string rs1 = o2.substr(pos + 1, o2.find(')') - pos - 1);

    bool isLineErrorFree = true;
    std::string output = "";
    //error checking 
    if (BASE::register_map.find(rs1) == BASE::register_map.end())
    {
        std::stringstream se;
        se << "Error: Invalid register '" << rs1 << "' in line " << line_number;
        output = output + se.str();
        isLineErrorFree = false;
    }
    if (BASE::register_map.find(rs2) == BASE::register_map.end())
    {
        std::stringstream se;
        se << "Error: Invalid register '" << rs2 << "' in line " << line_number;
        output = output + se.str();
        isLineErrorFree = false;
    }
      if(imm== "" || !BASE::isNumericString(imm)){
            isLineErrorFree = false;
            output = "Error : Wrong Instruction (Incomplete)\n";

        }
        else{

        
    if (stoll(imm) < -2048 || stoll(imm) > 2047)
    {
        std::stringstream se;
        se << "Error: Imm Value " << imm << " too large (Must be in range [-2048,2047]) in line " << line_number;
        output = output + se.str();
        isLineErrorFree = false;
    }
        }
    //Convert if no error
    if (isLineErrorFree == true)
    {
        SInstruction format = S_TYPE::mappings_S[operation];
        std::string rs1_map = BASE::register_map[rs1];
        std::string rs2_map = BASE::register_map[rs2];

        std::string imm_ = BASE::immValue(stoi(imm));

        std::string machine_code = imm_.substr(0, 7) + rs2_map + rs1_map + format.func3 + imm_.substr(7, 5) + format.opcode;
        output = BASE::binary_to_hex(machine_code);
    }
    return output;
}