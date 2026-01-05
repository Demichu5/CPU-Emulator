// Biblioteki
#include <stdio.h>

// Definicje
#define HIGH_NIBBLE(byte)   (((byte) >> 4) & 0x0F)
#define LOW_NIBBLE(byte)    ((byte) & 0x0F)
#define ARGUMENT_DEST(byte) HIGH_NIBBLE(byte)
#define ARGUMENT_SRC(byte)  LOW_NIBBLE(byte)

#define FLAG_ZERO   0
#define FLAG_CARRY  1
#define FLAG_NEG    2

#define ARG_LOOP    0x81
#define ARG_PTR     0x82

#define STACK_SIZE  511
typedef union {
    struct {unsigned char ptrL, ptrH;};
    unsigned short int ptrW;
} Pointer;

// BUS
unsigned char outBuffer;
unsigned char inBuffer;
unsigned char interrupt = 0;

// CORE
unsigned short int CORE_Fun_stackSpace[STACK_SIZE];
unsigned short int CORE_Loop_stackSpace[STACK_SIZE];
unsigned char CORE_Rej_stackSpace[STACK_SIZE];

unsigned char CORE_ramSpace[1024];
unsigned char CORE_running = 1;
unsigned char CORE_funPointer = 0;
unsigned char CORE_loopPointer = 0;
unsigned char CORE_rejPointer = 0;

// Rejestry
unsigned short int CPU_PC = 0;
Pointer CPU_PTR;

/*  r[0] - r[7] Rejestry uniwersalne R0-R7
    r[8] - r[12] Rejestry specialne (agrumenty) S0 - S5
    r[13] - interrupt data
    r[14] - rejest ptrH
    r[15] - rejestr ptrL*/
unsigned char CPU_R[16];

/*  Flags[0] - zero
    Flags[1] - carry
    Flags[2] - negative*/
unsigned char CPU_Flags[3];

/*  Aktywny rejestr*/
unsigned char CPU_activeRejestrs;

void push8(unsigned char *stack, unsigned char *ptr, unsigned char data){
    if(*ptr == 0xFF){
        CORE_running = 0;
        return;
    }
    stack[*ptr] = data;
    (*ptr)++;
}

unsigned char pop8(unsigned char *stack, unsigned char *ptr){
    if(*ptr > 0x00){
        (*ptr)--;
        return stack[*ptr];
    }
    return 0;
}

void push16(unsigned short int *stack, unsigned char *ptr, unsigned short data){
    if(*ptr == 0xFF){
        CORE_running = 0;
        return;
    }
    stack[*ptr] = data;
    (*ptr)++;
}

unsigned short int pop16(unsigned short int *stack, unsigned char *ptr){
    if(*ptr > 0x00){
        (*ptr)--;
        return stack[*ptr];
    }
    return 0;
}

unsigned short int peek16(unsigned short int *stack, unsigned char *ptr){
    if(*ptr > 0x00){
        return stack[*ptr - 1];
    }
    return 0;
}

void checkInterrupt(){

}

void fetch(const unsigned char *opcode, const unsigned char *argument){
    CPU_PTR.ptrH = CPU_R[14];
    CPU_PTR.ptrL = CPU_R[15];
    switch(*opcode) {
        /* ----- Dane ----- */
        /*0001-0001 SET*/
        case 0x11:
            CPU_activeRejestrs = *argument & 0x0F;
            break;

        /*0001-0010 MOV*/
        case 0x12:
            CPU_R[CPU_activeRejestrs] = *argument;
            break;

        /*0001-0011 CPY*/
        case 0x13: {
            unsigned char dest = ARGUMENT_DEST(*argument);
            unsigned char src = ARGUMENT_SRC(*argument);

            CPU_R[dest] = CPU_R[src];
            break;
        }

        /*0001-0100 PTR*/
        case 0x14: {
            unsigned char high = ARGUMENT_DEST(*argument);
            unsigned char low = ARGUMENT_SRC(*argument);

            CPU_PTR.ptrH = CPU_R[high];
            CPU_PTR.ptrL = CPU_R[low];
            break;
        }

        /*0001-0101 PTR_H*/
        case 0x15:
            CPU_PTR.ptrH = *argument;
            break;

        /*0001-0110 PTR_L*/
        case 0x16:
            CPU_PTR.ptrL = *argument;
            break;
        
        /*0001-0111 LOAD*/
        case 0x17:
            CPU_R[*argument & 0x0F] = CORE_ramSpace[CPU_PTR.ptrW];
            break;

        /*0001-1000 STORE*/
        case 0x18:
            CORE_ramSpace[CPU_PTR.ptrW] = CPU_R[*argument & 0x0F];
            break;

        /*0001-1001 PUSH*/
        case 0x19:
            push8(CORE_Rej_stackSpace, &CORE_rejPointer, CPU_R[*argument & 0x0F]);
            break;

        /*0001-1010 POP*/
        case 0x1A:
            CPU_R[*argument & 0x0F] = pop8(CORE_Rej_stackSpace, &CORE_rejPointer);
            break;

            
        /* ----- Arytmeryka ----- */
        /*0010-0001 ADD*/
        case 0x21: {
            unsigned char dest = ARGUMENT_DEST(*argument);
            unsigned char src = ARGUMENT_SRC(*argument);

            unsigned short int result = (unsigned short int)CPU_R[dest] + (unsigned short int)CPU_R[src];
            CPU_R[dest] = (unsigned char)(result & 0xFF);

            if(result > 255){
                CPU_Flags[FLAG_CARRY] = 1;
            } else {
                CPU_Flags[FLAG_CARRY] = 0;
            }
            break;
        }

        /*0010-0010 SUB*/
        case 0x22: {
            unsigned char dest = ARGUMENT_DEST(*argument);
            unsigned char src = ARGUMENT_SRC(*argument);

            unsigned short int result = (unsigned short int)CPU_R[dest] - (unsigned short int)CPU_R[src];
            CPU_R[dest] = (unsigned char)(result & 0xFF);

            if(result > CPU_R[dest]){
                CPU_Flags[FLAG_NEG] = 1;
            } else {
                CPU_Flags[FLAG_NEG] = 0;
            }
            break;
        }

        /*0010-0011 CMP*/
        case 0x23: {
            CPU_Flags[FLAG_ZERO] = 0;
            CPU_Flags[FLAG_CARRY] = 0;
            CPU_Flags[FLAG_NEG] = 0;

            unsigned char desc = ARGUMENT_DEST(*argument);
            unsigned char src = ARGUMENT_SRC(*argument);

            short int result = (short int)CPU_R[desc] - (short int)CPU_R[src];
            if(result == 0){
                CPU_Flags[FLAG_ZERO] = 1;
            } else if (result > 0){
                CPU_Flags[FLAG_CARRY] = 1;
            } else if (result < 0){
                CPU_Flags[FLAG_NEG] = 1;
            }
            break;
        }
        
        /*0010_0100 INC*/
        case 0x24:
            CPU_R[*argument & 0x0F]++;
            if(CPU_R[*argument & 0x0F] == 0){
                CPU_Flags[FLAG_ZERO] = 1;
            } else {
                CPU_Flags[FLAG_ZERO] = 0;
            }
            break;

        /*0010-0101 DEC*/
        case 0x25:
            CPU_R[*argument & 0x0F]--;
            if(CPU_R[*argument & 0x0F] == 0){
                CPU_Flags[FLAG_ZERO] = 1;
            } else {
                CPU_Flags[FLAG_ZERO] = 0;
            }
            break;

        /* ----- Kontrola ----- */
        /*0011-0001 SETLOOP*/
        case 0x31:
            push16(CORE_Loop_stackSpace, &CORE_loopPointer, CPU_PC + 2);
            break;

        /*0011-0010 ENDLOOP*/
        case 0x32:
            pop16(CORE_Loop_stackSpace, &CORE_loopPointer);
            break;

        /*0011-0011 JMP*/
        case 0x33:
            // LOOP 
            if(*argument == ARG_LOOP){
                unsigned short int _target = peek16(CORE_Loop_stackSpace, &CORE_loopPointer);
                if(CORE_loopPointer > 0){
                    CPU_PC = _target - 2;
                }
            }
            // PTR
            if(*argument == ARG_PTR){
                CPU_PC = CPU_PTR.ptrW;
            }
            break;

        /*0011-0100 JMP_Z*/
        case 0x34:
            if(CPU_Flags[FLAG_ZERO] == 0){
                if(*argument == ARG_LOOP){
                    unsigned short int _target = peek16(CORE_Loop_stackSpace, &CORE_loopPointer);
                    if(CORE_loopPointer > 0){
                        CPU_PC = _target - 2;
                    }
                }
                if(*argument == ARG_PTR){
                    CPU_PC = CPU_PTR.ptrW;
                }
            } else {
                CPU_Flags[FLAG_ZERO] = 0;
            }
            break;

        /*0011-0101 JMP_C*/
        case 0x35:
            if(CPU_Flags[FLAG_CARRY] == 0){
                if(*argument == ARG_LOOP){
                    unsigned short int _target = peek16(CORE_Loop_stackSpace, &CORE_loopPointer);
                    if(CORE_loopPointer > 0){
                        CPU_PC = _target - 2;
                    }
                }
                if(*argument == ARG_PTR){
                    CPU_PC = CPU_PTR.ptrW;
                }
            } else {
                CPU_Flags[FLAG_CARRY] = 0;
            }
            break;

        /*0011-0110 JMP_N*/
        case 0x36:
            if(CPU_Flags[FLAG_NEG] == 0){
                if(*argument == ARG_LOOP){
                    unsigned short int _target = peek16(CORE_Loop_stackSpace, &CORE_loopPointer);
                    if(CORE_loopPointer > 0){
                        CPU_PC = _target - 2;
                    }
                }
                if(*argument == ARG_PTR){
                    CPU_PC = CPU_PTR.ptrW;
                }
            } else {
                CPU_Flags[FLAG_NEG] = 0;
            }
            break;

        /*0011-0111 CALL*/
        case 0x37:
            push16(CORE_Fun_stackSpace, &CORE_funPointer, CPU_PC+2);
            CPU_PC = CPU_PTR.ptrW - 2;
            break;

        /*0011-1000 RET*/
        case 0x38: {
            unsigned short int addr = pop16(CORE_Fun_stackSpace, &CORE_funPointer);
            if(addr > 0){
                CPU_PC = addr - 2;
            }
            break;
        }
        /* ----- I/O ----- */
        /*0100-0001 cout command out*/
        case 0x41:
            outBuffer = *argument;
            break;
        
        /*0100-0010 dout data out*/
        case 0x42:
            outBuffer = CPU_R[*argument & 0x0F];
            break;

        /* ----- Specialne ----- */
        /*0000-0000 HALT*/
        case 0x00:
            CORE_running = 0;
            break;
        /*NOP-ERROR*/
        default:
            break;
    }
}

int main(void){

    CORE_ramSpace[0] = 0x11;    CORE_ramSpace[1] = 0x01;    // set r1
    CORE_ramSpace[2] = 0x12;    CORE_ramSpace[3] = 0x05;    // mov 5

    CORE_ramSpace[4] = 0x31;    CORE_ramSpace[5] = 0x00;    // setloop
    CORE_ramSpace[6] = 0x25;    CORE_ramSpace[7] = 0x01;    // dec r1
    CORE_ramSpace[8] = 0x34;    CORE_ramSpace[9] = 0x81;    // jmp_z loop
    CORE_ramSpace[10] = 0x32;   CORE_ramSpace[11] = 0x00;   // endloop

    CORE_ramSpace[12] = 0x11;   CORE_ramSpace[13] = 0x01;   // set r1
    CORE_ramSpace[14] = 0x12;   CORE_ramSpace[15] = 0x05;   // mov 5
    CORE_ramSpace[16] = 0x31;   CORE_ramSpace[17] = 0x00;   // setLoop
    CORE_ramSpace[18] = 0x25;   CORE_ramSpace[19] = 0x01;   // dec r1
    CORE_ramSpace[20] = 0x34;   CORE_ramSpace[21] = 0x81;   // jmp_z loop
    CORE_ramSpace[22] = 0x32;   CORE_ramSpace[23] = 0x00;   // endLoop

    CORE_ramSpace[24] = 0x00;   CORE_ramSpace[25] = 0x00;   // halt

    while(CORE_running == 1){
        if(CPU_PC % 4 == 0){
            printf("REJESTR %d\n", CPU_R[1]);
            printf("LP0 %d\n", CORE_Loop_stackSpace[0]);
            printf("LP1 %d\n", CORE_Loop_stackSpace[1]);
            printf("FLAGA %d\n", CPU_Flags[FLAG_ZERO]);
        }
        fetch(&CORE_ramSpace[CPU_PC], &CORE_ramSpace[CPU_PC + 1]);
        CPU_PC = CPU_PC + 2;
    }
    return 0;
}