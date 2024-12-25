#include "base.hh"
#include "b_type.hh"

//13 bit offset value 
std::string B_TYPE::offsetValue(long long int number)
{
   

    return BASE::int_to_binary_advanced(number,13);
}

std::string B_TYPE::encodingDecodingBInstruction(std::string operation, std::string o1, std::string o2, std::string o3, int line_number, std::unordered_map<std::string, int> &labels)
{
    std::string rs1 = o1;
    std::string rs2 = o2;
    std::string label = o3;

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

    if (!BASE::isNumericString(label))
    {

        if (labels.find(label) == labels.end())
        {

            std::stringstream se;
            se << "Error: Label " << label << " undefined  in line " << line_number << std::endl;
            output = output + se.str();
            isLineErrorFree = false;
        }
        else
        {
            long long int offset = 4 * (labels[label] - line_number + 1);

            if (offset < -4096 || offset > 4094)
            {
                std::stringstream se;
                se << "Error: Imm Value " << offset << " too large (Must be in range [-4096,4094]) in line " << line_number << std::endl;
                output = output + se.str();
                isLineErrorFree = false;
            }
        }
    }
    else
    {
        long long int offset = stoll(label);
        if (offset < -4096 || offset > 4094)
        {
            std::stringstream se;
            se << "Error: Imm Value " << offset << " too large (Must be in range [-4096,4094]) in line " << line_number << std::endl;
            output = output + se.str();
            isLineErrorFree = false;
        }
    }
    //Convert Error Free Instruction
    if (isLineErrorFree == true)
    {

        long long int offset;
        if (!BASE::isNumericString(label))
        {

            offset = 4 * (labels[label] - line_number);
        }
        else
        {
            offset = stoi(label);
        }

        B_TYPE::BInstruction format = B_TYPE::mappings_B[operation];
        std::string rs1_map = BASE::register_map[rs1];
        std::string rs2_map = BASE::register_map[rs2];

        std::string offset_ = B_TYPE::offsetValue(offset);

        std::string machine_code = offset_[0] + offset_.substr(2, 6) + rs2_map + rs1_map + format.func3 + offset_.substr(8, 4) + offset_[1] + format.opcode;

        output = BASE::binary_to_hex(machine_code);
       
    }
    return output;
}