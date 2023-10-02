#ifndef _6502_H_
#define _6502_H_

#include <stdio.h>
#include <stdlib.h>

#define MAX_MEM 1024*64 // 64KB

#define STATUS_CARRY_FLAG   0x01
#define STATUS_ZERO_FLAG    0x02
#define STATUS_INTERRUPT_DISABLE_FLAG 0x04
#define STATUS_DECIMAL_MODE_FLAG 0x08
#define STATUS_BREAK_FLAG   0x10
#define STATUS_UNUSED_FLAG  0x20
#define STATUS_OVERFLOW_FLAG    0x40
#define STATUS_NEGATIVE_FLAG    0x80

typedef unsigned char byte;
typedef unsigned short word;
typedef unsigned int u32;


#define INS_LDA_IM 0xa9

struct CPU 
{
    word PC; // program counter
    word SP; // stack pointer
    
    byte A, X, Y; // registers

    byte status; // status flags
};

struct Memory
{
    byte data[MAX_MEM];
};

void initialise_mem(struct Memory *mem)
{
    for (u32 i = 0; i < MAX_MEM; i++)
    {
        mem->data[i] = 0;
    }
}

void reset(struct CPU *cpu, struct Memory *mem)
{
    cpu->PC = 0xfffc;
    cpu->SP = 0x0100;
    cpu->status = 0x0;
    cpu->A = cpu->X = cpu->Y = 0;
    initialise_mem(mem);
}

void tick(u32 *cycles) 
{
    (*cycles)--;
}

byte fetch(u32 *cycles, struct CPU *cpu, struct Memory *mem)
{
	byte data = mem->data[cpu->PC++];
    tick(cycles);
    return data;
}

void execute(u32 cycles, struct CPU *cpu, struct Memory *mem)
{
   while (cycles > 0)
   {
	   byte instruction = fetch(&cycles, cpu, mem);
       switch (instruction)
       {
            case INS_LDA_IM:
            {
                byte value = fetch(&cycles, cpu, mem);
                cpu->A = value;
                cpu->status = ((cpu->A == 0) ? STATUS_ZERO_FLAG : 0) | cpu->status;
                cpu->status = ((cpu->A & STATUS_NEGATIVE_FLAG > 0) ? STATUS_NEGATIVE_FLAG : 0) | cpu->status;
            } break;
            
            default:
            {
                printf("Instruction not handled: 0x%x\n", instruction);
            } break;
        }
   }
}

#endif // _6502_H_
