# Fully Associative Cache Test
.data 
    .word 100
.text


    lui x3, 0x10           # Base address of data section (0x10000000)
    
    # Initial load to bring data into cache
    ld x5, 0(x3)             # x5 = 10

    # Modify the loaded data and store
    addi x7, x0, 123         # x7 = 123
   addi x21,x0,8

    # Verify modification
    ld x9, 0(x3)             # x9 should be 123

    # Access multiple addresses to fully utilize cache space
    ld x10, 16(x3)           # Access 0x10000010
    sd x21,0(x3)
    ld x11, 32(x3)           # Access 0x10000020
    ld x12, 48(x3)           # Access 0x10000030

    ld x4,64(x3)