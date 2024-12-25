.data
.dword 10, 20, 30, 40, 50
.text
lui x3, 0x10
ld x5, 0(x3)
addi x7, x0, 123
addi x7, x7, 8
sd x7, 0(x3)
sd x9, 64(x3)
sd x9, 0(x3)