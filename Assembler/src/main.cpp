#include <fstream>
#include <iostream>
#include <unordered_map>
#include <string>
#include <sstream>
#include <cmath>
#include <algorithm>
#include <cctype>
#include "r_type.hh"
#include "i_type.hh"
#include "s_type.hh"
#include "b_type.hh"
#include "j_type.hh"
#include "u_type.hh"

using namespace std;

//To handle extranous characters in the immediate
inline bool isNumericString(const std::string &str)
{
    bool isNumeric = true;
    for (char c : str)
    {
        if (!std::isdigit(c) && c != '-')
        {
            isNumeric = false;
            break;
        }
    }
    return isNumeric;
}
void removeLeadingSpaces(string &str)
{
    if (str.empty())
        return;

    size_t pos = str.find_first_not_of(' ');
    if (pos != string::npos)
    {
        str.erase(0, pos);
    }
}
//helper functions for string handling
void removeTrailingSpaces(string &str)
{
    if (str.empty())
        return;

    size_t pos = str.find_last_not_of(' ');
    str.erase(pos + 1);
}


string trimSpaces(string str)
{

    if (str.empty())
    {
        return "";
    }

    // Removing Leading Spaces
    size_t pos = str.find_first_not_of(' ');
    if (pos != string::npos)
    {
        str.erase(0, pos);
    }
    // Removing Trailing Spaces
    return str.erase(str.find_last_not_of(' ') + 1);
}

int main(int argc, char *argv[])
{
    //Reading File Input through Command Line 
    if (argc < 2)
    {
        cerr << "Error:Usage: " << argv[0] << " <input_file>" << endl;
        return 1;
    }

    string output_file_name;
    //Meeting the demaind of Lab Assignment
    if (argc == 2)
    {
        output_file_name = "output.hex";
    }
    //For Testing The Project with CTest
    else if (argc == 3)
    {
        string test_number = argv[2];
        output_file_name = "/home/panshul/Downloads/ai23btech11018_assembler_final/test/output/output" + test_number + ".hex";/////For checking my test results ,replace the output file path with //test/ouput/ folder 
    }

    string input_file_name = argv[1];

    ifstream file(input_file_name);

    ofstream fout(output_file_name);
    if (!fout.is_open())
    {
         fout<< "Error: Could not open output file " << output_file_name << endl;
        return 1;
    }

    // Creating the labels mapping by reading through the file once
    unordered_map<string, int> labels;

    if (file.is_open())
    {
        string line;
        int line_number = 1;

        while (getline(file, line))
        {


             if(trimSpaces(line) == ".data"){
                while(line != ".text"){
                    getline(file,line);
                }
             getline(file,line);

            }
            
            //empty line
            if (line.length() == 0)
            {
                continue;
            }
            //line containing only spaces
            if (all_of(line.begin(), line.end(), [](unsigned char c)
                       { return std::isspace(c); }))
            {

                continue; 
            }

           line =  trimSpaces(line);
            //handling comment
            if (line[0] == '#')
            {

                continue;
            }
            //handling inline label
            if (line[line.length() - 1] == ':')
            {

                string label = trimSpaces(line.substr(0, line.length() - 1));

                if (labels.find(label) != labels.end())
                {
                    fout << "Error:Redeclearation of Label " << label << " at line number " << line_number << endl;

                    exit(1);
                }

                labels.insert({label, line_number});

                continue;
            }
            //handling diff line label
            else if (line.find(':') != string::npos)
            {
                int pos = line.find(':');

                string label = trimSpaces(line.substr(0, pos));
                if (labels.find(label) != labels.end())
                {
                    fout << "Error:Redeclearation of Label " << label << " at line number " << line_number << endl;

                    exit(1);
                }
                labels.insert({label, line_number});

                line = line.substr(pos + 1);
            } 

            line_number++;
        }
    }

    file.clear();
    file.seekg(ios::beg);

    // Reading the file one by once
    if (file.is_open())
    {
        string line;
        int line_number = 1;//line number of input file
        int program_line_number = 1;//program control (current instruction number starting from 1)

        while (getline(file, line))
        { // Iterating through each instruction
            
            if(trimSpaces(line) == ".data"){
                while(line != ".text"){
                    getline(file,line);
                }
                getline(file,line);

            }
            line = trimSpaces(line);
            // Skipping the empty lines
            if (line.length() == 0)
            {
                line_number++;
                continue;
            }
            if (all_of(line.begin(), line.end(), [](unsigned char c)
                       { return std::isspace(c); }))
            {
                line_number++;
                continue; 
            }

            // skipping the comment   (Can Comment the Line with "#")

            if (line[0] == '#')
            {
                line_number++;
                continue;
            }
            // Comment inside the line 
            if (line.find('#') != string::npos)
            {
                line = line.substr(0, line.find('#') - 1);
                line.erase(line.find_last_not_of(' ') + 1); 
            }

            bool isLineErrorFree = true;

            // Skipping the Labels

            if (line[line.length() - 1] == ':') // Labels in diff line
            {
                line_number++;
                continue;
            }

            else if (line.find(':') != string::npos) // Labels in Same Line
            {
                int pos = line.find(':');

                string line2 = line.substr(pos + 1);
                removeLeadingSpaces(line2);
                line = line2;
            }

            /////////////////////////////Parse the Assembly Instructions//////////////////////////:

            stringstream ss(line);
            string operation, o1, o2, o3;
            string word;
            int index = 0;
            bool validInstruction = true;
            while (getline(ss, word, ','))
            {

                switch (index)
                {
                case 0:
                {

                    int pos_first_space = word.find_first_of(' ');
                    operation = word.substr(0, pos_first_space);
                    o1 = trimSpaces(word.substr(pos_first_space + 1));
                }
                case 1:

                    o2 = trimSpaces(word);

                    break;
                case 2:

                    o3 = trimSpaces(word);

                    break;

                default:
                    fout << "Error: Invalid Instruction {Extra Characters at end}" << endl;
                    validInstruction = false;
                }
                index++;
            }
            if (validInstruction == false)
            {
                continue;
            }
        


            if ((R_TYPE::mappings_R.find(operation) != R_TYPE::mappings_R.end()))
            {
                fout << R_TYPE::encodingDecodingRInstruction(operation, o1, o2, o3, line_number) << endl;
            }
            else if (I_TYPE::mappings_I.find(operation) != I_TYPE::mappings_I.end())
            {

                if ((!isNumericString(o3)))
                {
                    fout << "Error : Invalid Immediate" << endl;
                }
                else
                {
                    fout << I_TYPE::encodingDecodingIInstruction(operation, o1, o2, o3, line_number) << endl;
                }
            }
            else if (S_TYPE::mappings_S.find(operation) != S_TYPE::mappings_S.end())
            {
                fout << S_TYPE::encodingDecodingSInstruction(operation, o1, o2, o3, line_number) << endl;
            }
            else if (B_TYPE::mappings_B.find(operation) != B_TYPE::mappings_B.end())
            {
                fout << B_TYPE::encodingDecodingBInstruction(operation, o1, o2, o3, program_line_number, labels) << endl;
            }
            else if (J_TYPE::mappings_J.find(operation) != J_TYPE::mappings_J.end())
            {
                fout << J_TYPE::encodingDecodingJInstruction(operation, o1, o2, o3, program_line_number, labels) << endl;
            }

            else if (U_TYPE::mappings_U.find(operation) != U_TYPE::mappings_U.end())
            {
                fout << U_TYPE::encodingDecodingUInstruction(operation, o1, o2, o3, line_number) << endl;
            }

            else
            {
                fout  << "Error: Unknown instruction '" << operation << "' in line " << line_number << std::endl;

                isLineErrorFree = false;
            }
            line_number++;
            program_line_number++;
        }
    }
    else
    {
        fout << "Error: Unable to open file " << input_file_name << endl;

        exit(EXIT_FAILURE);
    }
    return 0;
}