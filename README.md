# CPU Emulator – README

Demichu5 CPU Architecture - Reference Manual[cite: 1]
Version 1.0 (Rev. 2026) 8-Bit Architecture[cite: 1]

---

### 1. Programming Model[cite: 1]

1.1 Memory Organization[cite: 1]
- Address Space: 64 KB (65,536 Bytes)[cite: 1]
- Architecture: Von Neumann (shared code and data)[cite: 1]
- Data Word: 8 Bits (1 Byte)[cite: 1]
- Address Bus: 16 Bits (2 Bytes)[cite: 1]
- Byte Order: Little-Endian[cite: 1]

1.2 General Purpose Registers[cite: 1]
- R0 - R15: 16 8-bit registers for data and counters.[cite: 1]

1.3 Special and Pointer Registers[cite: 1]
- PC: 16-bit Program Counter (+2 per fetch).[cite: 1]
- PTR: 16-bit Pointer Register (High/Low) for load/store.[cite: 1]

1.4 Status Register[cite: 1]
- Z (ZERO): Set if result is 0.[cite: 1]
- C (CARRY): Set if result > 255.[cite: 1]
- N (NEGATIVE): Set if result is negative.[cite: 1]

---

### 2. Instruction Format[cite: 1]

Length: 2 Bytes (16 bits)[cite: 1]
Encoding: [ 8 bits : OPCODE ] [ 8 bits : ARGUMENT ][cite: 1]

---

### 3. Instruction Set Architecture (ISA)[cite: 1]

3.1 Data Transfer[cite: 1]
- set   (0x11) rD      : Select active register[cite: 1]
- mov   (0x12) imm8    : act = imm8[cite: 1]
- cpy   (0x13) rD rS   : rD = rS[cite: 1]
- ptr   (0x14) rH rL   : Set PTR High/Low[cite: 1]
- load  (0x17) rD      : rD = MEM[PTR][cite: 1]
- store (0x18) rS      : MEM[PTR] = rS[cite: 1]
- push  (0x19) rS      : STACK[SP++] = rS[cite: 1]
- pop   (0x1A) rD      : rD = STACK[--SP][cite: 1]

3.2 Arithmetic (ALU)[cite: 1]
- add (0x21) rD rS : rD = rD + rS (Flags: Z, C)[cite: 1]
- sub (0x22) rD rS : rD = rD - rS (Flags: Z, N)[cite: 1]
- cmp (0x23) rD rS : rD - rS (Flags only)[cite: 1]
- inc (0x24) rD    : rD++ (Flag: Z)[cite: 1]
- dec (0x25) rD    : rD-- (Flag: Z)[cite: 1]

3.3 Flow Control[cite: 1]
- jmp   (0x33) Label : Unconditional jump[cite: 1]
- jmp_z (0x34) Label : Jump if NOT ZERO[cite: 1]
- jmp_c (0x35) Label : Jump if NO CARRY[cite: 1]
- jmp_n (0x36) Label : Jump if POSITIVE[cite: 1]
- call  (0x37) Label : Call function[cite: 1]
- ret   (0x38)       : Return[cite: 1]
- halt  (0x00)       : Stop CPU[cite: 1]

---

### 4. Assembler Directives[cite: 1]

- .code : Instruction section[cite: 1]
- .data : Variable section[cite: 1]
- var <name> <size> <value> : Define variable[cite: 1]

---

### 5. Example Program (Bubble Sort)[cite: 1]

.data[cite: 1]
var numbers[5] {5 1 4 2 8}[cite: 1]

.code[cite: 1]
fn MAIN[cite: 1]
    set r5[cite: 1]
    mov 20[cite: 1]
    set r0[cite: 1]
    mov 0[cite: 1]
    set r15[cite: 1]
    mov 0[cite: 1]

MAIN_LOOP:[cite: 1]
    cmp r5 r0[cite: 1]
    jmp_z START_PASS[cite: 1]
    halt[cite: 1]

START_PASS:[cite: 1]
    set r10[cite: 1]
    mov numbers[cite: 1]
    set r11[cite: 1]
    mov numbers[cite: 1]
    inc r11[cite: 1]
    set r4[cite: 1]
    mov 4[cite: 1]

INNER_LOOP:[cite: 1]
    cmp r4 r0[cite: 1]
    jmp_z CHECK_PAIR[cite: 1]
    jmp NEXT_MAIN_PASS[cite: 1]

CHECK_PAIR:[cite: 1]
    ptr r15 r10[cite: 1]
    load r1 PTR[cite: 1]
    ptr r15 r11[cite: 1]
    load r2 PTR[cite: 1]
    cmp r1 r2[cite: 1]
    jmp_n DO_SWAP[cite: 1]
    jmp CONTINUE[cite: 1]

DO_SWAP:[cite: 1]
    ptr r15 r11[cite: 1]
    store r1 PTR[cite: 1]
    ptr r15 r10[cite: 1]
    store r2 PTR[cite: 1]

CONTINUE:[cite: 1]
    inc r10[cite: 1]
    inc r11[cite: 1]
    dec r4[cite: 1]
    jmp INNER_LOOP[cite: 1]

NEXT_MAIN_PASS:[cite: 1]
    dec r5[cite: 1]
    jmp MAIN_LOOP[cite: 1]

---

### 6. Licensing[cite: 1]

Copyright (C) 2025 Demichu5[cite: 1]
Licensed under GPLv3. Commercial licenses available upon contact.[cite: 1]
