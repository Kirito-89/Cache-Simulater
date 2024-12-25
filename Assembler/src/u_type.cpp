#include "base.hh"
#include "u_type.hh"
std::string U_TYPE::offsetValueU(long long int number)
{ 
    return BASE::int_to_binary_advanced(number,20);
}



std::string  U_TYPE :: encodingDecodingUInstruction(std::string operation, std::string o1, std::string o2, std::string o3, int line_number)
{
    std::string rd = o1;

    std::string imm = o2;

    bool immIshex = false;
    bool isLineErrorFree = true;
        std::string output ="";

    if (BASE::register_map.find(rd) == BASE::register_map.end())
    {
           std:: stringstream se ;
        se << "Error: Invalid register '" << rd << "' in line " << line_number ;
                           output = output+  se.str() ;
        isLineErrorFree = false;
    }
      if(imm== ""){
            isLineErrorFree = false;
            output = "Error : Wrong Instruction (Incomplete)\n";

        }
        else{

        

    if (imm.substr(0, 2) == "0x")
    {
        immIshex = true;
        if (imm.length() > 7)
        {
             std:: stringstream se ;
            se<< "Error: Invalid size of immediate '" << imm << "' in line " << line_number ;
              output = output+  se.str() ;
            isLineErrorFree = false;
        }

    }

    else
    {
        if (std::stoll(imm) < 0 || std::stoll(imm) >  1048575)
        {
             std:: stringstream se ;
            se << "Error: Invalid size of immediate  '" << imm << "' in line " << line_number << " Must be [0:1,048,575]";
                           output = output+  se.str() ;
            isLineErrorFree = false;
        }
    }
        }
    if (isLineErrorFree == true)
    {

        U_TYPE :: UInstruction format = mappings_U[operation];
        std::string rd_map = BASE::register_map[rd];

        if (immIshex)
        {
            imm = imm.substr(2);
            if(imm.length() !=7){
                    while(imm.length() <5){
                        imm =  "0"+ imm ;
            }   
            imm = "0x" + imm ;
           
            }
            
            std::string temp = rd_map + format.opcode;
            std::string machine_code = BASE::strToLower(imm.substr(2)) + BASE::binary_to_hex(temp);
            output =   machine_code;
        }
        else
        {
            std::string machine_code =  U_TYPE:: offsetValueU(stoi(imm)) + rd_map + format.opcode;
               output =   BASE::binary_to_hex(machine_code);
        }
    }
    return output;
}