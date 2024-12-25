# Assembler

## Project Structure

```bash

├── CMakeLists.txt
├── include
│   ├── base.hh
│   ├── b_type.hh
│   ├── i_type.hh
│   ├── j_type.hh
│   ├── r_type.hh
│   ├── s_type.hh
│   └── u_type.hh
├── readme.md
├── report.pdf
├── src
│   ├── b_type.cpp
│   ├── compare_files.cpp
│   ├── i_type.cpp
│   ├── j_type.cpp
│   ├── main.cpp
│   ├── r_type.cpp
│   ├── s_type.cpp
│   └── u_type.cpp
└── test
    ├── expected
    │   ├── answer10U.hex
    │   ├── answer1R.hex
    │   ├── answer2R.hex
    │   ├── answer3I.hex
    │   ├── answer4I.hex
    │   ├── answer5S.hex
    │   ├── answer8B.hex
    │   ├── answerF1.hex
    │   ├── answerF2.hex
    │   ├── answerF3.hex
    │   ├── answerF4.hex
    │   ├── answerF5.hex
    │   └── answerF.hex
    ├── input
    │   ├── input10U.s
    │   ├── input1R.s
    │   ├── input2R.s
    │   ├── input3I.s
    │   ├── input4I.s
    │   ├── input5S.s
    │   ├── input6S.s
    │   ├── input7J.s
    │   ├── input8B.s
    │   ├── input9B.s
    │   ├── inputE1.s
    │   ├── inputF1.s
    │   ├── inputF2.s
    │   ├── inputF3.s
    │   ├── inputF4.s
    │   ├── inputF5.s
    │   ├── inputF.s
    │   └── inputtest.s
    ├── output
    └── run_and_compare.sh
  

```


- The _include_ directory contains header files with function definitions for different instruction classes.
- The _src_ directory holds the source files containing function implementations and the `main.cpp` file.
- The _test_ directory contains the testing files used to validate the project.
- The _report.pdf_ contains a detailed report on the project.
- run_and_compare.sh  : Used by Ctest to compare the generated and actual answer

## 🚀 Getting Started

  
  Navigate to the `ai23btech11018` directory. To build and compile the project, use the following commands:
  

```bash

cmake -S . -B build

cmake --build build -j 8

```

  
The binary `riscv_asm` will be located in `/build/bin/riscv_asm`.

To run the assembler with a specific input file:

```bash

./build/bin/riscv_asm input.s

```


The output (in hexadecimal format) will be generated as `./output.hex`.


### To View My Testing

Navigate to bash directory and run "ctest" command.

Change the output file path  in main.cpp to path of (/test/output at your machine)  + "output" 
```bash
cd build
ctest
```


## Features

The assembler supports the following RISC-V 64 instructions: R-type, S-type, I-type, B-type, J-type, and U-type instructions.

### Key Features:

- **Comments**: Supports comments using `#`, similar to the Ripes simulator. Both inline and separate-line comments are supported.
- **Flexible Spacing**: Handles varying amounts of space between instructions and arguments.
- **Line Handling**: Can process an arbitrary number of lines in the input file.
- **Immediate Values**:
    - For I-type instructions: [-2048, 2047] (except for `slli`, `srai`, `srli`, which support [0, 63]).
    - For B-type instructions: [-4096, 4094].
    - For S-type instructions: [-2048, 2047].
    - For U-type instructions: [0, 1048575], with `lui` allowing up to 5 hexadecimal digits (maximum 20-bit immediate).
    - For J-type instructions: [-524287, 1048574] (limited to a range of -2^19 to 2^20 - 1 due to implementation constraints).
- **Instruction Formatting**: Allows arbitrary spaces between registers (e.g., `sub x1 , x2 ,x3` is valid).
- **Offsets** Supports only Decimal Offsets . Only for lui, hexadecimal offsets are allowed

## Error Handling

The assembler is equipped with robust error handling, covering the following scenarios:

- Invalid register names.
- Immediate values out of bounds.
- Redeclaration of labels.
- Undefined labels.
- Invalid or unsupported instructions._