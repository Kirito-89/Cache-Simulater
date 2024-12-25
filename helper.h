#include <string>
#include <map>
#include <vector>
#include <sstream>
#include <algorithm>
#include <cstdint>
#include <stack>


#define ll long long int 
#define ull unsigned long long int 
using namespace std;
map<string, char> hex_map = {{"0000", '0'}, {"0001", '1'}, {"0010", '2'}, {"0011", '3'}, {"0100", '4'}, {"0101", '5'}, {"0110", '6'}, {"0111", '7'}, {"1000", '8'}, {"1001", '9'}, {"1010", 'a'}, {"1011", 'b'}, {"1100", 'c'}, {"1101", 'd'}, {"1110", 'e'}, {"1111", 'f'}};



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
int find_filename(string s)
{
    int index = 0;
    for (int i = 0; i < s.size(); i++)
    {
        if (s[i] == ' ')
        {
            index = i;
            break;
        }
    }
    return index;
}

bool find_load_keyword(string s)
{
    int index = -1;
    for (int i = 0; i < s.size(); i++)
    {
        if (s[i] == ' ')
        {
            index = i;
            break;
        }
    }
    if (s.substr(0, index) == "load")
    {
        return true;
    }
    return false;
}

string binary_to_hexa(string s)
{
    string ans = "";
    for (int i = 0; i < 64; i += 4)
    {
        ans += hex_map[s.substr(i, 4)]; // segment the string into 4 bits each and use the hexadecimal map to get the hex digit corrresponding to it
    }
    return ans;
}
string binary_to_hexa_advanced(string s,ll n)
{
    string ans = "";
    for (int i = 0; i < n; i += 4)
    {
        ans += hex_map[s.substr(i, 4)]; // segment the string into 4 bits each and use the hexadecimal map to get the hex digit corrresponding to it
    }
    return ans;
}


vector<string> parse(string s)
{
    vector<string> parsed;
    stringstream ss(s);
    string token;
    bool first_operand = true;
    while (getline(ss, token, ','))
    {
        if(first_operand){
            int pos_first_space = token.find_first_of(' ');

            string operation = token.substr(0, pos_first_space);
            string o1 = token.substr(pos_first_space + 1);
            // no need to trim the operation !!!(Check !!! Reducing Computation )
          
          
            parsed.push_back(  trimSpaces(operation));
            parsed.push_back(  trimSpaces(o1));

            first_operand = false;
        }
        else{
             // o3
            parsed.push_back( trimSpaces(token));
        }
      
    }

    return parsed;
}

string int_to_binary_advanced(ll n, ll x)
{
    ull num = n;
    string binary = "";
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
    reverse(binary.begin(), binary.end());
    return binary.substr(64 - x);
}
ll binary_to_int(string num){
  ll sum = 0;
  int length= num.length();
  for(int i = 0;i< length;i++){
        char c = num[i];
        if(c=='1'){
            sum+= pow(2,length-1 -i);
           
        }
  }  
  return sum;
}

bool check_data(string x)
{

    // cout<<index<<endl;
    if (x == ".data")
        return true;
    return false;
}
bool check_text(string x)
{
    if (x == ".text")
        return true;
    return false;
}

string give_type(string s)
{
    int index = 0;
    int n = s.size();
    for (int i = 0; i < n; i++)
    {
        if (s[i] == ' ')
        {
            index = i;
            break;
        }
    }
    return s.substr(0, index);
}
string reduce_excessive_zeros(string s)
{
    int index = -1;
    for (int i = 0; i < s.size(); i++)
    {
        if (s[i] != '0')
        {
            index = i;
            break;
        }
    }
    if (index == -1)
    {
        return "0";
    }
    return s.substr(index);
}

bool find_load(string s)

{
    int index = 0;
    for (int i = 0; i < s.size(); i++)
    {
        if (s[i] == ' ')
        {
            index = i;
            break;
        }
    }
    if (s.substr(0, index) == "load")
    {
        return true;
    }
    return false;
}

bool isNumericString(const std::string &str)
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

vector<string> get_address_and_count(string s)
{
    vector<string> v;
    int l = 0;
    for (int i = 0; i < s.size(); i++)
    {
        if (s[i] == ' ')
        {
            v.push_back(s.substr(l, i - l));
            l = i + 1;
        }
    }
    v.push_back(s.substr(l));
    return v;
}
vector<string> get_numbers(string s)
{
    // cout << "here" << endl;
    // cout << s << endl;
    int index = 0;
    for (int i = 0; i < s.size(); i++)
    {
        if (s[i] == ' ')
        {
            index = i;
            break;
        }
    }
    vector<string> v;
    int l = index + 1;
    for (int i = index + 1; i < s.size(); i++)
    {
        if (s[i] == ',')
        {
            v.push_back(s.substr(l, i - l));
            l = i + 1;
        }
    }
    v.push_back(s.substr(l));
    return v;
}
bool check_mem(string s)
{
    int index = 0;
    for (int i = 0; i < s.size(); i++)
    {
        if (s[i] == ' ')
        {
            index = i;
            break;
        }
    }
    if (s.substr(0, index) == "mem")
    {
        return true;
    }
    return false;
}

uint64_t parse_hex(const string &str)
{
    return stoll(str, nullptr, 16);
}

// void printStack(stack<string> &s){
//         if (s.empty()){return;}
//         string x = s.top();
//         s.pop();
//         printStack(s);

//         cout << x << endl;
//         s.push(x);

// }
void printStack(stack<pair<string,ll>> &s){
        if (s.empty()){return;}
        pair<string,ll> x = s.top();
        s.pop();
        printStack(s);

        cout << x.first<<x.second << endl;
        s.push(x);

}