#include "i_type.hh"
#include "base.hh"

std::string I_TYPE ::encodingDecodingIInstruction(std::string operation, std::string o1, std::string o2, std::string o3, int line_number)
{

    std::string output = "";
    //seperate for L type  (1 comma in Syntax so 2 Operands if parsed by ,)
    if (operation[0] == 'l' || operation == "jalr")
    {


        std::string rd = o1;
        size_t pos = o2.find('(');
      
        std::string imm = o2.substr(0, pos);

        std::string rs1 = o2.substr(pos + 1, o2.find(')') - pos - 1);
                bool isLineErrorFree = true;

   

        if (BASE::register_map.find(rs1) == BASE::register_map.end())
        {
            std::stringstream se;

            se << "Error: Invalid register '" << rs1 << "' in line " << line_number;
            isLineErrorFree = false;
        }
        if (BASE::register_map.find(rd) == BASE::register_map.end())
        {
            std::stringstream se;

            se << "Error: Invalid register '" << rd << "' in line " << line_number;
            output = output + se.str();
            isLineErrorFree = false;
        }
        if(imm== "" || !BASE::isNumericString(imm)){
            isLineErrorFree = false;
            output = "Error : Wrong Instruction (Incomplete)\n";

        }
        else{

        
        if (std::stoll(imm) < -2048 || std::stoll(imm) > 2047)
        {
            std::stringstream se;
            se << "Error: Imm Value " << imm << " too large (Must be in range [-2048,2047]) in line " << line_number;
            output = output + se.str();
            isLineErrorFree = false;
        }
        }
        if (isLineErrorFree == true)
        {

            I_TYPE ::IInstruction format = I_TYPE ::mappings_I[operation];
            std::string rs1_map = BASE::register_map[rs1];
            std::string rd_map = BASE::register_map[rd];

            std::string imm_ = BASE::immValue(stoll(imm));

            std::string machine_code = imm_ + rs1_map + format.func3 + rd_map + format.opcode;

            output = BASE::binary_to_hex(machine_code);
        }
    }
    //All I Instructions except L (2 comma in Syntax so 3 Operands if parsed by ,)
    else
    {
        std::string rd = o1;
        std::string rs1 = o2;
        std::string imm = o3;
      

        bool isLineErrorFree = true;
        

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

            se << "Error: Invalid register '" << rs1 << "' in line " << line_number;
            output = output + se.str();
            isLineErrorFree = false;
        }
          if (imm == ""){
            isLineErrorFree = false;
            output = "Error : Wrong Instruction(Incomplete)\n";

        }
        else  {

        
        if (operation == "slli" || operation == "srli" || operation == "srai")
        {
            if (std::stoll(imm) < 0 || std::stoll(imm) > 63)
            {
                std::stringstream se;
                se << "Error: Imm Value " << imm << " too large (Must be in range [0,63]) in line " << line_number;
                output = output + se.str();
                isLineErrorFree = false;
            }
        }
        else
        {
            if (std::stoll(imm) < -2048 || std::stoll(imm) > 2047)
            {
                std::stringstream se;
                se << "Error: Imm Value " << imm << " too large (Must be in range [-2048,2047]) in line " << line_number;
                output = output + se.str();

                isLineErrorFree = false;
            }
        }
        }
        if (isLineErrorFree)
        {
            I_TYPE::IInstruction format = I_TYPE::mappings_I[operation];
            std::string rd_num = BASE::register_map[rd];
            std::string rs1_num = BASE::register_map[rs1];

            std::string imm_ = BASE::immValue(stoi(o3));

            if (operation == "srai")
            {
                imm_.replace(0, 6, "010000");
            }
            if (operation == "slli" || operation == "srli")
            {
                imm_.replace(0, 6, "000000");
            }

            std::string machine_code = imm_ + rs1_num + format.func3 + rd_num + format.opcode;
            output = BASE::binary_to_hex(machine_code);
        }
    }
    return output;
}