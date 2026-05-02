# CPU Emulator – README

Demichu5 CPU Architecture - Reference Manual
Version 1.0 (Rev. 2026) 8-Bit Architecture

Tested with GCC/TCC
tcc cpu.c filehandler.c -o main
python3 compiler.py <program.asm>
./main <program.bin>

---

### Table of Contents
1. Programming Model
2. Instruction Format
3. Instruction Set Architecture (ISA)
4. Assembler Directives
5. Example Program (Bubble Sort)
6. Licensing

---

# 1. Programming Model
1.1 Memory Organization
    * Address Space: 64 KB (65,536 Bytes)
    * Architecture: Von Neumann (shared code and data space)
    * Data Word: 8 Bits (1 Byte)
    * Address Bus: 16 Bits (2 Bytes)
    * Byte Order: Little-Endian

1.2 General Purpose Registers
    * R0 - R15: 16 8-bit registers for temporary data and counters.

1.3 Special and Pointer Registers
    * PC: 16-bit Program Counter (auto-increment +2).
    * PTR: 16-bit Pointer Register (High/Low) for load/store operations.

1.4 Status Register
    * Z Flag (ZERO): Set if result is 0.
    * C Flag (CARRY): Set if result > 255.
    * N Flag (NEGATIVE): Set if result is negative.

---

# 2. Instruction Format
Length: 2 Bytes (16 bits).
Encoding: [ 8 bits : OPCODE ] [ 8 bits : ARGUMENT ]
Argument types: Immediate, Register (4 bits), or Register-Register (nibbles).

---

# 3. Instruction Set Architecture (ISA)

3.1 Data Transfer
Mnemonic    Opcode      Operand     Description
set         0x11        rD          Selects rD as active register for mov
mov         0x12        imm8        act = imm8
cpy         0x13        rD rS       rD = rS
ptr         0x14        rH rL       PTR_High = rH, PTR_Low = rL
ptrh        0x15        imm8        PTR_High = imm8
ptrl        0x16        imm8        PTR_Low  = imm8
load        0x17        rD          rD = MEM[PTR]
store       0x18        rS          MEM[PTR] = rS
push        0x19        rS          STACK[SP++] = rS
pop         0x1A        rD          rD = STACK[--SP]

3.2 Arithmetic and Logic (ALU)
Mnemonic    Opcode      Operand     Flags       Description
add         0x21        rD rS       Z, C        rD = rD + rS
sub         0x22        rD rS       Z, N        rD = rD - rS
cmp         0x23        rD rS       Z, N, C     rD - rS (updates flags only)
inc         0x24        rD          Z           rD = rD + 1
dec         0x25        rD          Z           rD = rD - 1

3.3 Flow Control
Mnemonic    Opcode      Operand     Condition       Description
jmp         0x33        Label       -               Unconditional jump
jmp_z       0x34        Label       Z == 0          Jump if NOT ZERO
jmp_c       0x35        Label       C == 0          Jump if NO CARRY
jmp_n       0x36        Label       N == 0          Jump if POSITIVE
call        0x37        Label       -               STACK[SP++] = PC
ret         0x38        -           -               PC = STACK[--SP]
halt        0x00        -           -               Stops CPU clock

---

# 4. Assembler Directives
* .code - instruction section
* .data - variable section
* var <name> <size> <value>
    * var buffer[10] (array of 0s)
    * var string "Text" (null-terminated)

---

# 5. Example Program (Bubble Sort)
; bubblesort.asm
.data
    var numbers[5] {5 1 4 2 8}

.code
fn MAIN
    set r5
    mov 20
    set r0
    mov 0
    set r15
    mov 0

MAIN_LOOP:
    cmp r5 r0
    jmp_z START_PASS
    halt

START_PASS:
    set r10
    mov numbers
    set r11
    mov numbers
    inc r11
    set r4
    mov 4

INNER_LOOP:
    cmp r4 r0
    jmp_z CHECK_PAIR
    jmp NEXT_MAIN_PASS

CHECK_PAIR:
    ptr r15 r10
    load r1 PTR
    ptr r15 r11
    load r2 PTR
    cmp r1 r2
    jmp_n DO_SWAP
    jmp CONTINUE

DO_SWAP:
    ptr r15 r11
    store r1 PTR
    ptr r15 r10
    store r2 PTR

CONTINUE:
    inc r10
    inc r11
    dec r4
    jmp INNER_LOOP

NEXT_MAIN_PASS:
    dec r5
    jmp MAIN_LOOP

---

# 6. Licensing
Copyright (C) 2025 Demichu5
Licensed under GNU General Public License (GPL) version 3.

Commercial Licensing: Available for proprietary products. Contact author for terms.