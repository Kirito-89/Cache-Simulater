#include "base.hh"
#include "j_type.hh"

//21 bit offset Value 
std::string J_TYPE::offsetValueJInstruction(long long int number)
{
   

    return BASE::int_to_binary_advanced(number,21);
}

std::string J_TYPE::encodingDecodingJInstruction(std::string operation, std::string o1, std::string o2, std::string o3, int line_number, std::unordered_map<std::string, int> &labels)
{

    std::string rd = o1;

    std::string label = o2;

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
    if (!BASE::isNumericString(label))
    {
        if (labels.find(label) == labels.end())
        {
            std::stringstream se;
            se << "Error: Label " << label << " undefined  in line " << line_number;
            output = output + se.str();
            isLineErrorFree = false;
        }

        else
        {
           long long  int offset = 4 * (labels[label] - line_number);

            if (offset < -1048576 || offset > 1048574)
            {

                std::stringstream se;
                se << "Error: Imm Value " << offset << " too large (Must be in range [-1048576, 1048574] ) in line " << line_number;
                output = output + se.str();
                isLineErrorFree = false;
            }
        }
    }
    else
    {
       long long  int offset = stoll(label);
        if (offset < -1048576 || offset > 1048574)
        {
            std::stringstream se;
            se << "Error: Imm Value " << offset << " too large (Must be in range [-1048576 ,1048574]) in line " << line_number;
            output = output + se.str();
            isLineErrorFree = false;
        }
    }
    //if error free
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

        J_TYPE::JInstruction format = J_TYPE::mappings_J[operation];
        std::string rd_map = BASE::register_map[rd];

        std::string offset_ = J_TYPE::offsetValueJInstruction(offset);

        std::string machine_code = offset_[0] + offset_.substr(10, 10) + offset_[9] + offset_.substr(1, 8) + rd_map + format.opcode;
        output = BASE::binary_to_hex(machine_code);

    }
    return output;
}