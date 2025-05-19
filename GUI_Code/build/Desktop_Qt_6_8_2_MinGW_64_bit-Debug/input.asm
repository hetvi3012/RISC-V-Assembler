.data
.byte 20,10,30,4,5,19,16,-50,10,3

.text
lui x10,0x10000
addi x11,x0,10
addi x12,x11,-1 # n-1

jal x1,bubble
beq x0,x0,exit

bubble:
addi sp,sp,-4
sw x1,0,sp

bge x0,x12,exit1 
lui x10,0x10000
add x13,x12,x0
        while2: bge x0,x13,exit2
        addi x10,x10,1
        lb x5,-1,x10
        lb x6,0,x10
        addi x13,x13,-1
        bge x6,x5, while2
        sb x6,-1,x10
        sb x5,0,x10
        beq x0,x0,while2

        exit2:
        addi x12,x12,-1
        jal x1,bubble
        lw x1,0,sp
exit1:
addi sp,sp,4
jalr x0,x1,0

exit: