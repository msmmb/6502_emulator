#ifndef _6502_H_
#define _6502_H_

#include <stdio.h>
#include <stdlib.h>
#include "ins.h"


// MEMORY SIZE
#define MAX_MEM 1024*64 // 64KB

// FLAG MASKS
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


struct CPU 
{
    word PC; // program counter
    word SP; // stack pointer
    byte SR; // status flags (NV_BDIZC)
    
    byte A, X, Y; // registers

	byte memory[MAX_MEM];
};


void initialise_mem(struct CPU *cpu)
{
    for (u32 i = 0; i < MAX_MEM; i++)
    {
        cpu->memory[i] = 0;
    }
}


void reset(struct CPU *cpu)
{
    cpu->PC = 0xfffc;
    cpu->SP = 0x0100;
    cpu->SR = 0x0 | STATUS_UNUSED_FLAG;
    cpu->A = cpu->X = cpu->Y = 0;
    initialise_mem(cpu);
}


void tick(u32 *cycles) 
{
    (*cycles)--;
}


byte fetch_byte(u32 *cycles, struct CPU *cpu)
{
	byte data = cpu->memory[cpu->PC++];
    tick(cycles);
    return data;
}


byte load_register_zp(u32 *cycles, struct CPU *cpu, byte address)
{
	byte value = cpu->memory[address];
	tick(cycles);
    return value;
}


void set_zero_and_negative_flags(struct CPU *cpu, byte reg)
{
	cpu->SR = ((reg == 0) ? STATUS_ZERO_FLAG : 0) | cpu->SR;
	cpu->SR = ((reg & STATUS_NEGATIVE_FLAG > 0) ? STATUS_NEGATIVE_FLAG : 0) | cpu->SR;
}


void execute(u32 cycles, struct CPU *cpu)
{
   while (cycles > 0)
   {
	   byte instruction = fetch_byte(&cycles, cpu);
       switch (instruction)
       {
            case INS_LDA_IM:
            {
                byte value = fetch_byte(&cycles, cpu);
                cpu->A = value;
                set_zero_and_negative_flags(cpu, cpu->A);
            } break;

            case INS_LDA_ZP:
            {
                byte address = fetch_byte(&cycles, cpu);
                cpu->A = load_register_zp(&cycles, cpu, address);
                set_zero_and_negative_flags(cpu, cpu->A);
            } break;

            default:
            {
                printf("Unknown instruction: 0x%x\n", instruction);
            } break;
        }
   }
}

#endif // _6502_H_
