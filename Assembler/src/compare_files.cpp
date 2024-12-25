#include <iostream>
#include <fstream>
#include <string>

using namespace std;

int main(int argc, char* argv[]) {
    if (argc != 3) {
        cerr << "Error: Usage: " << argv[0] << " <generated_file> <expected_file>" << endl;
        return 1;
    }

    ifstream generated(argv[1]);
    ifstream expected(argv[2]);

    if (!generated.is_open() || !expected.is_open()) {
        cerr << "Error: Could not open files." << endl;
        return 1;
    }

    string gen_line, exp_line;
    int line_num = 1;
    bool differences_found = false;

    while (getline(generated, gen_line) && getline(expected, exp_line)) {
        if (gen_line != exp_line) {
            cout << "Difference found on line " << line_num << ":" << endl;
            cout << "Generated: " << gen_line << endl;
            cout << "Expected:  " << exp_line << endl;
            differences_found = true;
        }
        line_num++;
    }

    // Check for extra lines in either file
    while (getline(generated, gen_line)) {
        cout << "Extra line in generated file on line " << line_num << ": " << gen_line << endl;
        differences_found = true;
        line_num++;
    }

    while (getline(expected, exp_line)) {
        cout << "Extra line in expected file on line " << line_num << ": " << exp_line << endl;
        differences_found = true;
        line_num++;
    }

        if (differences_found){
            return 1;
        }
    return 0;
}
