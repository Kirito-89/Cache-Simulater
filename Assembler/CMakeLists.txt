# Define the minimum version of CMake
cmake_minimum_required(VERSION 3.10)

# Project name and language
project(riscv_asm LANGUAGES CXX)

# Specify the C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED YES)

# Include directory
include_directories(${CMAKE_SOURCE_DIR}/include)

# Set directories
set(SOURCE_DIR "${CMAKE_SOURCE_DIR}/src")
set(TEST_DIR "${CMAKE_SOURCE_DIR}/test")

# Create object files for each source file
add_library(riscv_lib STATIC
    ${SOURCE_DIR}/r_type.cpp
    ${SOURCE_DIR}/s_type.cpp
    ${SOURCE_DIR}/i_type.cpp
    ${SOURCE_DIR}/b_type.cpp
    ${SOURCE_DIR}/u_type.cpp
    ${SOURCE_DIR}/j_type.cpp)


# Add executable that links to the static library
add_executable(riscv_asm ${SOURCE_DIR}/main.cpp)

# Link the static library to the main executable
target_link_libraries(riscv_asm riscv_lib)

# Define test executable
add_executable(compare_files ${SOURCE_DIR}/compare_files.cpp)



enable_testing()



add_test(NAME CombinedTestCase1
COMMAND ${CMAKE_SOURCE_DIR}/test/run_and_compare.sh
        ${TEST_DIR}/input/input1R.s
        1R
        ${TEST_DIR}/output/output1R.hex
        ${TEST_DIR}/expected/answer1R.hex
        ${CMAKE_SOURCE_DIR})


add_test(NAME CombinedTestCase2
COMMAND ${CMAKE_SOURCE_DIR}/test/run_and_compare.sh
        ${TEST_DIR}/input/input2R.s
        2R
        ${TEST_DIR}/output/output2R.hex
        ${TEST_DIR}/expected/answer2R.hex
        ${CMAKE_SOURCE_DIR})


        
                                
add_test(NAME CombinedTestCase3
COMMAND ${CMAKE_SOURCE_DIR}/test/run_and_compare.sh
        ${TEST_DIR}/input/input3I.s
        3I
        ${TEST_DIR}/output/output3I.hex
        ${TEST_DIR}/expected/answer3I.hex
        ${CMAKE_SOURCE_DIR})        
                                
                                
                                
add_test(NAME CombinedTestCase4
COMMAND ${CMAKE_SOURCE_DIR}/test/run_and_compare.sh
        ${TEST_DIR}/input/input4I.s
        4I
        ${TEST_DIR}/output/output4I.hex
        ${TEST_DIR}/expected/answer4I.hex
        ${CMAKE_SOURCE_DIR})
        
                                        
add_test(NAME CombinedTestCase5
COMMAND ${CMAKE_SOURCE_DIR}/test/run_and_compare.sh
        ${TEST_DIR}/input/input5S.s
        5S
        ${TEST_DIR}/output/output5S.hex
        ${TEST_DIR}/expected/answer5S.hex
        ${CMAKE_SOURCE_DIR})

add_test(NAME CombinedTestCase6
COMMAND ${CMAKE_SOURCE_DIR}/test/run_and_compare.sh
        ${TEST_DIR}/input/input8B.s
        8B
        ${TEST_DIR}/output/output8B.hex
        ${TEST_DIR}/expected/answer8B.hex
        ${CMAKE_SOURCE_DIR})



add_test(NAME CombinedTestCase7
COMMAND ${CMAKE_SOURCE_DIR}/test/run_and_compare.sh
        ${TEST_DIR}/input/inputF.s
        F
        ${TEST_DIR}/output/outputF.hex
        ${TEST_DIR}/expected/answerF.hex
        ${CMAKE_SOURCE_DIR})
        

        add_test(NAME CombinedTestCase8
        COMMAND ${CMAKE_SOURCE_DIR}/test/run_and_compare.sh
                ${TEST_DIR}/input/inputF1.s
                F1
                ${TEST_DIR}/output/outputF1.hex
                ${TEST_DIR}/expected/answerF1.hex
                ${CMAKE_SOURCE_DIR})     
                
                
                add_test(NAME CombinedTestCase9
        COMMAND ${CMAKE_SOURCE_DIR}/test/run_and_compare.sh
                ${TEST_DIR}/input/inputF2.s
                F2
                ${TEST_DIR}/output/outputF2.hex
                ${TEST_DIR}/expected/answerF2.hex
                ${CMAKE_SOURCE_DIR})


                

        add_test(NAME CombinedTestCase10
        COMMAND ${CMAKE_SOURCE_DIR}/test/run_and_compare.sh
                ${TEST_DIR}/input/inputF3.s
                F3
                ${TEST_DIR}/output/outputF3.hex
                ${TEST_DIR}/expected/answerF3.hex
                ${CMAKE_SOURCE_DIR})
                  


                add_test(NAME CombinedTestCase11
                COMMAND ${CMAKE_SOURCE_DIR}/test/run_and_compare.sh
                        ${TEST_DIR}/input/inputF4.s
                        F4
                        ${TEST_DIR}/output/outputF4.hex
                        ${TEST_DIR}/expected/answerF4.hex
                        ${CMAKE_SOURCE_DIR})
                          
        
                        add_test(NAME CombinedTestCase12
                        COMMAND ${CMAKE_SOURCE_DIR}/test/run_and_compare.sh
                                ${TEST_DIR}/input/inputF5.s
                                F5
                                ${TEST_DIR}/output/outputF5.hex
                                ${TEST_DIR}/expected/answerF5.hex
                                ${CMAKE_SOURCE_DIR})
                                  
                

                                
                                add_test(NAME CombinedTestCase13
                                COMMAND ${CMAKE_SOURCE_DIR}/test/run_and_compare.sh
                                        ${TEST_DIR}/input/test.s
                                        test
                                        ${TEST_DIR}/output/outputtest.hex
                                        ${TEST_DIR}/expected/answertest.hex
                                        ${CMAKE_SOURCE_DIR})
                                          
                                      
        

set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR}/bin)