#include <bits/stdc++.h>
#include "helper.h"
using namespace std;
#define ll long long int
#define ull unsigned long long int

enum Fields
{
    VALID,
    DIRTY,
    TAG,
    DATA,
    TIMESTAMP
};

ll base_address_data_section = 65536;       // base  address of data section(= 0x100000 )
ll base_address_stack_section = 327680 - 1; // 0x50000 as addressing starts from zero
ll stack_pointer = 0;

// vector to store values  of the registers
std::vector<std::pair<std::string, std::string>> registers = {
    {"x0", "0"}, {"x1", "0"}, {"x2", "0"}, {"x3", "0"}, {"x4", "0"}, {"x5", "0"}, {"x6", "0"}, {"x7", "0"}, {"x8", "0"}, {"x9", "0"}, {"x10", "0"}, {"x11", "0"}, {"x12", "0"}, {"x13", "0"}, {"x14", "0"}, {"x15", "0"}, {"x16", "0"}, {"x17", "0"}, {"x18", "0"}, {"x19", "0"}, {"x20", "0"}, {"x21", "0"}, {"x22", "0"}, {"x23", "0"}, {"x24", "0"}, {"x25", "0"}, {"x26", "0"}, {"x27", "0"}, {"x28", "0"}, {"x29", "0"}, {"x30", "0"}, {"x31", "0"}};

vector<string> memory(327680, "00");
// Map containing the aliases of the registers
map<string, string> register_map = {{"zero", "x0"}, {"ra", "x1"}, {"sp", "x2"}, {"gp", "x3"}, {"tp", "x4"}, {"t0", "x5"}, {"t1", "x6"}, {"t2", "x7"}, {"s0", "x8"}, {"fp", "x8"}, {"s1", "x9"}, {"a0", "x10"}, {"a1", "x11"}, {"a2", "x12"}, {"a3", "x13"}, {"a4", "x14"}, {"a5", "x15"}, {"a6", "x16"}, {"a7", "x17"}, {"s2", "x18"}, {"s3", "x19"}, {"s4", "x20"}, {"s5", "x21"}, {"s6", "x22"}, {"s7", "x23"}, {"s8", "x24"}, {"s9", "x25"}, {"s10", "x26"}, {"s11", "x27"}, {"t3", "x28"}, {"t4", "x29"}, {"t5", "x30"}, {"t6", "x31"}};

map<string, string> opcodeR_map = {{"add", "0110011"}, {"sub", "0110011"}, {"xor", "0110011"}, {"or", "0110011"}, {"and", "0110011"}, {"sll", "0110011"}, {"srl", "0110011"}, {"sra", "0110011"}, {"slt", "0110011"}, {"sltu", "0110011"}};

// Opcode mapping for I format
map<string, string> opcodeI_map = {{"addi", "0010011"}, {"xori", "0010011"}, {"ori", "0010011"}, {"andi", "0010011"}, {"slli", "0010011"}, {"srli", "0010011"}, {"srai", "0010011"}, {"slti", "0010011"}, {"sltiu", "0010011"}, {"lb", "0000011"}, {"lh", "0000011"}, {"lw", "0000011"}, {"ld", "0000011"}, {"lbu", "0000011"}, {"lhu", "0000011"}, {"lwu", "0000011"}}; // {"jalr", "1100111"}

// Opcode mapping for S format
map<string, string> opcodeS_map = {{"sb", "0100011"}, {"sh", "0100011"}, {"sw", "0100011"}, {"sd", "0100011"}};

// Opcode mapping for B format
map<string, string> opcodeB_map = {{"beq", "1100011"}, {"bne", "1100011"}, {"blt", "1100011"}, {"bge", "1100011"}, {"bltu", "1100011"}, {"bgeu", "1100011"}};

// Opcode mapping for J format
map<string, string> opcodeJ_map = {{"jal", "1101111"}};

// Opcode mapping for U format
map<string, string> opcodeU_map = {{"lui", "0110111"}, {"auipc", "0010111"}};

// special I instructions that contains funct6 bits
set<string> specialI = {{"slli"}, {"srli"}, {"srai"}};

// In I format load Instructions
set<string> specialLoadI = {{"lb"}, {"lh"}, {"lw"}, {"ld"}, {"lbu"}, {"lhu"}, {"lwu"}};

int ptr_to_memory = base_address_data_section; // used for storing data in  .data section in memory

// caches implementation start

ll address_bits = 20;
ll hits = 0;
ll misses = 0;
ll write_backs = 0;
bool isCacheFull = false;
bool cache_enable_status = false;
// string loaded_file_name="";
ofstream cache_logs;

// Cache Configuration
ll SIZE_OF_CACHE;
ll BLOCK_SIZE;
ll ASSOCIATIVITY;
string REPLACEMENT_POLICY;
string WRITEBACK_POLICY;

ll no_of_sets;

ofstream logfile;

// DIRECT MAPPED CACHE
// 2nd string is for dirty bit
map<string, tuple<string, string, string, vector<string>>> direct_mapped;
vector<string> parse_address(ll address, ll block_size, ll cache_size)
{
    vector<string> res;
    string address_in_binary = int_to_binary_advanced(address, 20);

    // rounding up in case of decimal bits
    ll index_bits = ceil(log2((cache_size / (float)block_size)));

    ll block_offset = log2((float)block_size); // block offset bits

    ll tag_bits = address_bits - block_offset - index_bits;

    string extracted_tag = address_in_binary.substr(0, tag_bits);
    string extracted_index = address_in_binary.substr(tag_bits, index_bits);
    string extracted_offset = address_in_binary.substr(tag_bits + index_bits, block_offset);
    res.push_back(extracted_tag);
    res.push_back(extracted_index);
    res.push_back(extracted_offset);
    return res;
}
tuple<bool, vector<string>, tuple<string, string, string, vector<string>>> direct_mapped_cache_initialize(ll address, ll block_size, ll cache_size, bool allocate)
{
    string address_in_binary = int_to_binary_advanced(address, 20);

    // rounding up in case of decimal bits
    ll index_bits = ceil(log2((cache_size / (float)block_size)));

    ll block_offset = log2((float)block_size); // block offset bits

    ll tag_bits = address_bits - block_offset - index_bits;

    string extracted_tag = address_in_binary.substr(0, tag_bits);
    string extracted_index = address_in_binary.substr(tag_bits, index_bits);
    string extracted_offset = address_in_binary.substr(tag_bits + index_bits, block_offset);

    if (get<VALID>(direct_mapped[extracted_index]) == "1")
    {
        // valid entry is there
        if (get<TAG>(direct_mapped[extracted_index]) == extracted_tag)
        { // tag matches
            hits++;
            return {true, {get<DATA>(direct_mapped[extracted_index])}, {"", "", "", {}}};
        }
        else
        {
            // tag mismatch simple policy just directly replace the data
            // still need to check it's correctness
            tuple<string, string, string, vector<string>> victim_block = direct_mapped[extracted_index];

            if (allocate)
            {
                ll number_of_bytes_to_store = block_size;

                get<TAG>(direct_mapped[extracted_index]) = extracted_tag;
                while (number_of_bytes_to_store--)
                {
                    get<DATA>(direct_mapped[extracted_index]).push_back(memory[address]);
                    address++;
                }
                get<DIRTY>(direct_mapped[extracted_index]) = "0";
            }

            misses++;
            vector<string> v = {""};
            return {false, v, victim_block};
        }
    }
    else
    {
        // not valid entry is there then fetch from the memory
        if (allocate)
        {
            ll number_of_bytes_to_store = block_size;
            get<VALID>(direct_mapped[extracted_index]) = "1";
            get<TAG>(direct_mapped[extracted_index]) = extracted_tag;
            while (number_of_bytes_to_store--)
            {
                get<DATA>(direct_mapped[extracted_index]).push_back(memory[address]);
                address++;
            }
            get<DIRTY>(direct_mapped[extracted_index]) = "0";
        }

        misses++;

        vector<string> v = {""};
        return {false, v, {"", "", "", {}}};
    }
}

// FULLY ASSOCIATIVE CACHE
vector<tuple<string, string, string, vector<string>, int>> fully_associative;
ll index_for_entry_in_fully_associative = 0; // minimal
int time_stamp = 1;
tuple<bool, vector<string>, ll, tuple<string, string, string, vector<string>, int>> fully_associative_initialize(ll address, ll block_size, ll cache_size, bool allocate)
{
    string address_in_binary = int_to_binary_advanced(address, 20);

    // no index bits

    ll block_offset = log2((float)block_size); // block offset bits

    ll tag_bits = address_bits - block_offset;

    string extracted_tag = address_in_binary.substr(0, tag_bits);
    string extracted_offset = address_in_binary.substr(tag_bits, block_offset);

    // CASE 1 :Element Found

    for (ll i = 0; i < fully_associative.size(); i++)
    {

        if (get<VALID>(fully_associative[i]) == "1")
        {
            if (get<TAG>(fully_associative[i]) == extracted_tag)
            {
                hits++;

                get<TIMESTAMP>(fully_associative[i]) = time_stamp;

                return {true, get<DATA>(fully_associative[i]), i, {"", "", "", {}, time_stamp}};
            }
        }
    }
    // CASE 2 :Element NOT Found
    misses++;
    // check whehter cache is full
    if (index_for_entry_in_fully_associative == SIZE_OF_CACHE / BLOCK_SIZE)
    {
        isCacheFull = true;

        if (allocate)
        {
            tuple<string, string, string, vector<string>, int> victim_block;
            // Replace by using Replacement Policies
            if (REPLACEMENT_POLICY == "RANDOM")
            {
                // choose a random number between 0 to fully_associative.size()-1
                ll randomly_choosen_block_index = rand() % fully_associative.size();

                ll number_of_bytes_to_store = block_size;
                victim_block = fully_associative[randomly_choosen_block_index];

                get<TAG>(fully_associative[randomly_choosen_block_index]) = extracted_tag;

                get<DATA>(fully_associative[randomly_choosen_block_index]).clear();
                while (number_of_bytes_to_store--)
                {
                    get<DATA>(fully_associative[randomly_choosen_block_index]).push_back(memory[address]);
                    address++;
                }
                get<DIRTY>(fully_associative[randomly_choosen_block_index]) = "0";
            }
            else if (REPLACEMENT_POLICY == "FIFO")
            {
                victim_block = fully_associative[0];

                for (int i = 0; i < fully_associative.size() - 1; i++)
                {
                    fully_associative[i] = fully_associative[i + 1];
                }

                ll number_of_bytes_to_store = block_size;

                get<TAG>(fully_associative[index_for_entry_in_fully_associative - 1]) = extracted_tag;

                get<DATA>(fully_associative[index_for_entry_in_fully_associative - 1]).clear();
                while (number_of_bytes_to_store--)
                {
                    get<DATA>(fully_associative[index_for_entry_in_fully_associative - 1]).push_back(memory[address]);
                    address++;
                }
                get<DIRTY>(fully_associative[index_for_entry_in_fully_associative - 1]) = "0";
            }
            else if (REPLACEMENT_POLICY == "LRU")
            {
                // Find the block with least timestamp
                int min_time_stamp_block_index = 0;
                ll min_time_stamp = INT_MAX;

                for (int i = 0; i < fully_associative.size(); i++)
                {
                    // logfile << get<TIMESTAMP>(fully_associative[i]) << " ";
                    if (get<TIMESTAMP>(fully_associative[i]) < min_time_stamp)
                    {

                        min_time_stamp_block_index = i;
                        min_time_stamp = get<TIMESTAMP>(fully_associative[i]);
                    }
                }
                // logfile << "Min Time Stamp Block Index " << min_time_stamp_block_index << " Min Time Stamp " << min_time_stamp << endl;
                victim_block = fully_associative[min_time_stamp_block_index];
                ll number_of_bytes_to_store = block_size;

                get<TAG>(fully_associative[min_time_stamp_block_index]) = extracted_tag;
                get<TIMESTAMP>(fully_associative[min_time_stamp_block_index]) = time_stamp;

                get<DATA>(fully_associative[min_time_stamp_block_index]).clear();

                while (number_of_bytes_to_store--)
                {
                    get<DATA>(fully_associative[min_time_stamp_block_index]).push_back(memory[address]);
                    address++;
                }
                get<DIRTY>(fully_associative[min_time_stamp_block_index]) = "0";
            }

            return {false, {}, -1, victim_block};
        }
    }

    else
    {
        if (allocate)
        {

            ll number_of_bytes_to_store = block_size;
            get<VALID>(fully_associative[index_for_entry_in_fully_associative]) = "1";
            get<TAG>(fully_associative[index_for_entry_in_fully_associative]) = extracted_tag;
            while (number_of_bytes_to_store--)
            {
                get<DATA>(fully_associative[index_for_entry_in_fully_associative]).push_back(memory[address]);
                address++;
            }
            get<TIMESTAMP>(fully_associative[index_for_entry_in_fully_associative]) = time_stamp;
            get<DIRTY>(fully_associative[index_for_entry_in_fully_associative]) = "0";
            index_for_entry_in_fully_associative++;
        }
    }

    vector<string> v = {""};
    return {false, v, -1, {"", "", "", {}, time_stamp}};
}

// SET ASSOCIATIVE CACHE
map<string, vector<tuple<string, string, string, vector<string>, int>>> set_associative;
// vector<int> index_for_entry((SIZE_OF_CACHE / BLOCK_SIZE) / ASSOCIATIVITY, 0);
vector<int> index_for_entry;
// now for the set associate part i have to see how to handle this i guess isko fifo ke tarah use kar skte hai
tuple<bool, vector<string>, ll, tuple<string, string, string, vector<string>, int>> set_associative_initialise(ll address, ll block_size, ll cache_size, ll associative, bool allocate)
{

    string address_in_binary = int_to_binary_advanced(address, 20);

    // rounding up in case of decimal bits
    ll index_bits = ceil(log2((cache_size / (float)block_size) / associative)); // set index bits

    ll block_offset = log2((float)block_size); // block offset bits

    ll tag_bits = address_bits - block_offset - index_bits;

    string extracted_tag = address_in_binary.substr(0, tag_bits);
    string extracted_index = address_in_binary.substr(tag_bits, index_bits);
    string extracted_offset = address_in_binary.substr(tag_bits + index_bits, block_offset);

    vector<tuple<string, string, string, vector<string>, int>> &matched_set = set_associative[extracted_index]; // us set pe aa gaye jaha check karna hai
    int &index_for_entry_inside_set = index_for_entry[binary_to_int(extracted_index)];
    // CASE 1 :Element Found

    for (ll i = 0; i < associative; i++)
    {

        if (get<VALID>(matched_set[i]) == "1")
        {
            if (get<TAG>(matched_set[i]) == extracted_tag)
            {
                hits++;
                get<TIMESTAMP>(matched_set[i]) = time_stamp;
                return {true, get<DATA>(matched_set[i]), i, {"", "", "", {}, time_stamp}};
            }
        }
    }
    // CASE 2 :Element NOT Found
    misses++;
    if (index_for_entry_inside_set == matched_set.size()) // can change matched_set to associativity
    {

        if (allocate)
        {
            tuple<string, string, string, vector<string>, int> victim_block;

            if (REPLACEMENT_POLICY == "RANDOM")
            {
                // choose a random number between 0 to fully_associative.size()-1
                ll randomly_choosen_block_index = rand() % associative;

                ll number_of_bytes_to_store = block_size;
                victim_block = matched_set[randomly_choosen_block_index];

                get<TAG>(matched_set[randomly_choosen_block_index]) = extracted_tag;

                get<DATA>(matched_set[randomly_choosen_block_index]).clear();
                while (number_of_bytes_to_store--)
                {
                    get<DATA>(matched_set[randomly_choosen_block_index]).push_back(memory[address]);
                    address++;
                }
                get<DIRTY>(matched_set[randomly_choosen_block_index]) = "0";
            }

            else if (REPLACEMENT_POLICY == "FIFO")
            {

                victim_block = matched_set[0];

                for (int i = 0; i < associative - 1; i++)
                {
                    matched_set[i] = matched_set[i + 1];
                }

                ll number_of_bytes_to_store = block_size;

                get<TAG>(matched_set[index_for_entry_inside_set - 1]) = extracted_tag;

                get<DATA>(matched_set[index_for_entry_inside_set - 1]).clear();
                while (number_of_bytes_to_store--)
                {
                    get<DATA>(matched_set[index_for_entry_inside_set - 1]).push_back(memory[address]);
                    address++;
                }
                get<DIRTY>(matched_set[index_for_entry_inside_set - 1]) = "0";
            }

            else if (REPLACEMENT_POLICY == "LRU")
            {
                // Find the block with least timestamp
                int min_time_stamp_block_index = 0;
                ll min_time_stamp = INT_MAX;

                for (int i = 0; i < associative; i++)
                {

                    // logfile << get<TIMESTAMP>(matched_set[i]) << " ";

                    if (get<TIMESTAMP>(matched_set[i]) < min_time_stamp)
                    {

                        min_time_stamp_block_index = i;
                        min_time_stamp = get<TIMESTAMP>(matched_set[i]);
                    }
                }
                // logfile << "Min Time Stamp Block Index " << min_time_stamp_block_index << " Min Time Stamp " << min_time_stamp << endl;
                victim_block = matched_set[min_time_stamp_block_index];
                ll number_of_bytes_to_store = block_size;

                get<TAG>(matched_set[min_time_stamp_block_index]) = extracted_tag;
                get<TIMESTAMP>(matched_set[min_time_stamp_block_index]) = time_stamp;

                get<DATA>(matched_set[min_time_stamp_block_index]).clear();

                while (number_of_bytes_to_store--)
                {
                    get<DATA>(matched_set[min_time_stamp_block_index]).push_back(memory[address]);
                    address++;
                }
                get<DIRTY>(matched_set[min_time_stamp_block_index]) = "0";
            }
            return {false, {}, -1, victim_block};
        }
    }
    else
    {
        if (allocate)
        {
            ll number_of_bytes_to_store = block_size;
            get<VALID>(matched_set[index_for_entry_inside_set]) = "1";
            get<TAG>(matched_set[index_for_entry_inside_set]) = extracted_tag;
            while (number_of_bytes_to_store--)
            {
                get<DATA>(matched_set[index_for_entry_inside_set]).push_back(memory[address]);
                address++;
            }
            get<TIMESTAMP>(matched_set[index_for_entry_inside_set]) = time_stamp;
            index_for_entry_inside_set++;
        }
    }

    vector<string> v = {""};
    return {false, v, -1, {"", "", "", {}, time_stamp}};
}

// function to store the values given in the .data section
void store_data(string s)
{
    vector<string> v = get_numbers(s);
    string type = give_type(s);
    if (type == ".dword")
    {
        for (int i = 0; i < v.size(); i++)
        {
            ll value = stoll(v[i]);
            string hex_str = binary_to_hexa(int_to_binary_advanced(value, 64));
            reverse(hex_str.begin(), hex_str.end());
            for (int i = 0; i < 16; i += 2)
            {
                string res = hex_str.substr(i, 2);
                reverse(res.begin(), res.end());
                memory[ptr_to_memory++] = res;
            }
        }
    }
    if (type == ".word")
    {
        for (int i = 0; i < v.size(); i++)
        {
            ll value = stoll(v[i]);
            string hex_str = binary_to_hexa(int_to_binary_advanced(value, 64));
            hex_str = hex_str.substr(8);
            reverse(hex_str.begin(), hex_str.end());
            for (int i = 0; i < 8; i += 2)
            {
                string res = hex_str.substr(i, 2);
                reverse(res.begin(), res.end());
                memory[ptr_to_memory++] = res;
            }
        }
    }
    if (type == ".half")
    {
        for (int i = 0; i < v.size(); i++)
        {
            ll value = stoll(v[i]);
            string hex_str = binary_to_hexa(int_to_binary_advanced(value, 64));
            hex_str = hex_str.substr(12);
            reverse(hex_str.begin(), hex_str.end());
            for (int i = 0; i < 4; i += 2)
            {
                string res = hex_str.substr(i, 2);
                reverse(res.begin(), res.end());
                memory[ptr_to_memory++] = res;
            }
        }
    }
    if (type == ".byte")
    {
        for (int i = 0; i < v.size(); i++)
        {
            ll value = stoll(v[i]);
            string hex_str = binary_to_hexa(int_to_binary_advanced(value, 64));
            hex_str = hex_str.substr(14);
            reverse(hex_str.begin(), hex_str.end());
            for (int i = 0; i < 2; i += 2)
            {
                string res = hex_str.substr(i, 2);
                reverse(res.begin(), res.end());
                memory[ptr_to_memory++] = res;
            }
        }
    }
}

// function to handle R type Instructions
void handleR(vector<string> v)
{

    if (register_map.count(v[1]))
    { // check if alternate register name is present
        v[1] = register_map[v[1]];
    }
    if (register_map.count(v[2]))
    { // check if alternate register name is present
        v[2] = register_map[v[2]];
    }
    if (register_map.count(v[3]))
    { // check if alternate register name is present
        v[3] = register_map[v[3]];
    }
    string rd = v[1], rs1 = v[2], rs2 = v[3];

    if (v[0] == "add")
    {
        ll rs1_value = stoll(registers[stoll(rs1.substr(1))].second);

        ll rs2_value = stoll(registers[stoll(rs2.substr(1))].second);
        ll rd_val = rs1_value + rs2_value;
        registers[stoll(rd.substr(1))].second = to_string(rd_val);
    }
    if (v[0] == "sub")
    {
        ll rs1_value = stoll(registers[stoll(rs1.substr(1))].second);
        ll rs2_value = stoll(registers[stoll(rs2.substr(1))].second);
        ll rd_val = rs1_value - rs2_value;
        registers[stoll(rd.substr(1))].second = to_string(rd_val);
    }
    if (v[0] == "xor")
    {
        ll rs1_value = stoll(registers[stoll(rs1.substr(1))].second);
        ll rs2_value = stoll(registers[stoll(rs2.substr(1))].second);
        ll rd_val = rs1_value ^ rs2_value;
        registers[stoll(rd.substr(1))].second = to_string(rd_val);
    }
    if (v[0] == "or")
    {
        ll rs1_value = stoll(registers[stoll(rs1.substr(1))].second);
        ll rs2_value = stoll(registers[stoll(rs2.substr(1))].second);
        ll rd_val = rs1_value | rs2_value;
        registers[stoll(rd.substr(1))].second = to_string(rd_val);
    }
    if (v[0] == "and")
    {
        ll rs1_value = stoll(registers[stoll(rs1.substr(1))].second);
        ll rs2_value = stoll(registers[stoll(rs2.substr(1))].second);
        ll rd_val = rs1_value & rs2_value;
        registers[stoll(rd.substr(1))].second = to_string(rd_val);
    }
    if (v[0] == "sll")
    {
        ll rs1_value = stoll(registers[stoll(rs1.substr(1))].second);
        ll rs2_value = stoll(registers[stoll(rs2.substr(1))].second);
        ll rd_val = rs1_value * (1 << rs2_value);
        registers[stoll(rd.substr(1))].second = to_string(rd_val);
    }
    if (v[0] == "srl")
    {
        ull rs1_value = stoll(registers[stoll(rs1.substr(1))].second);
        ull rs2_value = stoll(registers[stoll(rs2.substr(1))].second);
        ull rd_val = rs1_value >> rs2_value;
        registers[stoll(rd.substr(1))].second = to_string(rd_val);
    }
    // I think I need to change something here
    if (v[0] == "sra")
    {
        ll rs1_value = stoll(registers[stoll(rs1.substr(1))].second);
        ll rs2_value = stoll(registers[stoll(rs2.substr(1))].second);
        ll rd_val = rs1_value / (1 << rs2_value);
        registers[stoll(rd.substr(1))].second = to_string(rd_val);
    }
    if (v[0] == "slt")
    {
        ll rs1_value = stoll(registers[stoll(rs1.substr(1))].second);
        ll rs2_value = stoll(registers[stoll(rs2.substr(1))].second);
        if (rs1_value < rs2_value)
        {
            ll rd_val = 1;
            registers[stoll(rd.substr(1))].second = to_string(rd_val);
        }
        else
        {
            ll rd_val = 0;
            registers[stoll(rd.substr(1))].second = to_string(rd_val);
        }
    }
    if (v[0] == "sltu")
    {
        ull rs1_value = stoll(registers[stoll(rs1.substr(1))].second);
        ull rs2_value = stoll(registers[stoll(rs2.substr(1))].second);
        if (rs1_value < rs2_value)
        {
            int rd_val = 1;
            registers[stoll(rd.substr(1))].second = to_string(rd_val);
        }
        else
        {
            int rd_val = 0;
            registers[stoll(rd.substr(1))].second = to_string(rd_val);
        }
    }
    registers[0].second = "0";
}

// function to handle I type Instructions
void handleI(vector<string> v)
{

    if (register_map.count(v[1]))
    { // check if alternate register name is present
        v[1] = register_map[v[1]];
    }
    if (register_map.count(v[2]))
    { // check if alternate register name is present
        v[2] = register_map[v[2]];
    }
    string rd = v[1], rs1 = v[2], imm = v[3];
    ll rd_val;
    ll rs1_value = stoll(registers[stoll(rs1.substr(1))].second);

    ull imm_value = stoll(imm);
    if (v[0] == "addi")
    {

        rd_val = rs1_value + imm_value;
    }
    if (v[0] == "xori")
    {

        rd_val = rs1_value ^ imm_value;
    }
    if (v[0] == "ori")
    {

        rd_val = rs1_value | imm_value;
    }
    if (v[0] == "andi")
    {

        rd_val = rs1_value & imm_value;
    }
    if (v[0] == "slli")
    {

        rd_val = rs1_value * (1 << imm_value);
    }
    if (v[0] == "srli")
    {

        ull rs1_val = rs1_value;
        rd_val = rs1_val >> imm_value;
    }
    if (v[0] == "srai")
    {

        double rd_value = (rs1_value / (double)(1 << imm_value));
        if (rd_value < 0.00)
        {

            rd_val = (ll)floor(rd_value);
        }
        else
        {

            rd_val = (ll)ceil(rd_value);
        }
    }
    if (v[0] == "slti")
    {

        ll imm_value = stoll(imm);
        if (rs1_value < imm_value)
        {
            rd_val = 1;
        }
        else
        {
            rd_val = 0;
        }
    }

    if (v[0] == "sltiu")
    {

        ull rs1_val = rs1_value;
        if (rs1_val < imm_value)
        {
            rd_val = 1;
        }
        else
        {
            rd_val = 0;
        }
    }
    registers[stoll(rd.substr(1))].second = to_string(rd_val);
    registers[0].second = "0";
}

// function to handle U type Instructions
void handleU(vector<string> v, int PC)
{
    if (register_map.count(v[1]))
    {
        v[1] = register_map[v[1]];
    }
    string rd = v[1];
    string imm = v[2];

    if (v[0] == "auipc")
    {
        registers[stoll(rd.substr(1))].second = PC + stoll(imm, nullptr, 16);
        return;
    }

    int t = 3;
    while (t--)
    {
        imm += '0';
    }
    imm = imm.substr(2);
    ll value = stoll(imm, nullptr, 16);
    registers[stoll(rd.substr(1))].second = to_string(value);
}

// function to handle S type Instructions
void handleS_with_size(vector<string> v, int number_of_bytes_to_store)
{
    string rs1, rs2, offset;

    rs2 = v[1];
    size_t pos1 = v[2].find('(');
    size_t pos2 = v[2].find(')');
    offset = v[2].substr(0, pos1);
    offset = trimSpaces(offset);
    rs1 = v[2].substr(pos1 + 1, pos2 - pos1 - 1);
    rs1 = trimSpaces(rs1);
    if (register_map.count(rs1))
    {
        rs1 = register_map[rs1];
    }
    if (register_map.count(rs2))
    {
        rs2 = register_map[rs2];
    }
    if (stoll(registers[stoll(rs1.substr(1))].second) < 65536)
    {
        cout << "Can not write to text memory" << endl;
        return;
    }
    ll address = stoll(registers[stoll(rs1.substr(1))].second);
    ll fetch_to = address + stoll(offset); // here is the address of the current fetch
    ll value = stoll(registers[stoll(rs2.substr(1))].second);
    string address_in_binary = int_to_binary_advanced(fetch_to, 20);
    // vector<string> extracted_info=parse_address(fetch_to,BLOCK_SIZE,SIZE_OF_CACHE);
    if (cache_enable_status)
    {
        // cache_logs<<"W: "<<"Address: "<< binary_to_hexa(int_to_binary_advanced(fetch_to,20))<<", ";
        tuple<bool, vector<string>, tuple<string, string, string, vector<string>>> cache_check_direct_mapped;
        tuple<bool, vector<string>, int, tuple<string, string, string, vector<string>, int>> cache_check_fully_associative;
        tuple<bool, vector<string>, int, tuple<string, string, string, vector<string>, int>> cache_check_set_associative;
        if (ASSOCIATIVITY == 1)
        {
            if (WRITEBACK_POLICY == "WT")
            {
                cache_check_direct_mapped = direct_mapped_cache_initialize(fetch_to, BLOCK_SIZE, SIZE_OF_CACHE, false);
            }
            else
            {
                cache_check_direct_mapped = direct_mapped_cache_initialize(fetch_to, BLOCK_SIZE, SIZE_OF_CACHE, true);
            }
        }
        else if (ASSOCIATIVITY == 0)
        {

            if (WRITEBACK_POLICY == "WT")
            {
                cache_check_fully_associative = fully_associative_initialize(fetch_to, BLOCK_SIZE, SIZE_OF_CACHE, false);
            }
            else
            {
                cache_check_fully_associative = fully_associative_initialize(fetch_to, BLOCK_SIZE, SIZE_OF_CACHE, true);
            }
            time_stamp++;
        }
        else
        {
            if (WRITEBACK_POLICY == "WT")
            {
                cache_check_set_associative = set_associative_initialise(fetch_to, BLOCK_SIZE, SIZE_OF_CACHE, ASSOCIATIVITY, false);
            }
            else
            {
                cache_check_set_associative = set_associative_initialise(fetch_to, BLOCK_SIZE, SIZE_OF_CACHE, ASSOCIATIVITY, true);
            }
            time_stamp++;
        }

        if (ASSOCIATIVITY == 1)
        {
            if (get<0>(cache_check_direct_mapped) == true)
            {

                vector<string> parse_address_vector = parse_address(fetch_to, BLOCK_SIZE, SIZE_OF_CACHE);
                string block_offset = parse_address_vector[2];
                ll block_offset_decimal = binary_to_int(block_offset);
                string copy_of_index = parse_address_vector[1];
                ll zeros_to_add = 4 - copy_of_index.size() % 4;
                while (zeros_to_add--)
                {
                    copy_of_index = '0' + copy_of_index;
                }
                string copy_of_tag = parse_address_vector[0];
                zeros_to_add = 4 - copy_of_tag.size() % 4;
                while (zeros_to_add--)
                {
                    copy_of_tag = '0' + copy_of_tag;
                }
                if (block_offset_decimal + number_of_bytes_to_store > BLOCK_SIZE)
                {
                    cout << "ERROR" << endl; // something to do
                    exit(0);
                }
                else
                {

                    string hex_str = binary_to_hexa(int_to_binary_advanced(value, 64));
                    hex_str = hex_str.substr(16 - (number_of_bytes_to_store * 2));
                    reverse(hex_str.begin(), hex_str.end());
                    ll address = stoll(registers[stoll(rs1.substr(1))].second);
                    ll fetch_to = address + stoll(offset);
                    for (int i = 0; i < number_of_bytes_to_store * 2; i += 2)
                    {
                        string res = hex_str.substr(i, 2);
                        reverse(res.begin(), res.end());
                        get<DATA>(direct_mapped[parse_address_vector[1]])[block_offset_decimal] = res;
                        block_offset_decimal++;
                    }
                    // cache_write complete
                }
                get<DIRTY>(direct_mapped[parse_address_vector[1]]) = "1"; // Block has become Dirty

                logfile << "W: " << "Address: " << "0x" + binary_to_hexa_advanced(address_in_binary, 20) + "," << " Set: " << "0x" + reduce_excessive_zeros(binary_to_hexa_advanced(copy_of_index, copy_of_index.size())) + "," << "Hit, " << "Tag: " << "0x" + reduce_excessive_zeros(binary_to_hexa_advanced(copy_of_tag, copy_of_tag.size())) + "," << " Dirty" << endl;
            }

            if (WRITEBACK_POLICY == "WT")
            {
                vector<string> parse_address_vector = parse_address(fetch_to, BLOCK_SIZE, SIZE_OF_CACHE);

                // logfile<<"W: "<<"Address: "<<binary_to_hexa_advanced(address_in_binary,20)<<"Set:"<<binary_to_hexa_advanced(parse_address_vector[1],parse_address_vector[1].size())<<"Hit, "<<"Tag: "<<binary_to_hexa_advanced(parse_address_vector[0],parse_address_vector[0].size())<<"clean"<<endl;
                string hex_str = binary_to_hexa(int_to_binary_advanced(value, 64));
                hex_str = hex_str.substr(16 - (number_of_bytes_to_store * 2));

                reverse(hex_str.begin(), hex_str.end());

                for (int i = 0; i < number_of_bytes_to_store * 2; i += 2)
                {
                    string res = hex_str.substr(i, 2);
                    reverse(res.begin(), res.end());
                    memory[fetch_to++] = res;
                }
            }
            else
            {
                if (get<0>(cache_check_direct_mapped) == false)
                {

                    tuple<string, string, string, vector<string>> evicted_block = get<2>(cache_check_direct_mapped);
                    vector<string> address_to_safe_to = parse_address(fetch_to, BLOCK_SIZE, SIZE_OF_CACHE);
                    // logfile << "W: " << "Address: " <<"0x"+ binary_to_hexa_advanced(address_in_binary, 20)+"," << "Set:" <<"0x"+ binary_to_hexa_advanced(address_to_safe_to[1], address_to_safe_to[1].size())+"," << " Miss, " << "Tag: " <<"0x"+ binary_to_hexa_advanced(address_to_safe_to[0], address_to_safe_to[0].size())+"," << " Clean" << endl;
                    // check if the dirty bit is 0;
                    string INDEX = address_to_safe_to[1];
                    ll size_of_offset = address_to_safe_to[2].size();
                    string OFFSET = "";
                    for (ll i = 0; i < size_of_offset; i++)
                    {
                        OFFSET += '0';
                    }
                    if (get<DIRTY>(evicted_block) == "1")
                    {
                        write_backs++;
                        string address_in_binary_evicted_block = get<TAG>(evicted_block) + INDEX + OFFSET;
                        ll address_in_decimal_to_change = binary_to_int(address_in_binary_evicted_block);
                        ll number_of_bytes_to_update = BLOCK_SIZE;
                        ll i = 0;
                        while (number_of_bytes_to_update--)
                        {
                            memory[address_in_decimal_to_change++] = get<DATA>(evicted_block)[i];
                            i++;
                        }
                    }

                    //////////////////
                    string offset = address_to_safe_to[2];
                    ll starting_byte_to_store = binary_to_int(offset);

                    string hex_str = binary_to_hexa(int_to_binary_advanced(value, 64));
                    hex_str = hex_str.substr(16 - (number_of_bytes_to_store * 2));

                    reverse(hex_str.begin(), hex_str.end());

                    for (int i = 0; i < number_of_bytes_to_store * 2; i += 2)
                    {
                        string res = hex_str.substr(i, 2);
                        reverse(res.begin(), res.end());

                        get<DATA>(direct_mapped[address_to_safe_to[1]])[starting_byte_to_store++] = res;
                    }
                    get<DIRTY>(direct_mapped[address_to_safe_to[1]]) = "1";
                }
            }

            if (get<0>(cache_check_direct_mapped) == false)

            {
                vector<string> parse_address_vector = parse_address(fetch_to, BLOCK_SIZE, SIZE_OF_CACHE);
                string copy_of_index = parse_address_vector[1];
                ll zeros_to_add = 4 - copy_of_index.size() % 4;
                while (zeros_to_add--)
                {
                    copy_of_index = '0' + copy_of_index;
                }
                string copy_of_tag = parse_address_vector[0];
                zeros_to_add = 4 - copy_of_tag.size() % 4;
                while (zeros_to_add--)
                {
                    copy_of_tag = '0' + copy_of_tag;
                }
                if (get<DIRTY>(direct_mapped[parse_address_vector[1]]) == "0")
                {
                    logfile << "W: " << "Address: " << "0x" + binary_to_hexa_advanced(address_in_binary, 20) + "," << " Set: " << "0x" + reduce_excessive_zeros(binary_to_hexa_advanced(copy_of_index, copy_of_index.size())) + "," << " Miss, " << "Tag: " << "0x" + reduce_excessive_zeros(binary_to_hexa_advanced(copy_of_tag, copy_of_tag.size())) + "," << " Clean" << endl;
                }
                else
                {
                    logfile << "W: " << "Address: " << "0x" + binary_to_hexa_advanced(address_in_binary, 20) + "," << " Set: " << "0x" + reduce_excessive_zeros(binary_to_hexa_advanced(copy_of_index, copy_of_index.size())) + "," << " Miss, " << "Tag: " << "0x" + reduce_excessive_zeros(binary_to_hexa_advanced(copy_of_tag, copy_of_tag.size())) + "," << " Dirty" << endl;
                }
            }
        }
        else if (ASSOCIATIVITY == 0)
        {
            vector<string> parse_address_vector = parse_address(fetch_to, BLOCK_SIZE, SIZE_OF_CACHE);
            string copy_of_tag_index = parse_address_vector[0] + parse_address_vector[1];
            ll zeros_to_add = 4 - copy_of_tag_index.size() % 4;
            while (zeros_to_add--)
            {
                copy_of_tag_index = '0' + copy_of_tag_index;
            }
            if (get<0>(cache_check_fully_associative))
            {
                // Parsing
                string address_in_binary = int_to_binary_advanced(fetch_to, 20);
                ll block_offset_bits = log2((float)BLOCK_SIZE); // block offset bits
                ll tag_bits = address_bits - block_offset_bits;
                string extracted_tag = address_in_binary.substr(0, tag_bits);
                string extracted_offset = address_in_binary.substr(tag_bits, block_offset_bits);
                vector<string> parse_address_vector = parse_address(fetch_to, BLOCK_SIZE, SIZE_OF_CACHE);
                ll number_of_bytes_to_write = 8;
                ll block_offset_decimal = binary_to_int(extracted_offset);
                string copy_of_tag_index = parse_address_vector[0] + parse_address_vector[1];
                ll zeros_to_add = 4 - copy_of_tag_index.size() % 4;
                while (zeros_to_add--)
                {
                    copy_of_tag_index = '0' + copy_of_tag_index;
                }

                if (block_offset_decimal + number_of_bytes_to_store > BLOCK_SIZE)
                {
                    cout << "ERROR" << endl;
                    exit(0);
                }
                else
                {
                    string hex_str = binary_to_hexa(int_to_binary_advanced(value, 64));
                    hex_str = hex_str.substr(16 - (number_of_bytes_to_store * 2));
                    // logfile<<"W: "<<"Address: "<<binary_to_hexa_advanced(address_in_binary,20)<<"Set:"<<binary_to_hexa_advanced(parse_address_vector[1],parse_address_vector[1].size())<<"Hit, "<<"Tag: "<<binary_to_hexa_advanced(parse_address_vector[0],parse_address_vector[0].size())<<endl;

                    reverse(hex_str.begin(), hex_str.end());
                    ll address = stoll(registers[stoll(rs1.substr(1))].second);
                    ll fetch_to = address + stoll(offset);
                    // logfile << "W: " << "Address: " << binary_to_hexa_advanced(address_in_binary, 20) + binary_to_hexa_advanced(parse_address_vector[1], parse_address_vector[1].size()) << "Hit, " << "Tag: " << binary_to_hexa_advanced(parse_address_vector[0], parse_address_vector[0].size()) << endl;
                    for (int i = 0; i < number_of_bytes_to_store * 2; i += 2)
                    {
                        string res = hex_str.substr(i, 2);
                        reverse(res.begin(), res.end());
                        get<DATA>(fully_associative[get<2>(cache_check_fully_associative)])[block_offset_decimal] = res;
                        block_offset_decimal++;
                    }
                    // cache_write complete
                }
                get<DIRTY>(fully_associative[get<2>(cache_check_fully_associative)]) = "1"; // Block has become Dirty
                logfile << "W: " << "Address: " << "0x" + binary_to_hexa_advanced(address_in_binary, 20) + "," <<" Set: " << "0x0"<< " Hit, " << "Tag: " << "0x" + reduce_excessive_zeros(binary_to_hexa_advanced(copy_of_tag_index, copy_of_tag_index.size())) + "," << " Dirty" << endl;
            }

            if (WRITEBACK_POLICY == "WT")
            {
                string hex_str = binary_to_hexa(int_to_binary_advanced(value, 64));
                hex_str = hex_str.substr(16 - (number_of_bytes_to_store * 2));

                reverse(hex_str.begin(), hex_str.end());

                for (int i = 0; i < number_of_bytes_to_store * 2; i += 2)
                {
                    string res = hex_str.substr(i, 2);
                    reverse(res.begin(), res.end());
                    memory[fetch_to++] = res;
                }

                if (get<0>(cache_check_fully_associative) == false)
                {

                    logfile << "W: " << "Address: " << "0x" + binary_to_hexa_advanced(address_in_binary, 20) + "," <<" Set: " << "0x0"<< " Miss, " << "Tag: " << "0x" + reduce_excessive_zeros(binary_to_hexa_advanced(copy_of_tag_index, copy_of_tag_index.size())) + "," << " Clean" << endl;
                }
            }
            else
            {
                if (get<0>(cache_check_fully_associative) == false)
                {
                    tuple<string, string, string, vector<string>, int> evicted_block = get<3>(cache_check_fully_associative);
                    vector<string> address_to_safe_to = parse_address(fetch_to, BLOCK_SIZE, SIZE_OF_CACHE);
                    // logfile << "W: " << "Address: " << binary_to_hexa_advanced(address_in_binary, 20) + binary_to_hexa_advanced(address_to_safe_to[1], address_to_safe_to[1].size()) << "Miss, " << "Tag: " << binary_to_hexa_advanced(address_to_safe_to[0], address_to_safe_to[0].size()) << "dirty" << endl;
                    ll block_offset = log2((float)BLOCK_SIZE); // block offset bits

                    string OFFSET = "";
                    for (ll i = 0; i < block_offset; i++)
                    {
                        OFFSET += '0';
                    }

                    if (get<DIRTY>(evicted_block) == "1")
                    {
                        write_backs++;
                        string address_in_binary_evicted_block = get<TAG>(evicted_block) + OFFSET;
                        ll address_in_decimal_to_change = binary_to_int(address_in_binary_evicted_block);
                        ll number_of_bytes_to_update = BLOCK_SIZE;
                        ll i = 0;
                        while (number_of_bytes_to_update--)
                        {
                            memory[address_in_decimal_to_change++] = get<DATA>(evicted_block)[i];
                            i++;
                        }
                    }

                    //////////////////////////
                    // caceh right_logic start;
                    ll index_to_wrtie = -1;
                    for (ll i = 0; i < fully_associative.size(); i++)
                    {
                        if (get<VALID>(fully_associative[i]) == "1")
                        {
                            if (get<TAG>(fully_associative[i]) == address_to_safe_to[0]+address_to_safe_to[1])
                            {
                                index_to_wrtie = i;
                                break;
                            }
                        }
                    }
                    //////////////////
                    string offset = address_to_safe_to[2];
                    ll starting_byte_to_store = binary_to_int(offset);

                    string hex_str = binary_to_hexa(int_to_binary_advanced(value, 64));
                    hex_str = hex_str.substr(16 - (number_of_bytes_to_store * 2));

                    reverse(hex_str.begin(), hex_str.end());

                    for (int i = 0; i < number_of_bytes_to_store * 2; i += 2)
                    {
                        string res = hex_str.substr(i, 2);
                        reverse(res.begin(), res.end());
                        get<DATA>(fully_associative[index_to_wrtie])[starting_byte_to_store++] = res;
                    }
                    get<DIRTY>(fully_associative[index_to_wrtie]) = "1";

                    logfile << "W: " << "Address: " << "0x" + binary_to_hexa_advanced(address_in_binary, 20) + "," <<" Set: " << "0x0"<< " Miss, " << "Tag: " << "0x" + reduce_excessive_zeros(binary_to_hexa_advanced(copy_of_tag_index, copy_of_tag_index.size())) + "," << " Dirty" << endl;
                }
            }
        }
        else
        {
            vector<string> parse_address_vector = parse_address(fetch_to, BLOCK_SIZE, SIZE_OF_CACHE / ASSOCIATIVITY);
            string copy_of_index = parse_address_vector[1];
            ll zeros_to_add = 4 - copy_of_index.size() % 4;
            while (zeros_to_add--)
            {
                copy_of_index = '0' + copy_of_index;
            }
            string copy_of_tag = parse_address_vector[0];
            zeros_to_add = 4 - copy_of_tag.size() % 4;
            while (zeros_to_add--)
            {
                copy_of_tag = '0' + copy_of_tag;
            }
            if (get<0>(cache_check_set_associative))
            {
                // logfile << "Writing into the cache the new value " << endl;
                vector<string> parse_address_vector = parse_address(fetch_to, BLOCK_SIZE, SIZE_OF_CACHE / ASSOCIATIVITY);

                string extracted_tag = parse_address_vector[0];
                string extracted_index = parse_address_vector[1];
                string extracted_offset = parse_address_vector[2];

                ll number_of_bytes_to_write = 8;
                ll block_offset_decimal = binary_to_int(extracted_offset);
                if (block_offset_decimal + number_of_bytes_to_store > BLOCK_SIZE)
                {
                    cout << "ERROR" << endl;
                    exit(0);
                }
                else
                {
                    string hex_str = binary_to_hexa(int_to_binary_advanced(value, 64));
                    hex_str = hex_str.substr(16 - (number_of_bytes_to_store * 2));
                    // logfile << "W: " << "Address: " << binary_to_hexa_advanced(address_in_binary, 20) << "Set:" << binary_to_hexa_advanced(parse_address_vector[1], parse_address_vector[1].size()) << "Hit, " << "Tag: " << binary_to_hexa_advanced(parse_address_vector[0], parse_address_vector[0].size()) << endl;
                    reverse(hex_str.begin(), hex_str.end());
                    ll address = stoll(registers[stoll(rs1.substr(1))].second);

                    for (int i = 0; i < number_of_bytes_to_store * 2; i += 2)
                    {
                        string res = hex_str.substr(i, 2);
                        reverse(res.begin(), res.end());
                        get<DATA>(set_associative[extracted_index][get<2>(cache_check_set_associative)])[block_offset_decimal] = res;
                        block_offset_decimal++;
                    }
                    // cache_write complete
                }
                if(WRITEBACK_POLICY=="WT")
                {//this line is added by me
                logfile << "W: " << "Address: " << "0x" + binary_to_hexa_advanced(address_in_binary, 20) + "," << " Set: " << "0x" + reduce_excessive_zeros(binary_to_hexa_advanced(copy_of_index, copy_of_index.size())) + "," << " Hit, " << "Tag: " << "0x" + reduce_excessive_zeros(binary_to_hexa_advanced(copy_of_tag, copy_of_tag.size())) + "," << " Clean" << endl;
                }else{

                get<DIRTY>(set_associative[extracted_index][get<2>(cache_check_set_associative)]) = "1"; // Block has become Dirty
                logfile << "W: " << "Address: " << "0x" + binary_to_hexa_advanced(address_in_binary, 20) + "," << " Set: " << "0x" + reduce_excessive_zeros(binary_to_hexa_advanced(copy_of_index, copy_of_index.size())) + "," << " Hit, " << "Tag: " << "0x" + reduce_excessive_zeros(binary_to_hexa_advanced(copy_of_tag, copy_of_tag.size())) + "," << " Dirty" << endl;
                }
            }

            if (WRITEBACK_POLICY == "WT")
            {
                // logfile << "Writing into the memory" << endl;
                string hex_str = binary_to_hexa(int_to_binary_advanced(value, 64));
                hex_str = hex_str.substr(16 - (number_of_bytes_to_store * 2));

                reverse(hex_str.begin(), hex_str.end());

                for (int i = 0; i < number_of_bytes_to_store; i += 2)
                {

                    string res = hex_str.substr(i, 2);
                    reverse(res.begin(), res.end());
                    memory[fetch_to++] = res;
                }
                if (get<0>(cache_check_set_associative) == false)
                {
                    logfile << "W: " << "Address: " << "0x" + binary_to_hexa_advanced(address_in_binary, 20) + "," << " Set: " << "0x" + reduce_excessive_zeros(binary_to_hexa_advanced(copy_of_index, copy_of_index.size())) + "," << " Miss, " << "Tag: " << "0x" + reduce_excessive_zeros(binary_to_hexa_advanced(copy_of_tag, copy_of_tag.size())) + "," << " Clean" << endl;
                }
            }
            else
            {
                if (get<0>(cache_check_set_associative) == false)
                {
                    tuple<string, string, string, vector<string>, int> evicted_block = get<3>(cache_check_set_associative);
                    vector<string> address_to_safe_to = parse_address(fetch_to, BLOCK_SIZE, SIZE_OF_CACHE / ASSOCIATIVITY);
                    string INDEX = address_to_safe_to[1];
                    // logfile << "W: " << "Address: " << binary_to_hexa_advanced(address_in_binary, 20) << "Set:" << binary_to_hexa_advanced(address_to_safe_to[1], address_to_safe_to[1].size()) << "Miss, " << "Tag: " << binary_to_hexa_advanced(address_to_safe_to[0], address_to_safe_to[0].size()) << "dirty" << endl;
                    ll size_of_offset = address_to_safe_to[2].size();
                    string OFFSET = "";
                    for (ll i = 0; i < size_of_offset; i++)
                    {
                        OFFSET += '0';
                    }
                    if (get<DIRTY>(evicted_block) == "1")
                    {
                        write_backs++;
                        string address_in_binary_evicted_block = get<TAG>(evicted_block) + INDEX + OFFSET;
                        ll address_in_decimal_to_change = binary_to_int(address_in_binary_evicted_block);
                        ll number_of_bytes_to_update = BLOCK_SIZE;
                        ll i = 0;
                        while (number_of_bytes_to_update--)
                        {
                            memory[address_in_decimal_to_change++] = get<DATA>(evicted_block)[i];
                            i++;
                        }
                    }

                    //////////writing to cqache
                    vector<tuple<string, string, string, vector<string>, int>> &to_check = set_associative[address_to_safe_to[1]];
                    ll index_to_wrtie = -1;
                    for (ll i = 0; i < to_check.size(); i++)
                    {
                        if (get<VALID>(to_check[i]) == "1")
                        {
                            if (get<TAG>(to_check[i]) == address_to_safe_to[0])
                            {
                                index_to_wrtie = i;
                                break;
                            }
                        }
                    }
                    //////////////////
                    string offset = address_to_safe_to[2];
                    ll starting_byte_to_store = binary_to_int(offset);

                    string hex_str = binary_to_hexa(int_to_binary_advanced(value, 64));
                    hex_str = hex_str.substr(16 - (number_of_bytes_to_store * 2));

                    reverse(hex_str.begin(), hex_str.end());

                    for (int i = 0; i < number_of_bytes_to_store * 2; i += 2)
                    {
                        string res = hex_str.substr(i, 2);
                        reverse(res.begin(), res.end());

                        get<DATA>(to_check[index_to_wrtie])[starting_byte_to_store++] = res;
                    }
                    // if(WRITEBACK_POLICY!="WT")
                    get<DIRTY>(to_check[index_to_wrtie]) = "1";

                    logfile << "W: " << "Address: " << "0x" + binary_to_hexa_advanced(address_in_binary, 20) + "," << " Set: " << "0x" + reduce_excessive_zeros(binary_to_hexa_advanced(copy_of_index, copy_of_index.size())) + "," << " Miss, " << "Tag: " << "0x" + reduce_excessive_zeros(binary_to_hexa_advanced(copy_of_tag, copy_of_tag.size())) + "," << " Dirty" << endl;
                }
            }
        }
    }
    else
    {
        string hex_str = binary_to_hexa(int_to_binary_advanced(value, 64));
        hex_str = hex_str.substr(16 - (number_of_bytes_to_store * 2));

        reverse(hex_str.begin(), hex_str.end());

        for (int i = 0; i < number_of_bytes_to_store; i += 2)
        {

            string res = hex_str.substr(i, 2);
            reverse(res.begin(), res.end());
            memory[fetch_to++] = res;
        }
    }
}

void handleS(vector<string> v)
{

    if (v[0] == "sd")
    {
        handleS_with_size(v, 8);
    }
    if (v[0] == "sw")
    {
        handleS_with_size(v, 4);
    }
    if (v[0] == "sh")
    {
        handleS_with_size(v, 2);
    }
    if (v[0] == "sb")
    {
        handleS_with_size(v, 1);
    }
}

// function to handle I type Instructions(load ones)
void handleI_load_with_size(vector<string> v, int number_of_bytes_to_load, bool is_signed)
{

    string rd, rs, offset;
    rd = v[1];
    size_t pos1 = v[2].find('(');
    size_t pos2 = v[2].find(')');
    offset = v[2].substr(0, pos1);
    offset = trimSpaces(offset);
    string rs1 = v[2].substr(pos1 + 1, pos2 - pos1 - 1);
    rs1 = trimSpaces(rs1);
    if (register_map.count(rd))
    {
        rd = register_map[rd];
    }
    if (register_map.count(rs1))
    {
        rs1 = register_map[rs1];
    }

    ll address = stoll(registers[stoll(rs1.substr(1))].second);
    ll index_from_fetch = address + stoll(offset);

    if (cache_enable_status)
    {
        // logfile<<"Loading from cache"<< endl;
        // cache_logs<<"R: "<<"Address: "<< binary_to_hexa(int_to_binary_advanced(index_from_fetch,20))<<", ";
        ll no_of_sets;
        // cache bringing logic
        if (ASSOCIATIVITY != 0)
        {
            no_of_sets = (SIZE_OF_CACHE / (float)BLOCK_SIZE) / ASSOCIATIVITY;
        }
        // result for each type of cache
        tuple<bool, vector<string>, tuple<string, string, string, vector<string>>> direct_mapped_result;
        tuple<bool, vector<string>, int, tuple<string, string, string, vector<string>, int>> fully_associative_result;
        tuple<bool, vector<string>, int, tuple<string, string, string, vector<string>, int>> set_associative_result;

        // cout << "Loading " << endl;
        // cout << "No of sets" << no_of_sets << endl;

        if (ASSOCIATIVITY == 1)
        {
            //   cout << "Loaded from Directly Mapped cached Block" << endl;
            direct_mapped_result = direct_mapped_cache_initialize(index_from_fetch, BLOCK_SIZE, SIZE_OF_CACHE, true);
        }
        else if (ASSOCIATIVITY == 0)
        {
            // cout << "Loaded from Fully Associative cached Block" << endl;
            fully_associative_result = fully_associative_initialize(index_from_fetch, BLOCK_SIZE, SIZE_OF_CACHE, true);
            time_stamp++;
        }
        else
        {
            // cout << "loaded from set associative" << endl;
            set_associative_result = set_associative_initialise(index_from_fetch, BLOCK_SIZE, SIZE_OF_CACHE, ASSOCIATIVITY, true);
            time_stamp++;
        }

        /////////////////////

        string hex_str = "";

        // // string address_from_which_we_will_found_offset=int_to_binary_advanced(index_from_fetch,20);
        // vector<string> extracted_info = parse_address(index_from_fetch, BLOCK_SIZE, SIZE_OF_CACHE / ASSOCIATIVITY);
        // string extracted_index_of = extracted_info[1];
        // string offset_in = extracted_info[2]; // ye offset fully associative ke liye bhi kaam karega
        // ll offset_in_decimal = binary_to_int(offset_in);
        // string to_write="Hit";
        string address_in_binary_to_generate_log_file = int_to_binary_advanced(index_from_fetch, 20);

        if (ASSOCIATIVITY == 1)
        {
            // string address_from_which_we_will_found_offset=int_to_binary_advanced(index_from_fetch,20);
            vector<string> extracted_info = parse_address(index_from_fetch, BLOCK_SIZE, SIZE_OF_CACHE / ASSOCIATIVITY);
            string extracted_index_of = extracted_info[1];
            string offset_in = extracted_info[2]; // ye offset fully associative ke liye bhi kaam karega
            ll offset_in_decimal = binary_to_int(offset_in);
            if (offset_in_decimal + number_of_bytes_to_load > BLOCK_SIZE)
            {
                cout << "ERROR" << endl; // something to do
                exit(0);
            }
            if (get<0>(direct_mapped_result) == true)
            {
                vector<string> data = get<1>(direct_mapped_result);

                for (int i = 0; i < number_of_bytes_to_load; i++)
                {
                    hex_str = data[offset_in_decimal] + hex_str;
                    offset_in_decimal++;
                }

                ll value;
                if (is_signed)
                {
                    ull converted_number = stoull(hex_str, NULL, 16); // this is giving what error
                    uint64_t sub_factor = static_cast<uint64_t>(pow(2, 64) - 1);

                    if (hex_str[0] >= '8')
                    {
                        value = converted_number - sub_factor - 1;
                    }
                    else
                    {
                        value = converted_number;
                    }
                }
                else
                {
                    value = stoll(hex_str, NULL, 16);
                }

                registers[stoll(rd.substr(1))].second = to_string(value);

                string copy_of_index = extracted_info[1];
                ll zeros_to_add = 4 - copy_of_index.size() % 4;
                while (zeros_to_add--)
                {
                    copy_of_index = '0' + copy_of_index;
                }
                string copy_of_tag = extracted_info[0];
                zeros_to_add = 4 - copy_of_tag.size() % 4;
                while (zeros_to_add--)
                {
                    copy_of_tag = '0' + copy_of_tag;
                }
                if (get<DIRTY>(direct_mapped[extracted_info[1]]) == "0")
                {

                    logfile << "R: " << "Address: " << "0x" + binary_to_hexa_advanced(address_in_binary_to_generate_log_file, 20) + "," << " Set: " << "0x" + reduce_excessive_zeros(binary_to_hexa_advanced(copy_of_index, copy_of_index.size())) + "," << " Hit, " << "Tag: " << "0x" + reduce_excessive_zeros(binary_to_hexa_advanced(copy_of_tag, copy_of_tag.size())) + "," << " Clean" << endl;
                }
                else
                {
                    logfile << "R: " << "Address: " << "0x" + binary_to_hexa_advanced(address_in_binary_to_generate_log_file, 20) + "," << " Set: " << "0x" + reduce_excessive_zeros(binary_to_hexa_advanced(copy_of_index, copy_of_index.size())) + "," << " Hit, " << "Tag: " << "0x" + reduce_excessive_zeros(binary_to_hexa_advanced(copy_of_tag, copy_of_tag.size())) + "," << " Dirty" << endl;
                }

                // yha pe just return kar dena hai
                return;
            }

            string copy_of_index = extracted_info[1];
            ll zeros_to_add = 4 - copy_of_index.size() % 4;
            while (zeros_to_add--)
            {
                copy_of_index = '0' + copy_of_index;
            }
            string copy_of_tag = extracted_info[0];
            zeros_to_add = 4 - copy_of_tag.size() % 4;
            while (zeros_to_add--)
            {
                copy_of_tag = '0' + copy_of_tag;
            }
            if (get<DIRTY>(direct_mapped[extracted_info[1]]) == "0")
            {

                logfile << "R: " << "Address: " << "0x" + binary_to_hexa_advanced(address_in_binary_to_generate_log_file, 20) + "," << " Set: " << "0x" + reduce_excessive_zeros(binary_to_hexa_advanced(copy_of_index, copy_of_index.size())) + "," << " Miss, " << "Tag: " << "0x" + reduce_excessive_zeros(binary_to_hexa_advanced(copy_of_tag, copy_of_tag.size())) + "," << " Clean" << endl;
            }
            else
            {
                logfile << "R: " << "Address: " << "0x" + binary_to_hexa_advanced(address_in_binary_to_generate_log_file, 20) + "," << " Set: " << "0x" + reduce_excessive_zeros(binary_to_hexa_advanced(copy_of_index, copy_of_index.size())) + "," << " Miss, " << "Tag: " << "0x" + reduce_excessive_zeros(binary_to_hexa_advanced(copy_of_tag, copy_of_tag.size())) + "," << " Dirty" << endl;
            }

            tuple<string, string, string, vector<string>> evicted_block = get<2>(direct_mapped_result);
            vector<string> address_to_safe_to = parse_address(index_from_fetch, BLOCK_SIZE, SIZE_OF_CACHE);
            // check if the dirty bit is 0;
            string INDEX = address_to_safe_to[1];
            ll size_of_offset = address_to_safe_to[2].size();
            string OFFSET = "";
            for (ll i = 0; i < size_of_offset; i++)
            {
                OFFSET += '0';
            }
            if (get<DIRTY>(evicted_block) == "1")
            {
                write_backs++;
                string address_in_binary_evicted_block = get<TAG>(evicted_block) + INDEX + OFFSET;
                ll address_in_decimal_to_change = binary_to_int(address_in_binary_evicted_block);
                ll number_of_bytes_to_update = BLOCK_SIZE;
                ll i = 0;
                while (number_of_bytes_to_update--)
                {
                    memory[address_in_decimal_to_change++] = get<DATA>(evicted_block)[i];
                    i++;
                }
            }

            ////////////////
        }
        else if (ASSOCIATIVITY == 0)
        { // string address_from_which_we_will_found_offset=int_to_binary_advanced(index_from_fetch,20);
            vector<string> extracted_info = parse_address(index_from_fetch, BLOCK_SIZE, SIZE_OF_CACHE);
            string extracted_index_of = extracted_info[1];
            string offset_in = extracted_info[2]; // ye offset fully associative ke liye bhi kaam karega
            ll offset_in_decimal = binary_to_int(offset_in);
            if (offset_in_decimal + number_of_bytes_to_load > BLOCK_SIZE)
            {
                cout << "ERROR" << endl; // something to do
                exit(0);
            }
            if (get<0>(fully_associative_result) == true)
            {
                vector<string> data = get<1>(fully_associative_result);

                for (int i = 0; i < number_of_bytes_to_load; i++)
                {
                    hex_str = data[offset_in_decimal] + hex_str;
                    offset_in_decimal++;
                }
                ll value;
                if (is_signed)
                {
                    ull converted_number = stoull(hex_str, NULL, 16); // this is giving what error
                    uint64_t sub_factor = static_cast<uint64_t>(pow(2, 64) - 1);

                    if (hex_str[0] >= '8')
                    {
                        value = converted_number - sub_factor - 1;
                    }
                    else
                    {
                        value = converted_number;
                    }
                }
                else
                {
                    value = stoll(hex_str, NULL, 16);
                }
                // i need to add zeros
                registers[stoll(rd.substr(1))].second = to_string(value);
                string copy_of_tag_index = extracted_info[0] + extracted_info[1];
                ll zeros_to_add = 4 - copy_of_tag_index.size() % 4;
                while (zeros_to_add--)
                {
                    copy_of_tag_index = '0' + copy_of_tag_index;
                }
                string dirty_bit = "1";
                if (get<2>(fully_associative_result) == -1)
                {
                    dirty_bit = "0";
                }
                else
                {
                    dirty_bit = get<DIRTY>(fully_associative[get<2>(fully_associative_result)]);
                }
                if (dirty_bit == "0")
                {

                    logfile << "R: " << "Address: " << "0x" + binary_to_hexa_advanced(address_in_binary_to_generate_log_file, 20) + "," <<" Set: " << "0x0"<< " Hit, " << "Tag: " << "0x" + reduce_excessive_zeros(binary_to_hexa_advanced(copy_of_tag_index, copy_of_tag_index.size())) + "," << " Clean" << endl;
                }
                else
                {

                    logfile << "R: " << "Address: " << "0x" + binary_to_hexa_advanced(address_in_binary_to_generate_log_file, 20) + "," <<" Set: " << "0x0"<< " Hit, " << "Tag: " << "0x" + reduce_excessive_zeros(binary_to_hexa_advanced(copy_of_tag_index, copy_of_tag_index.size())) + "," << " Dirty" << endl;
                }
                // yha pe just return kar dena hai
                return;
            }
            string copy_of_tag_index = extracted_info[0] + extracted_info[1];
            ll zeros_to_add = 4 - copy_of_tag_index.size() % 4;
            while (zeros_to_add--)
            {
                copy_of_tag_index = '0' + copy_of_tag_index;
            }

            string dirty_bit = "1";
            if (get<2>(fully_associative_result) == -1)
            {
                dirty_bit = "0";
            }
            else
            {
                dirty_bit = get<DIRTY>(fully_associative[get<2>(fully_associative_result)]);
            }
            if (dirty_bit == "0")
            {

                logfile << "R: " << "Address: " << "0x" + binary_to_hexa_advanced(address_in_binary_to_generate_log_file, 20) + "," <<" Set: " << "0x0"<< " Miss, " << "Tag: " << "0x" + reduce_excessive_zeros(binary_to_hexa_advanced(copy_of_tag_index, copy_of_tag_index.size())) + "," << " Clean" << endl;
            }
            else
            {

                logfile << "R: " << "Address: " << "0x" + binary_to_hexa_advanced(address_in_binary_to_generate_log_file, 20) + "," << " Set: " << "0x0"<<" Miss, " << "Tag: " << "0x" + reduce_excessive_zeros(binary_to_hexa_advanced(copy_of_tag_index, copy_of_tag_index.size())) + "," << " Dirty" << endl;
            }

            // handling evicted block!!

            tuple<string, string, string, vector<string>, int> evicted_block = get<3>(fully_associative_result);
            vector<string> address_to_safe_to = parse_address(index_from_fetch, BLOCK_SIZE, SIZE_OF_CACHE);
            // logfile << "W: " << "Address: " << binary_to_hexa_advanced(address_in_binary, 20) + binary_to_hexa_advanced(address_to_safe_to[1], address_to_safe_to[1].size()) << "Miss, " << "Tag: " << binary_to_hexa_advanced(address_to_safe_to[0], address_to_safe_to[0].size()) << "dirty" << endl;
            ll block_offset = log2((float)BLOCK_SIZE); // block offset bits

            string OFFSET = "";
            for (ll i = 0; i < block_offset; i++)
            {
                OFFSET += '0';
            }

            if (get<DIRTY>(evicted_block) == "1")
            {
                write_backs++;
                string address_in_binary_evicted_block = get<TAG>(evicted_block) + OFFSET;
                ll address_in_decimal_to_change = binary_to_int(address_in_binary_evicted_block);
                ll number_of_bytes_to_update = BLOCK_SIZE;
                ll i = 0;
                while (number_of_bytes_to_update--)
                {
                    memory[address_in_decimal_to_change++] = get<DATA>(evicted_block)[i];
                    i++;
                }
            }

            // logfile<<"R: "<<"Address: "<<binary_to_hexa_advanced(address_in_binary_to_generate_log_file,20)<<"Tag: "<<binary_to_hexa_advanced(copy_of_tag_index,copy_of_tag_index.size())<<endl;
        }
        else
        {
            // string address_from_which_we_will_found_offset=int_to_binary_advanced(index_from_fetch,20);
            vector<string> extracted_info = parse_address(index_from_fetch, BLOCK_SIZE, SIZE_OF_CACHE / ASSOCIATIVITY);
            string extracted_index_of = extracted_info[1];
            string offset_in = extracted_info[2]; // ye offset fully associative ke liye bhi kaam karega
            ll offset_in_decimal = binary_to_int(offset_in);
            if (offset_in_decimal + number_of_bytes_to_load > BLOCK_SIZE)
            {
                cout << "ERROR" << endl;
                exit(0);
            }
            if (get<0>(set_associative_result) == true)
            {
                vector<string> data = get<1>(set_associative_result);

                for (int i = 0; i < number_of_bytes_to_load; i++)
                {
                    hex_str = data[offset_in_decimal] + hex_str;
                    offset_in_decimal++;
                }
                ull converted_number = stoull(hex_str, NULL, 16); // this is giving what error
                ll value;
                if (is_signed)
                {
                    ull converted_number = stoull(hex_str, NULL, 16); // this is giving what error
                    uint64_t sub_factor = static_cast<uint64_t>(pow(2, 64) - 1);

                    if (hex_str[0] >= '8')
                    {
                        value = converted_number - sub_factor - 1;
                    }
                    else
                    {
                        value = converted_number;
                    }
                }
                else
                {
                    value = stoll(hex_str, NULL, 16);
                }
                registers[stoll(rd.substr(1))].second = to_string(value);
                string copy_of_index = extracted_info[1];
                ll zeros_to_add = 4 - copy_of_index.size() % 4;
                while (zeros_to_add--)
                {
                    copy_of_index = '0' + copy_of_index;
                }
                string copy_of_tag = extracted_info[0];
                zeros_to_add = 4 - copy_of_tag.size() % 4;
                while (zeros_to_add--)
                {
                    copy_of_tag = '0' + copy_of_tag;
                }
                ll index_to_get = get<2>(set_associative_result);
                string dirty_bit = "1";
                if (index_to_get == -1)
                {
                    dirty_bit = "0";
                }
                else
                {
                    dirty_bit = get<DIRTY>(set_associative[extracted_info[1]][index_to_get]);
                }
                if (dirty_bit == "0")
                {

                    logfile << "R: " << "Address: " << "0x" + binary_to_hexa_advanced(address_in_binary_to_generate_log_file, 20) + "," << " Set: " << "0x" + reduce_excessive_zeros(binary_to_hexa_advanced(copy_of_index, copy_of_index.size())) + "," << " Hit, " << "Tag: " << "0x" + reduce_excessive_zeros(binary_to_hexa_advanced(copy_of_tag, copy_of_tag.size())) + "," << " Clean" << endl;
                }
                else
                {
                    logfile << "R: " << "Address: " << "0x" + binary_to_hexa_advanced(address_in_binary_to_generate_log_file, 20) + "," << " Set: " << "0x" + reduce_excessive_zeros(binary_to_hexa_advanced(copy_of_index, copy_of_index.size())) + "," << " Hit, " << "Tag: " << "0x" + reduce_excessive_zeros(binary_to_hexa_advanced(copy_of_tag, copy_of_tag.size())) + "," << " Dirty" << endl;
                }
                // yha pe just return kar dena hai
                return;
            }
            string copy_of_index = extracted_info[1];
            ll zeros_to_add = 4 - copy_of_index.size() % 4;
            while (zeros_to_add--)
            {
                copy_of_index = '0' + copy_of_index;
            }
            string copy_of_tag = extracted_info[0];
            zeros_to_add = 4 - copy_of_tag.size() % 4;
            while (zeros_to_add--)
            {
                copy_of_tag = '0' + copy_of_tag;
            }
            ll index_to_get = get<2>(set_associative_result);
            string dirty_bit = "1";
            if (index_to_get == -1)
            {
                dirty_bit = "0";
            }
            else
            {
                dirty_bit = get<DIRTY>(set_associative[extracted_info[1]][index_to_get]);
            }
            if (dirty_bit == "0")
            {

                logfile << "R: " << "Address: " << "0x" + binary_to_hexa_advanced(address_in_binary_to_generate_log_file, 20) + "," << " Set: " << "0x" + reduce_excessive_zeros(binary_to_hexa_advanced(copy_of_index, copy_of_index.size())) + "," << " Miss, " << "Tag: " << "0x" + reduce_excessive_zeros(binary_to_hexa_advanced(copy_of_tag, copy_of_tag.size())) + "," << " Clean" << endl;
            }
            else
            {
                logfile << "R: " << "Address: " << "0x" + binary_to_hexa_advanced(address_in_binary_to_generate_log_file, 20) + "," << " Set: " << "0x" + reduce_excessive_zeros(binary_to_hexa_advanced(copy_of_index, copy_of_index.size())) + "," << " Miss, " << "Tag: " << "0x" + reduce_excessive_zeros(binary_to_hexa_advanced(copy_of_tag, copy_of_tag.size())) + "," << " Dirty" << endl;
            }

            // check for evicted block

            tuple<string, string, string, vector<string>, int> evicted_block = get<3>(set_associative_result);
            vector<string> address_to_safe_to = parse_address(index_from_fetch, BLOCK_SIZE, SIZE_OF_CACHE / ASSOCIATIVITY);
            string INDEX = address_to_safe_to[1];
            // logfile << "W: " << "Address: " << binary_to_hexa_advanced(address_in_binary, 20) << "Set:" << binary_to_hexa_advanced(address_to_safe_to[1], address_to_safe_to[1].size()) << "Miss, " << "Tag: " << binary_to_hexa_advanced(address_to_safe_to[0], address_to_safe_to[0].size()) << "dirty" << endl;
            ll size_of_offset = address_to_safe_to[2].size();
            string OFFSET = "";
            for (ll i = 0; i < size_of_offset; i++)
            {
                OFFSET += '0';
            }
            // logfile << get<DIRTY>(evicted_block) << endl;

            if (get<DIRTY>(evicted_block) == "1")
            {
                // logfile << "Memory Updated" << endl;
                write_backs++;
                string address_in_binary_evicted_block = get<TAG>(evicted_block) + INDEX + OFFSET;
                ll address_in_decimal_to_change = binary_to_int(address_in_binary_evicted_block);
                ll number_of_bytes_to_update = BLOCK_SIZE;
                ll i = 0;
                while (number_of_bytes_to_update--)
                {
                    memory[address_in_decimal_to_change++] = get<DATA>(evicted_block)[i];
                    i++;
                }
            }
        }

        // here i guess logic is repeated
        for (int i = 0; i < number_of_bytes_to_load; i++)
        {
            hex_str = memory[i + index_from_fetch] + hex_str;
        }
        ll value;
        if (is_signed)
        {
            if (hex_str[0] >= '8')
            {
                int z = 16 - number_of_bytes_to_load * 2;
                while (z != 0 && z--)
                {
                    hex_str = 'f' + hex_str;
                }
            }
            ull converted_number = stoull(hex_str, NULL, 16); // this is giving what error
            uint64_t sub_factor = static_cast<uint64_t>(pow(2, 64) - 1);

            if (hex_str[0] >= '8')
            {
                value = converted_number - sub_factor - 1;
            }
            else
            {
                value = converted_number;
            }
        }
        else
        {
            value = stoll(hex_str, NULL, 16);
        }

        registers[stoll(rd.substr(1))].second = to_string(value);
    }
    else
    {
        // logfile << "Loading from Memory" << endl;
        string hex_str = "";
        for (int i = 0; i < number_of_bytes_to_load; i++)
        {
            hex_str = memory[i + index_from_fetch] + hex_str;
        }
        ll value;
        if (is_signed)
        {
            if (hex_str[0] >= '8')
            {
                int z = 16 - number_of_bytes_to_load * 2;
                while (z != 0 && z--)
                {
                    hex_str = 'f' + hex_str;
                }
            }
            ull converted_number = stoull(hex_str, NULL, 16); // this is giving what error
            uint64_t sub_factor = static_cast<uint64_t>(pow(2, 64) - 1);

            if (hex_str[0] >= '8')
            {
                value = converted_number - sub_factor - 1;
            }
            else
            {
                value = converted_number;
            }
        }
        else
        {
            value = stoll(hex_str, NULL, 16);
        }

        registers[stoll(rd.substr(1))].second = to_string(value);
    }
}
void handleI_load(vector<string> v)
{

    if (v[0] == "ld")
    {
        handleI_load_with_size(v, 8, true);
    }
    if (v[0] == "lw")
    {
        handleI_load_with_size(v, 4, true);
    }
    if (v[0] == "lh")
    {
        handleI_load_with_size(v, 2, true);
    }
    if (v[0] == "lb")
    {
        handleI_load_with_size(v, 1, true);
    }
    if (v[0] == "lbu")
    {
        handleI_load_with_size(v, 1, false);
    }
    if (v[0] == "lhu")
    {
        handleI_load_with_size(v, 2, false);
    }
    if (v[0] == "lwu")
    {
        handleI_load_with_size(v, 4, false);
    }
    registers[0].second = "0";
}

// function to handle J type Instructions
void handleJ(vector<string> v, int &PC, unordered_map<string, ll> labelmap)
{
    if (register_map.count(v[1]))
    {
        v[1] = register_map[v[1]];
    }
    string rd = v[1];
    string offset = v[2];
    if (rd != "x0")
    {
        registers[stoll(rd.substr(1))].second = to_string(PC + 4);
    }

    if (isNumericString(offset))
    {
        PC = PC + stoll(offset);
    }
    else
    {
        PC = labelmap[offset] * 4;
    }
}

// function to handle B type Instructions
void handleB(vector<string> v, int &PC, unordered_map<string, ll> labelmap)

{

    if (register_map.count(v[1]))
    { // check if alternate register name is present
        v[1] = register_map[v[1]];
    }
    if (register_map.count(v[2]))
    { // check if alternate register name is present
        v[2] = register_map[v[2]];
    }
    string rs1 = v[1], rs2 = v[2], label = v[3];

    ll number1 = stoll(registers[stoll(rs1.substr(1))].second);
    ll number2 = stoll(registers[stoll(rs2.substr(1))].second);

    if (v[0] == "beq")
    {
        if (number1 == number2)
        {

            PC = labelmap[label] * 4;
        }
        else
        {
            PC += 4;
        }
    }
    else if (v[0] == "bne")
    {
        if (number1 != number2)
        {
            PC = labelmap[label] * 4;
        }
        else
        {
            PC += 4;
        }
    }
    else if (v[0] == "blt")
    {
        if (number1 < number2)
        {
            PC = labelmap[label] * 4;
        }
        else
        {
            PC += 4;
        }
    }
    else if (v[0] == "bge")
    {
        if (number1 >= number2)
        {
            PC = labelmap[label] * 4;
        }
        else
        {
            PC += 4;
        }
    }
    else
    {
        ull number1 = stoll(registers[stoll(rs1.substr(1))].second);
        ull number2 = stoll(registers[stoll(rs2.substr(1))].second);

        if (v[0] == "bltu")
        {
            if (number1 < number2)
            {
                PC = labelmap[label] * 4;
            }
            else
            {
                PC += 4;
            }
        }

        if (v[0] == "bgeu")
        {
            if (number1 >= number2)
            {
                PC = labelmap[label] * 4;
            }
            else
            {
                PC += 4;
            }
        }
    }
}

// function to remove extra spaces from the lines
void remove_extra_spaces_from_lines(vector<string> &lines)
{
    for (int i = 0; i < lines.size(); i++)
    {
        int index = -1;
        for (int j = 0; j < lines[i].size(); j++)
        {
            if (lines[i][j] != ' ' && lines[i][j] != '\t')
            {
                index = j;
                break;
            }
        }
        if (index != -1)
        {
            lines[i] = lines[i].substr(index);
        }
    }
}

// function to populate the text section of the memory
void populate_text_memory(vector<string> v)
{

    int ptr_to_text = 0;
    for (int i = 0; i < v.size(); i++)
    {
        string hex_str = v[i];
        int req = 8 - hex_str.size();
        while (req--)
        {
            hex_str = '0' + hex_str;
        }
        reverse(hex_str.begin(), hex_str.end());
        for (int i = 0; i < 8; i += 2)
        {
            string res = hex_str.substr(i, 2);
            reverse(res.begin(), res.end());
            memory[ptr_to_text++] = res;
        }
    }
}
int main()
{
    string fileName;
    srand(time(NULL));
    vector<int> break_points;
    vector<string> lines;
    unordered_map<string, ll> labelmap;
    stack<pair<string, ll>> call_stack;
    int PC = 0;
    ll text_section_starts = 0;

    while (1)
    {
        string x;
        getline(cin, x);
        x = trimSpaces(x); // trimming of spaces

        if (find_load(x))
        {
            logfile.close();
            lines.clear();
            labelmap.clear();
            PC = 0; // again set the PC to zero
            // clear the stack (clear the memory)
            stack_pointer = 0;

            break_points.clear();
            registers = {
                {"x0", "0"}, {"x1", "0"}, {"x2", "0"}, {"x3", "0"}, {"x4", "0"}, {"x5", "0"}, {"x6", "0"}, {"x7", "0"}, {"x8", "0"}, {"x9", "0"}, {"x10", "0"}, {"x11", "0"}, {"x12", "0"}, {"x13", "0"}, {"x14", "0"}, {"x15", "0"}, {"x16", "0"}, {"x17", "0"}, {"x18", "0"}, {"x19", "0"}, {"x20", "0"}, {"x21", "0"}, {"x22", "0"}, {"x23", "0"}, {"x24", "0"}, {"x25", "0"}, {"x26", "0"}, {"x27", "0"}, {"x28", "0"}, {"x29", "0"}, {"x30", "0"}, {"x31", "0"}};

            call_stack.push({"main:", 0});

            int index = find_filename(x);
            string file_name = x.substr(index + 1);
            fileName = file_name;
            fileName=fileName.substr(0,fileName.size()-2);

            fileName += ".output";
            cout<<fileName<<endl;
            logfile.open(fileName);
            if (!logfile.is_open())
            {
                cout << "file not opened" << endl;
            }
            else
            {
                cout << "file opened successfully" << endl;
            }

            cache_logs.open(file_name + ".output", ios::out);
            ifstream file;
            file.open(file_name);

            // assembler logic integrated
            //  Construct the command by concatenating the executable with the file name
            // // string command = "./assembler_executable/riscv_asm " + file_name;
            // string command = "./riscv_asm " + file_name;

            // // Use c_str() to convert the string to a C-style string (const char*)
            // int result = system(command.c_str());

            // string c;
            // ifstream output_file("output.hex");
            // vector<string> instructions_from_output;
            // bool correct_code = 1;
            // while (getline(output_file, c))
            // {
            //     if ( trimSpaces(c).substr(0,6) =="Error:" )
            //     {
            //         correct_code = 0;
            //         break;
            //     }
            //     instructions_from_output.push_back(c);
            // }
            // if (correct_code == 0)
            // {
            //     break;
            // }
            // populate_text_memory(instructions_from_output);

            int flag = 1; // indicates that whether in text segment or data
            string line;
            int memptr = 0;
            ll line_count = 0;
            while (getline(file, line))
            {
                line_count++;
                line = trimSpaces(line); // Trimming of Leading and Trailing Spaces

                if (line.size() == 0) // skipping empty lines
                    continue;

                // skipping the comment   (Can Comment the Line with "#")

                if (line[0] == '#')
                {
                    continue;
                }
                // Comment inside the line
                if (line.find('#') != string::npos)
                {
                    line = line.substr(0, line.find('#') - 1);
                    line.erase(line.find_last_not_of(' ') + 1);
                }

                if (check_data(line))
                {
                    flag = 0;

                    continue;
                }
                else if (check_text(line))
                {
                    flag = 1;
                    text_section_starts = line_count;
                    continue;
                }

                if (flag == 1)
                {

                    // Handling different line Labels
                    if (line[line.length() - 1] == ':')
                    {

                        string label = trimSpaces(line.substr(0, line.length() - 1));

                        labelmap.insert({label, size(lines)});
                        continue;
                    }
                    // handling inline lines labels
                    else if (line.find(':') != string::npos)
                    {
                        int pos = line.find(':');
                        string label = trimSpaces(line.substr(0, pos));
                        labelmap.insert({label, size(lines)});

                        line = trimSpaces(line.substr(pos + 1));
                    }

                    lines.push_back(line);
                }
                else
                {
                    store_data(line);
                }
            }

            remove_extra_spaces_from_lines(lines);

            file.close();
        }

        if (x.substr(0, 14) == "cache_sim dump")
        {

            ll index = -1;
            for (int i = 14; i < x.size(); i++)
            {
                if (x[i] != ' ')
                {
                    index = i;
                    break;
                }
            }

            string dump_file_name = x.substr(index);
            cout<<dump_file_name<<endl;
            ofstream cacheSnapShot(dump_file_name, ios::out);

            if (ASSOCIATIVITY == 1)
            {
                ll y = 0;
                string set_to_write = int_to_binary_advanced(y, ceil(log2(no_of_sets)));
                ll zeros_to_add = 4 - set_to_write.size() % 4;
                while (zeros_to_add--)
                {
                    set_to_write = '0' + set_to_write;
                }

                while (y != no_of_sets)
                {
                    string z = int_to_binary_advanced(y, ceil(log2(no_of_sets)));
                    string set_to_write = z;
                    ll zeros_to_add = 4 - set_to_write.size() % 4;
                    while (zeros_to_add--)
                    {
                        set_to_write = '0' + set_to_write;
                    }
                    string tag_to_write = get<TAG>(direct_mapped[z]);
                    zeros_to_add = 4 - tag_to_write.size() % 4;
                    while (zeros_to_add--)
                    {
                        tag_to_write = '0' + tag_to_write;
                    }
                    if (get<VALID>(direct_mapped[z]) == "1")
                    {
                        cacheSnapShot << "Set: " << "0x" << reduce_excessive_zeros(binary_to_hexa_advanced(set_to_write, set_to_write.size())) << ", ";
                        cacheSnapShot << "Tag: " << "0x" << reduce_excessive_zeros(binary_to_hexa_advanced(tag_to_write, tag_to_write.size())) << ", ";
                        if (get<DIRTY>(direct_mapped[z]) == "1")
                        {
                            cacheSnapShot << "Dirty" << endl;
                        }
                        else
                        {
                            cacheSnapShot << "Clean" << endl;
                        }
                    }
                    y++;
                }
            }
            else if (ASSOCIATIVITY == 0)
            {

                ll y = 0;
                while (y != SIZE_OF_CACHE / BLOCK_SIZE)
                {
                    string tag_to_write = get<TAG>(fully_associative[y]);
                    ll zeros_to_add = 4 - tag_to_write.size() % 4;
                    while (zeros_to_add--)
                    {
                        tag_to_write = '0' + tag_to_write;
                    }
                    if (get<VALID>(fully_associative[y]) == "1")
                    {

                        cacheSnapShot << "Tag: " << "0x" << reduce_excessive_zeros(binary_to_hexa_advanced(tag_to_write, tag_to_write.size())) << ", ";
                        if (get<DIRTY>(fully_associative[y]) == "1")
                        {
                            cacheSnapShot << "Dirty" << endl;
                        }
                        else
                        {
                            cacheSnapShot << "Clean" << endl;
                        }
                    }

                    // cacheSnapShot << "TimeStamp" << get<TIMESTAMP>(fully_associative[y]) << endl;
                    y++;
                }
            }
            else
            {
                // set associative
                ll y = 0;
                while (y != no_of_sets)
                {
                    string z = int_to_binary_advanced(y, ceil(log2(no_of_sets)));
                    string set_to_write = z;
                    ll zeros_to_add = 4 - set_to_write.size() % 4;
                    while (zeros_to_add--)
                    {
                        set_to_write = '0' + set_to_write;
                    }

                    for (int j = 0; j < ASSOCIATIVITY; j++)
                    {
                        string tag_to_write = get<TAG>(set_associative[z][j]);
                        ll zeros_to_add = 4 - tag_to_write.size() % 4;
                        while (zeros_to_add--)
                        {
                            tag_to_write = '0' + tag_to_write;
                        }
                        if (get<VALID>(set_associative[z][j]) == "1")
                        {
                            cacheSnapShot << "Set: " << "0x" << reduce_excessive_zeros(binary_to_hexa_advanced(set_to_write, set_to_write.size())) << ", ";
                            cacheSnapShot << "Tag: " << "0x" << reduce_excessive_zeros(binary_to_hexa_advanced(tag_to_write, tag_to_write.size())) << ", ";
                            if (get<DIRTY>(set_associative[z][j]) == "1")
                            {
                                cacheSnapShot << "Dirty" << endl;
                            }
                            else
                            {
                                cacheSnapShot << "Clean" << endl;
                            }
                        }

                        // cacheSnapShot << "Valid " << get<VALID>(set_associative[z][j]) << " ";
                        // cacheSnapShot << "Tag " << get<TAG>(set_associative[z][j]) << " ";
                        // cacheSnapShot << "TimeStamp" << get<TIMESTAMP>(set_associative[z][j]) << endl;
                    }

                    y++;
                }
            }
            cacheSnapShot.close();
        }
        if (x == "cache_sim stats")
        {
            cout << "D-cache statistics: Accesses=" << hits + misses << ", Hit=" << hits << ", Miss=" << misses << ", Hit Rate=" << (float)hits / (hits + misses) << endl;
        }
        if (x == "cache_sim invalidate")
        {
            hits = 0;
            misses = 0;
            write_backs = 0;
            ll no_of_blocks = SIZE_OF_CACHE / (float)BLOCK_SIZE;

            // Invalidate direct mapped cache
            if (ASSOCIATIVITY == 1)
            {
                for(auto [x,y]:direct_mapped){
                    string index=x;
                    string dirty=get<DIRTY>(y);
                    if(dirty=="1"){
                        string tag=get<TAG>(y);
                        ll num_of_bloak_offset=20-x.size()-tag.size();
                        string offset="";
                        while(num_of_bloak_offset--){
                            offset+='0';
                        }
                        ll address_to_write=binary_to_int(tag+index+offset);
                        vector<string>data=get<DATA>(y);
                        for(auto i:data){
                            memory[address_to_write++]=i;
                        }
                    }
                }
                //if there is no dirty blocks then move with no change and just delete it
                direct_mapped.clear();
            }
            // Invalidate fully assoiative cache
            else if (ASSOCIATIVITY == 0)
            {
                for(ll i=0;i<fully_associative.size();i++){
                    string dirty=get<DIRTY>(fully_associative[i]);
                    if(dirty=="1"){
                        ll num_of_bloak_offset=20-get<TAG>(fully_associative[i]).size();
                        string offset="";
                        while(num_of_bloak_offset--){
                            offset+='0';
                        }
                        ll address_to_write=binary_to_int(get<TAG>(fully_associative[i])+offset);
                        vector<string>data=get<DATA>(fully_associative[i]);
                        for(auto i:data){
                            memory[address_to_write++]=i;
                        }
                    }
                }
                index_for_entry_in_fully_associative = 0;
                fully_associative.clear();
            }
            else
            {
                time_stamp = 1;
                index_for_entry.clear();
                int y = 0;
                while (y != no_of_sets)
                {

                    string z = int_to_binary_advanced(y, ceil(log2(no_of_sets)));
                    vector<tuple<string, string, string, vector<string>, int>> fully=set_associative[z];
                    for(ll i=0;i<fully.size();i++){
                    string dirty=get<DIRTY>(fully[i]);
                    if(dirty=="1"){
                        ll num_of_bloak_offset=20-get<TAG>(fully[i]).size()-z.size();
                        string offset="";
                        while(num_of_bloak_offset--){
                            offset+='0';
                        }
                        ll address_to_write=binary_to_int(get<TAG>(fully[i])+z+offset);
                        vector<string>data=get<DATA>(fully[i]);
                        for(auto i:data){
                            memory[address_to_write++]=i;
                        }
                    }
                }
                    set_associative[z].clear();
                    y++;
                }
            }

            if (ASSOCIATIVITY == 1)
            { // initialization of direct mapped cache;

                ll y = 0;
                while (y != no_of_blocks)
                {
                    string z = int_to_binary_advanced(y, ceil(log2(no_of_blocks)));

                    direct_mapped[z] = {"0", "0", "", {}};
                    y++;
                }
            } // initialization of fully associative cache;
            else if (ASSOCIATIVITY == 0)
            {
                ll y = 0;
                while (y != SIZE_OF_CACHE / BLOCK_SIZE)
                {
                    vector<string> empty_string;
                    fully_associative.push_back({"0", "0", "", empty_string, INT64_MAX});
                    y++;
                }
            }
            else
            {
                int y = 0;
                while (y != no_of_sets)
                {
                    index_for_entry.push_back(0);
                    string z = int_to_binary_advanced(y, ceil(log2(no_of_sets)));
                    for (ll i = 0; i < ASSOCIATIVITY; i++)
                    {
                        tuple<string, string, string, vector<string>, int> rt = {"0", "0", "", {}, INT_MAX};
                        set_associative[z].push_back(rt);
                    }
                    y++;
                }
            }
        }
        if (x == "cache_sim status")
        {
            if (cache_enable_status)
            {
                cout << "Number of sets: " << no_of_sets << endl;
                cout << "Dwords per block: " << BLOCK_SIZE / 8 << endl;
                cout << "Associativity: " << ASSOCIATIVITY << endl;
                cout << "Replacement Policy: " << REPLACEMENT_POLICY << endl;
                cout << "Write Back Policy " << WRITEBACK_POLICY << endl;
            }
            else
            {
                cout << "D-Cache is Disabled" << endl;
            }
        }

        // if (x == "en")
        if (x.substr(0, 16) == "cache_sim enable")
        {
            cache_enable_status = true;
            // Reading Cache Config File
            ll index = -1;
            for (ll i = 16; i < x.size(); i++)
            {
                if (x[i] != ' ')
                {
                    index = i;
                    break;
                }
            }
            string config_file_name = x.substr(index);

            ifstream file;
            file.open(config_file_name);
            string line;
            int count = 0;
            while (getline(file, line))
            {
                switch (count)
                {
                case 0:
                    SIZE_OF_CACHE = stoll(line);
                    count++;
                    break;
                case 1:
                    BLOCK_SIZE = stoll(line);
                    count++;
                    break;
                case 2:
                    ASSOCIATIVITY = stoll(line);
                    count++;
                    break;
                case 3:
                    REPLACEMENT_POLICY = line;
                    count++;
                    break;
                case 4:
                    WRITEBACK_POLICY = line;
                    count++;
                    break;
                }
            }
            if (ASSOCIATIVITY != 0)
            {
                no_of_sets = SIZE_OF_CACHE / (BLOCK_SIZE * ASSOCIATIVITY);
            }
            else
            {
                no_of_sets = 1;
            }

            ll no_of_blocks = SIZE_OF_CACHE / (float)BLOCK_SIZE;

            if (ASSOCIATIVITY == 1)
            { // initialization of direct mapped cache;

                ll y = 0;
                while (y != no_of_blocks)
                {
                    string z = int_to_binary_advanced(y, ceil(log2(no_of_blocks)));

                    direct_mapped[z] = {"0", "0", "", {}};
                    y++;
                }
            } // initialization of fully associative cache;
            else if (ASSOCIATIVITY == 0)
            {

                ll y = 0;
                while (y != no_of_blocks)
                {
                    vector<string> empty_string;
                    fully_associative.push_back({"0", "0", "", empty_string, INT64_MAX});
                    y++;
                }
            }
            else
            {
                int y = 0;
                while (y != no_of_sets)
                {
                    index_for_entry.push_back(0);
                    string z = int_to_binary_advanced(y, ceil(log2(no_of_sets)));
                    for (ll i = 0; i < ASSOCIATIVITY; i++)
                    {
                        tuple<string, string, string, vector<string>, int> rt = {"0", "0", "", {}, INT_MAX};
                        set_associative[z].push_back(rt);
                    }
                    y++;
                }
            }
            file.close();
        }
        else if (x == "cache_sim disable")
        {
            cache_enable_status = false;
        }

        if (x.substr(0, 5) == "break")
        {
            ll break_point = stoll(trimSpaces(x.substr(5)));
            cout << "Breakpoint set at line " << break_point << endl;
            break_points.push_back(break_point - text_section_starts - 1); // pushing the line number starting from zero
        }
        int pos = x.find_first_of(' ');
        if (x.substr(0, pos) == "del")
        {
            string remaining = trimSpaces(x.substr(pos));
            int pos2 = remaining.find_first_of(' ');
            if (remaining.substr(0, pos2) == "break")
            {
                ll break_point = stoll(trimSpaces(remaining.substr(pos2))) - text_section_starts - 1;
                cout << "Deleting Breakpoint at Indx at break point Number " << break_point << endl;

                for (std::vector<int>::iterator it = break_points.begin(); it != break_points.end();)
                {
                    if (*it == break_point)
                    {
                        it = break_points.erase(it);
                    }
                    else
                        ++it;
                }
            }
        }

        if ((x == "step" || x == "run") && PC / 4 >= lines.size()) // end of instructions
        {

            cout << "Nothing to step" << endl;
            while (!call_stack.empty())
                call_stack.pop();
            continue;
        }

        if (x == "step")
        {
            vector<string> v = parse(lines[PC / 4]);
            if (text_section_starts != 0)
            {

                call_stack.top().second = PC / 4 + text_section_starts + 1;
            }
            else
            {
                call_stack.top().second = PC / 4 + text_section_starts + 1;
            }
            if (opcodeR_map.count(v[0]))
            {
                handleR(v);
                cout << "Executed " << lines[PC / 4] << ";" << " PC=" << "0x" + binary_to_hexa(int_to_binary_advanced(PC, 64)).substr(8) << endl;
                PC += 4;
            }
            else if (specialLoadI.count(v[0]))
            {
                handleI_load(v);
                cout << "Executed " << lines[PC / 4] << ";" << " PC=" << "0x" + binary_to_hexa(int_to_binary_advanced(PC, 64)).substr(8) << endl;
                PC += 4;
            }
            else if (opcodeI_map.count(v[0]))
            {
                handleI(v);
                cout << "Executed " << lines[PC / 4] << ";" << " PC=" << "0x" + binary_to_hexa(int_to_binary_advanced(PC, 64)).substr(8) << endl;
                PC += 4;
            }
            else if (opcodeU_map.count(v[0]))
            {
                handleU(v, PC);
                cout << "Executed " << lines[PC / 4] << ";" << " PC=" << "0x" + binary_to_hexa(int_to_binary_advanced(PC, 64)).substr(8) << endl;
                PC += 4;
            }
            else if (opcodeS_map.count(v[0]))
            {
                handleS(v);
                cout << "Executed " << lines[PC / 4] << ";" << " PC=" << "0x" + binary_to_hexa(int_to_binary_advanced(PC, 64)).substr(8) << endl;
                PC += 4;
            }
            else if (opcodeB_map.count(v[0]))
            {
                cout << "Executed " << lines[PC / 4] << ";" << " PC=" << "0x" + binary_to_hexa(int_to_binary_advanced(PC, 64)).substr(8) << endl;
                handleB(v, PC, labelmap);
            }

            else if (opcodeJ_map.count(v[0]))
            {
                cout << "Executed " << lines[PC / 4] << ";" << " PC=" << "0x" + binary_to_hexa(int_to_binary_advanced(PC, 64)).substr(8) << endl;
                ll cur_line_number = PC / 4 + text_section_starts;
                call_stack.push({v[2] + ":", call_stack.top().second});

                handleJ(v, PC, labelmap);
            }
            else if (v[0] == "jalr")
            {

                string rd, rs, offset;
                rd = v[1];
                size_t pos1 = v[2].find('(');
                size_t pos2 = v[2].find(')');
                offset = v[2].substr(0, pos1);
                offset = trimSpaces(offset);
                string rs1 = v[2].substr(pos1 + 1, pos2 - pos1 - 1);
                rs1 = trimSpaces(rs1);

                if (rd != "x0")
                {
                    registers[stoll(rd.substr(1))].second = to_string(PC + 4);
                }
                cout << "Executed " << lines[PC / 4] << ";" << " PC=" << "0x" + binary_to_hexa(int_to_binary_advanced(PC, 64)).substr(8) << endl;
                PC = stoll(offset) + stoll(registers[stoll(rs1.substr(1))].second);

                call_stack.pop();
            }
        }
        else if (x == "regs")
        {
            cout << "Registers:" << endl;
            for (int i = 0; i < 32; i++)
            {
                if (i <= 9)
                {

                    cout << registers[i].first << "  = " << "0x" << reduce_excessive_zeros(binary_to_hexa(int_to_binary_advanced(stoll(registers[i].second), 64))) << endl;
                }
                else
                {

                    cout << registers[i].first << " = " << "0x" << reduce_excessive_zeros(binary_to_hexa(int_to_binary_advanced(stoll(registers[i].second), 64))) << endl;
                }
            }
            cout << endl;
        }
        else if (check_mem(x))
        {
            vector<string> v = get_address_and_count(x);
            string address = v[1];
            ll count = stoll(v[2]);
            address = address.substr(2);
            ll address_value = stoll(address, nullptr, 16);
            ll index = address_value;
            for (int i = 0; i < count; i++)
            {

                stringstream ss;
                ss << hex << address_value + i;
                string hex_str = ss.str();
                if (memory[index].size() == 0)
                {
                    cout << "Memory[0x" << hex_str << "] = 0x0" << memory[index++] << endl;
                }
                else
                {

                    cout << "Memory[0x" << hex_str << "] = 0x" << memory[index++] << endl;
                }
            }
        }
        else if (trimSpaces(x) == "show-stack")
        {
            if (call_stack.empty())
            {
                cout << "Empty Call Stack: Execution complete" << endl;
            }
            else
            {
                cout << "Call Stack:" << endl;
                printStack(call_stack);
                cout << endl;
            }
        }
        else if (x == "run")
        {

            while (PC / 4 != lines.size())
            {
                // if (isCacheFull)
                // {
                //     logfile << "Cache is Full so run Stoppped , Jumping " << endl;
                //     goto cacheFull;
                // }
                const auto i = std::find(break_points.begin(), break_points.end(), PC / 4);
                if (i != break_points.end())
                {
                    cout << "Execution stopped at breakpoint " << endl;
                    break;
                }
                if (text_section_starts != 0)
                {

                    call_stack.top().second = PC / 4 + text_section_starts + 1;
                }
                else
                {
                    call_stack.top().second = PC / 4 + text_section_starts + 1;
                }

                vector<string> v = parse(lines[PC / 4]);
                if (opcodeR_map.count(v[0]))
                {
                    handleR(v);
                    cout << "Executed " << lines[PC / 4] << ";" << " PC=" << "0x" + binary_to_hexa(int_to_binary_advanced(PC, 64)).substr(8) << endl;
                    PC += 4;
                }
                else if (specialLoadI.count(v[0]))
                {
                    handleI_load(v);
                    cout << "Executed " << lines[PC / 4] << ";" << " PC=" << "0x" + binary_to_hexa(int_to_binary_advanced(PC, 64)).substr(8) << endl;
                    PC += 4;
                }
                else if (opcodeI_map.count(v[0]))
                {
                    handleI(v);
                    cout << "Executed " << lines[PC / 4] << ";" << " PC=" << "0x" + binary_to_hexa(int_to_binary_advanced(PC, 64)).substr(8) << endl;
                    PC += 4;
                }
                else if (opcodeU_map.count(v[0]))
                {
                    handleU(v, PC);
                    cout << "Executed " << lines[PC / 4] << ";" << " PC=" << "0x" + binary_to_hexa(int_to_binary_advanced(PC, 64)).substr(8) << endl;
                    PC += 4;
                }
                else if (opcodeS_map.count(v[0]))
                {
                    handleS(v);
                    cout << "Executed " << lines[PC / 4] << ";" << " PC=" << "0x" + binary_to_hexa(int_to_binary_advanced(PC, 64)).substr(8) << endl;
                    PC += 4;
                }
                else if (opcodeB_map.count(v[0]))
                {
                    cout << "Executed " << lines[PC / 4] << ";" << " PC=" << "0x" + binary_to_hexa(int_to_binary_advanced(PC, 64)).substr(8) << endl;
                    handleB(v, PC, labelmap);
                }

                else if (opcodeJ_map.count(v[0]))
                {
                    cout << "Executed " << lines[PC / 4] << ";" << " PC=" << "0x" + binary_to_hexa(int_to_binary_advanced(PC, 64)).substr(8) << endl;
                    call_stack.push({v[2] + ":", call_stack.top().second});
                    handleJ(v, PC, labelmap);
                }
                else if (v[0] == "jalr")
                {
                    string rd, rs, offset;
                    rd = v[1];
                    size_t pos1 = v[2].find('(');
                    size_t pos2 = v[2].find(')');
                    offset = v[2].substr(0, pos1);
                    offset = trimSpaces(offset);
                    string rs1 = v[2].substr(pos1 + 1, pos2 - pos1 - 1);
                    rs1 = trimSpaces(rs1);
                    if (register_map.count(rd))
                    {
                        rd = register_map[rd];
                    }
                    if (register_map.count(rs1))
                    {
                        rs1 = register_map[rs1];
                    }
                    if (rd != "x0")
                    {
                        registers[stoll(rd.substr(1))].second = to_string(PC + 4);
                    }
                    cout << "Executed " << lines[PC / 4] << ";" << " PC=" << "0x" + binary_to_hexa(int_to_binary_advanced(PC, 64)).substr(8) << endl;
                    PC = stoll(offset) + stoll(registers[stoll(rs1.substr(1))].second);
                    call_stack.pop();
                }
                else
                {
                    cout << "no instruction found" << endl;
                }
            }
            cout << "D-cache statistics: Accesses=" << hits + misses << ", Hit=" << hits << ", Miss=" << misses << ", Hit Rate=" << (float)hits / (hits + misses) << endl;
        }
    cacheFull:

        if (PC / 4 == lines.size())
        { // after executing last line
            if (!call_stack.empty())
            {
                call_stack.pop();
            }
        }

        if (x == "exit")
        {
            cout << "Exited the Simulator" << endl;
            break;
        }
    }

    return 0;
}