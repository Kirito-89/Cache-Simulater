#include <fstream>
#include <iostream>
#include <unordered_map>
#include <string>
#include <sstream>
#include <cmath>
#include <algorithm>

namespace BASE
{

    inline unsigned long long int base2RepresentationOfNumber(long long int number)
    {

        int remainder = number % 2;
        int quotient = number / 2;

        if (quotient == 0)
        {
            return remainder;
        }
        return base2RepresentationOfNumber(quotient) * 10 + remainder;
    }

    inline std::string int_to_binary_advanced(long long int  n, long long int  x)
    {
        unsigned long long int  num = n;
        std::string binary = "";
        while (num)
        {
            binary += (num % 2) + '0';
            num /= 2;
        }
        int req = 64 - binary.size();
        while (req--)
        {
            binary += '0';
        }
        std::reverse(binary.begin(), binary.end());
        return binary.substr(64 - x);
    }
    // Check whether string is only composed of numbers
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
    // calculate immediate value for I and S (12 bits) type Instructions
    inline std::string immValue(long long int number)
    {
        // std::string binaryReprNumber;
        // if (number < 0)
        // {
        //     binaryReprNumber = std::to_string(base2RepresentationOfNumber((1LL << 12) - (-number)));
        //     while (binaryReprNumber.length() < 12)
        //     {
        //         binaryReprNumber = "1" + binaryReprNumber;
        //     }
        // }
        // else
        // {
        //     binaryReprNumber = std::to_string(base2RepresentationOfNumber(number));

        //     while (binaryReprNumber.length() < 12)
        //     {
        //         binaryReprNumber = "0" + binaryReprNumber;
        //     }
        // }

        return int_to_binary_advanced(number,12);
    }

    inline std::string strToLower(const std::string &input)
    {
        std::string result = input;
        for (auto &c : result)
        {
            c = tolower((unsigned char)c);
        }
        return result;
    }

    inline ::std::string binary_to_hex(std::string binary_machine_code)
    {

        std::unordered_map<std::string, char> binaryToHexMap = {
            {"0000", '0'}, {"0001", '1'}, {"0010", '2'}, {"0011", '3'}, {"0100", '4'}, {"0101", '5'}, {"0110", '6'}, {"0111", '7'}, {"1000", '8'}, {"1001", '9'}, {"1010", 'a'}, {"1011", 'b'}, {"1100", 'c'}, {"1101", 'd'}, {"1110", 'e'}, {"1111", 'f'}};

        std::string hex = "";

        for (int i = 0; i < binary_machine_code.length(); i += 4)
        {
            std::string four = binary_machine_code.substr(i, 4);
            hex += binaryToHexMap[four];
        }
        return hex;
    }
    // register map
    inline ::std::unordered_map<std::string, std::string> register_map = {
        {"x0", "00000"}, {"x1", "00001"}, {"ra", "00001"}, {"x2", "00010"}, {"sp", "00010"}, {"x3", "00011"}, {"gp", "00011"}, {"x4", "00100"}, {"tp", "00100"}, {"x5", "00101"}, {"t0", "00101"}, {"x6", "00110"}, {"t1", "00110"}, {"x7", "00111"}, {"t2", "00111"}, {"x8", "01000"}, {"s0", "01000"}, {"fp", "01000"}, {"x9", "01001"}, {"s1", "01001"}, {"x10", "01010"}, {"a0", "01010"}, {"x11", "01011"}, {"a1", "01011"}, {"x12", "01100"}, {"a2", "01100"}, {"x13", "01101"}, {"a3", "01101"}, {"x14", "01110"}, {"a4", "01110"}, {"x15", "01111"}, {"a5", "01111"}, {"x16", "10000"}, {"a6", "10000"}, {"x17", "10001"}, {"a7", "10001"}, {"x18", "10010"}, {"s2", "10010"}, {"x19", "10011"}, {"s3", "10011"}, {"x20", "10100"}, {"s4", "10100"}, {"x21", "10101"}, {"s5", "10101"}, {"x22", "10110"}, {"s6", "10110"}, {"x23", "10111"}, {"s7", "10111"}, {"x24", "11000"}, {"s8", "11000"}, {"x25", "11001"}, {"s9", "11001"}, {"x26", "11010"}, {"s10", "11010"}, {"x27", "11011"}, {"s11", "11011"}, {"x28", "11100"}, {"t3", "11100"}, {"x29", "11101"}, {"t4", "11101"}, {"x30", "11110"}, {"t5", "11110"}, {"x31", "11111"}, {"t6", "11111"}};
}
