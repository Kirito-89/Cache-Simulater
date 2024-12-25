#include "r_type.hh"
#include "base.hh"

std::string R_TYPE ::encodingDecodingRInstruction(std::string operation, std::string o1, std::string o2, std::string o3, int line_number)
{

    std::string rd = o1;
    std::string rs1 = o2;
    std::string rs2 = o3;
    bool isLineErrorFree = true;
    std::string output = "";
    //error checking 
    if (BASE::register_map.find(rd) == BASE::register_map.end())
    {
        std::stringstream se;
        se << "Error: Invalid register '" << rd << "' in line " << line_number;
        output = output + se.str();
        isLineErrorFree = false;
    }
    if (BASE::register_map.find(rs1) == BASE::register_map.end())
    {
        std::stringstream se;
        se << "Error: Invalid register '" << rs1 << "' in line " << line_number << std::endl;
        output = output + se.str();
        isLineErrorFree = false;
    }
    if (BASE::register_map.find(rs2) == BASE::register_map.end())
    {
        std::stringstream se;
        se << "Error: Invalid register '" << rs2 << "' in line " << line_number << std::endl;
        output = output + se.str();
        isLineErrorFree = false;
    }
    //convert if no error
    if (isLineErrorFree == true)
    {
        RInstruction format = R_TYPE::mappings_R[operation];
        std::string rd_num = BASE::register_map[rd];
        std::string rs1_num = BASE::register_map[rs1];
        std::string rs2_num = BASE::register_map[rs2];

        std::string machine_code = format.func7 + rs2_num + rs1_num + format.func3 + rd_num + format.opcode;

        output = BASE::binary_to_hex(machine_code);
    }
    return output;
}