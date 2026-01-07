#include <stdio.h>
#include <string.h>
#include <stdint.h>

#include "filehandler.h"

#define PUSH        1
#define POP         2
#define Peek8Low    3
#define Peek8High   4

#define FLAG_ZERO   0
#define FLAG_CARRY  1
#define FLAG_NEG    2

#define STACK_SIZE  254
#define RAM_SIZE    65536

#define ARG_LOOP    0x81
#define ARG_PTR     0x82

#define TERMINAL_SCREEN_ADDR    0xFA00
#define TERMINAL_SCREEN_HEIGHT  30
#define TERMINAL_SCREEN_WIDTH   40


#define HIGH_NIBBLE(byte)   (((byte) >> 4) & 0x0F)
#define LOW_NIBBLE(byte)    ((byte) & 0x0F)
#define ARGUMENT_DEST(byte) HIGH_NIBBLE(byte)
#define ARGUMENT_SRC(byte)  LOW_NIBBLE(byte)

uint8_t ramSpace[RAM_SIZE];

typedef union {
    struct {
        uint8_t lowByte;
        uint8_t highByte;
    };
    uint16_t word;
} Word;

typedef struct {
    Word PC;
    Word PTR;

    uint8_t FNStackPTR;
    uint8_t LPStackPTR;
    uint8_t RGStackPTR;

    uint8_t functionStack[STACK_SIZE];
    uint8_t loopStack[STACK_SIZE];
    uint8_t regDataStack[STACK_SIZE];
    
    uint8_t registers[16];
    uint8_t flags[3];
    uint8_t activeRegister;
    
    uint8_t interruptSignal;
    uint8_t busIn;
    uint8_t running;
} CPU;

uint8_t stack(
    uint8_t operationType,
    uint8_t *table,
    uint8_t *ptr,
    uint8_t data,
    uint8_t *result
){
    /*  1 - Push
        2 - Pop
        3 - Peek8lowBits
        4 - Peek8HighBits  */
    switch(operationType){
        case 1:
            if(*ptr == 0xFF){
                // STOP CPU RUNNING
                return 0;
            }
            table[*ptr] = data;
            (*ptr)++;
            return 1;

        case 2:
            if(*ptr > 0x00){
                (*ptr)--;
                *result = table[*ptr];
                return 1;
            }
            // STOP CPU RUNNING
            return 0;

        case 3:
            if(*ptr > 0x00){
                *result = table[*ptr - 1];
                return 1;
            }
            // STOP CPU RUNNING
            return 0;

        case 4:
            if(*ptr > 0x01){
                *result = table[*ptr - 2];
                return 1;
            }
            // STOP CPU RUNNING
            return 0;

        default:
            // STOP CPU RUNNING
            return 0;
    }
}

void checkInterrupt(){
    // TO DO
}

void fetch(CPU *core, uint8_t *opcode, uint8_t *argument){
    switch(*opcode){
        /* ----- DANE -----*/
        /*0001-0001 SET*/
        case 0x11:
            core->activeRegister = *argument & 0x0F;
            break;

        /*0001-0010 MOV*/
        case 0x12:
            core->registers[core->activeRegister] = *argument;
            break;

        /*0001-0011 CPY*/
        case 0x13: {
            uint8_t _dest = ARGUMENT_DEST(*argument);
            uint8_t _src = ARGUMENT_SRC(*argument);

            core->registers[_dest] = core->registers[_src];
            break;
        }
        /*0001-0100 PTR*/
        case 0x14: {
            uint8_t _high = ARGUMENT_DEST(*argument);
            uint8_t _low = ARGUMENT_SRC(*argument);

            core->PTR.highByte = core->registers[_high];
            core->PTR.lowByte = core->registers[_low];
            break;
        }
        /*0001-0101 PTR_H*/
        case 0x15:
            core->PTR.highByte = *argument;
            break;

        /*0001-0110 PTR_L*/
        case 0x16:
            core->PTR.lowByte = *argument;
            break;

        /*0001-0111 LOAD*/
        case 0x17:
            core->registers[*argument & 0x0F] = ramSpace[core->PTR.word];
            break;

        /*0001-1000 STORE*/
        case 0x18:
            ramSpace[core->PTR.word] = core->registers[*argument & 0x0F];
            break;

        /*0001-1001 PUSH*/
        case 0x19: {
            uint8_t __result;
            core->running = stack(
                PUSH, 
                core->regDataStack, 
                &core->RGStackPTR, 
                core->registers[*argument & 0x0F], 
                &__result
            );
            break;
        }

        /*0001-1010 POP*/
        case 0x1A: {
            uint8_t __data;
            core->running = stack(
                POP,
                core->regDataStack,
                &core->RGStackPTR,
                __data,
                &core->registers[*argument & 0x0F]
            );
            break;
        }

        /* ----- ARYTMERYKA ----- */
        /*0010-0001 ADD*/
        case 0x21: {
            uint8_t dest = ARGUMENT_DEST(*argument);
            uint8_t src = ARGUMENT_SRC(*argument);
            uint16_t result = (uint8_t)core->registers[dest] + (uint8_t)core->registers[src];
            core->registers[dest] = (uint8_t)(result & 0xFF);

            if(result > 0xFF){
                core->flags[FLAG_CARRY] = 0x01;
            } else {
                core->flags[FLAG_CARRY] = 0x00;
            }
            break;
        }

        /*0010-0010 SUB*/
        case 0x22: {
            uint8_t dest = ARGUMENT_DEST(*argument);
            uint8_t src = ARGUMENT_SRC(*argument);
            uint16_t result = (uint8_t)core->registers[dest] - (uint8_t)core->registers[src];
            core->registers[dest] = (uint8_t)(result & 0xFF);

            if(result > core->registers[dest]){
                core->flags[FLAG_NEG] = 0x01;
            } else {
                core->flags[FLAG_NEG] = 0x00;
            }
            break;
        }

        /*0010-0011 CMP*/
        case 0x23: {
            core->flags[FLAG_CARRY] = 0x00;
            core->flags[FLAG_NEG] = 0x00;
            core->flags[FLAG_ZERO] = 0x00;

            uint8_t dest = ARGUMENT_DEST(*argument);
            uint8_t src = ARGUMENT_SRC(*argument);
            int16_t result = (int16_t)core->registers[dest] - (int16_t)core->registers[src];

            if(result == 0){
                core->flags[FLAG_ZERO] = 0x01;
            } else if(result > 0){
                core->flags[FLAG_CARRY] = 0x01;
            } else if(result < 0){
                core->flags[FLAG_NEG] = 0x01;
            }
            break;
        }

        /*0010-0100 INC*/
        case 0x24:
            if(*argument == ARG_PTR){
                core->PTR.word++;
                break;
            }
            core->registers[*argument & 0x0F]++;
            if(core->registers[*argument & 0x0F] == 0){
                core->flags[FLAG_ZERO] = 0x01;
            } else {
                core->flags[FLAG_ZERO] = 0x00;
            }
            break;

        /*0010-0101 DEC*/
        case 0x25:
            if(*argument == ARG_PTR){
                core->PTR.word--;
                break;
            }
            core->registers[*argument & 0x0F]--;
            if(core->registers[*argument & 0x0F] == 0){
                core->flags[FLAG_ZERO] = 0x01;
            } else {
                core->flags[FLAG_ZERO] = 0x00;
            }
            break;
        
        /* ----- Kontrola -----*/
        /*0011-0001 SETLOOP*/
        case 0x31:{
            uint8_t __result;
            uint8_t __status;
            uint8_t __nextInstruction = core->PC.word + 2;

            __status = stack(
                PUSH,
                core->loopStack,
                &core->LPStackPTR,
                (__nextInstruction >> 8) & 0xFF,
                &__result
            );
            if(__status == 0){
                core->running = 0x00;
                break;
            }
            __status = stack(
                PUSH,
                core->loopStack,
                &core->LPStackPTR,
                __nextInstruction & 0xFF,
                &__result
            );
            if(__status == 0){
                core->running = 0x00;
                break;
            }
            break;
        }
        /*0011-0010 ENDLOOP*/
        case 0x32:{
            uint8_t __result;
            uint8_t __status;

            __status = stack(
                POP,
                core->loopStack,
                &core->LPStackPTR,
                0,
                &__result
            );
            if(__status == 0){
                core->running = 0x00;
                break;
            }
            __status = stack(
                POP,
                core->loopStack,
                &core->LPStackPTR,
                0,
                &__result
            );
            if(__status == 0){
                core->running = 0x00;
                break;
            }
            break;
        }
        /*0011-0011 JMP*/
        case 0x33:{
            uint8_t __status;
            //ARG LOOP
            if(*argument == ARG_LOOP){
                __status = stack(
                    Peek8Low,
                    core->loopStack,
                    &core->LPStackPTR,
                    core->PTR.lowByte,
                    &core->PC.lowByte
                );
                if(__status == 0){
                    core->running = 0x00;
                    break;
                }
                __status = stack(
                    Peek8High,
                    core->loopStack,
                    &core->LPStackPTR,
                    core->PTR.highByte,
                    &core->PC.highByte
                );
                if(__status == 0){
                    core->running = 0x00;
                    break;
                }
                core->PC.word = core->PC.word - 2;
            }
            //ARG PTR
            if(*argument == ARG_PTR){
                core->PC.word = core->PTR.word - 2;
                break;
            }
            break;
        }
        /*0011-0100 JMP_Z*/
        case 0x34:
            if(core->flags[FLAG_ZERO] == 0x00){
                //ARG LOOP
                uint8_t __status;
                if(*argument == ARG_LOOP){
                    __status = stack(
                        Peek8Low,
                        core->loopStack,
                        &core->LPStackPTR,
                        core->PTR.lowByte,
                        &core->PC.lowByte
                        );
                    if(__status == 0){
                        core->running = 0x00;
                        break;
                    }
                    __status = stack(
                        Peek8High,
                        core->loopStack,
                        &core->LPStackPTR,
                        core->PTR.highByte,
                        &core->PC.highByte
                        );
                    if(__status == 0){
                        core->running = 0x00;
                        break;
                    }
                    core->PC.word = core->PC.word - 2;
                }
                //ARG PTR
                if(*argument == ARG_PTR){
                    core->PC.word = core->PTR.word - 2;
                    break;
                }
                break;
            }
            break;

        /*0011-0101 JMP_C*/
        case 0x35:
            if(core->flags[FLAG_CARRY] == 0x00){
                //ARG LOOP
                uint8_t __status;
                if(*argument == ARG_LOOP){
                    __status = stack(
                        Peek8Low,
                        core->loopStack,
                        &core->LPStackPTR,
                        core->PTR.lowByte,
                        &core->PC.lowByte
                        );
                    if(__status == 0){
                        core->running = 0x00;
                        break;
                    }
                    __status = stack(
                        Peek8High,
                        core->loopStack,
                        &core->LPStackPTR,
                        core->PTR.highByte,
                        &core->PC.highByte
                        );
                    if(__status == 0){
                        core->running = 0x00;
                        break;
                    }
                    core->PC.word = core->PC.word - 2;
                }
                //ARG PTR
                if(*argument == ARG_PTR){
                    core->PC.word = core->PTR.word - 2;
                    break;
                }
                break;
            } 
            break;

        /*0011-0110 JMP_N*/
        case 0x36:
            if(core->flags[FLAG_NEG] == 0x00){
                //ARG LOOP
                uint8_t __status;
                if(*argument == ARG_LOOP){
                    __status = stack(
                        Peek8Low,
                        core->loopStack,
                        &core->LPStackPTR,
                        core->PTR.lowByte,
                        &core->PC.lowByte
                        );
                    if(__status == 0){
                        core->running = 0x00;
                        break;
                    }
                    __status = stack(
                        Peek8High,
                        core->loopStack,
                        &core->LPStackPTR,
                        core->PTR.highByte,
                        &core->PC.highByte
                        );
                    if(__status == 0){
                        core->running = 0x00;
                        break;
                    }
                    core->PC.word = core->PC.word - 2;
                }
                //ARG PTR
                if(*argument == ARG_PTR){
                    core->PC.word = core->PTR.word - 2;
                    break;
                }
                break;
            }
            break;

        /*0011-0111 CALL*/
        case 0x37:{
            uint8_t __result;
            uint8_t __status;
            core->PC.word = core->PC.word+2;
            __status = stack(
                PUSH,
                core->functionStack,
                &core->FNStackPTR,
                core->PC.highByte,
                &__result
            );
            if(__status == 0){
                core->running = 0x00;
                break;
            }
            __status = stack(
                PUSH,
                core->functionStack,
                &core->FNStackPTR,
                core->PC.lowByte,
                &__result
            );
            if(__status == 0){
                core->running = 0x00;
                break;
            }
            core->PC.word = core->PTR.word - 2;
            break;
        }

        /*0011-1000 RET*/
        case 0x38:{
            uint8_t __result;
            uint8_t __status;

            __status = stack(
                POP,
                core->functionStack,
                &core->FNStackPTR,
                0,
                &core->PC.lowByte
            );
            if(__status == 0){
                core->running = 0x00;
                break;
            }
            __status = stack(
                POP,
                core->functionStack,
                &core->FNStackPTR,
                0,
                &core->PC.highByte
            );
            if(__status == 0){
                core->running = 0x00;
                break;
            }
            core->PC.word = core->PC.word - 2;
            break;
        }

        /* ----- Specialne -----*/
        /*0000-0000 HALT*/
        case 0x00:
            core->running = 0x00;
            break;
        /*NOP-ERROR*/
        default:
            break;
    }
}

void nicePrint(CPU *cpu, uint8_t *operation, uint8_t *argument, uint8_t debug){
    //printf("\033[2J");
    printf("===============================================\n");
    printf("PC:%04X   | OP:%02X %02X   | PTR:%04X |\n", cpu->PC.word, *operation, *argument, cpu->PTR.word);
    printf("FN:%02X     | LP:%02X      | RG:%02X    |\n", cpu->FNStackPTR, cpu->LPStackPTR, cpu->RGStackPTR);
    printf("R0:%02X     | R1:%02X      | R2:%02X    | R3:%02X\n", cpu->registers[0], cpu->registers[1], cpu->registers[2], cpu->registers[3]);
    printf("R4:%02X     | R5:%02X      | R6:%02X    | R7:%02X\n", cpu->registers[4], cpu->registers[5], cpu->registers[6], cpu->registers[7]);
    printf("R8:%02X     | R9:%02X      | R10:%02X   | R11:%02X\n", cpu->registers[8], cpu->registers[9], cpu->registers[10], cpu->registers[11]);
    printf("R12:%02X    | R13:%02X     | R14:%02X   | R15:%02X\n", cpu->registers[12], cpu->registers[13], cpu->registers[14], cpu->registers[15]);
    printf("F_ZERO:%02X | F_CARRY:%02X | F_NEG:%02X | ACT_REG: %02X\n", cpu->flags[FLAG_ZERO], cpu->flags[FLAG_CARRY], cpu->flags[FLAG_NEG], cpu->activeRegister);
    printf("\n");
    for(uint16_t i=0x0000;i<0xFFFF;i++){
        if(i % 8 == 0){
            printf("%04X: %02X %02X %02X %02X %02X %02X %02X %02X\n", i, ramSpace[i],ramSpace[i+1],ramSpace[i+2],ramSpace[i+3],ramSpace[i+4],ramSpace[i+5],ramSpace[i+6],ramSpace[i+7]);
            if(ramSpace[i] == 0x00){
                break;
            }
        }
    }
    if(debug == 1){
        getchar();
    }
}

int main(int argc, char *argv[]){

    if(argc == 2){
        loadFile(argv[1], ramSpace, RAM_SIZE-1);
    } else {
        printf("Try: ./program <bin file>\n");
        return 1;
    }

    CPU cpuCore;
    memset(&cpuCore, 0, sizeof(CPU));
    cpuCore.running = 1;

    while(cpuCore.running == 1){
        if(cpuCore.PC.word % 2 == 0){
            //nicePrint(&cpuCore, &ramSpace[cpuCore.PC.word], &ramSpace[cpuCore.PC.word + 1], 0);
        }
        fetch(&cpuCore, &ramSpace[cpuCore.PC.word], &ramSpace[cpuCore.PC.word + 1]);
        cpuCore.PC.word = cpuCore.PC.word + 2;
    }
    nicePrint(&cpuCore, &ramSpace[cpuCore.PC.word], &ramSpace[cpuCore.PC.word + 1], 0);
    return 0;
}
