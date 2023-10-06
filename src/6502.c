#include <stdio.h>
#include <stdlib.h>
#include "../inc/6502.h"


void initialise_mem(struct CPU *cpu)
{
    for (u32 i = 0; i < 1024*64; i++)
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
    cpu->cycles = 0;
    initialise_mem(cpu);
}
		
void tick_clock(struct CPU *cpu)
{
    cpu->cycles--;
}


byte fetch_byte(struct CPU *cpu)
{
	byte data = cpu->memory[cpu->PC++];
    tick_clock(cpu);
    return data;
}


word fetch_word(struct CPU *cpu)
{
	word data = cpu->memory[cpu->PC++];
    tick_clock(cpu);
    data |= (cpu->memory[cpu->PC++]) << 8;
    tick_clock(cpu);
    return data;
}


byte am_immeadiate(struct CPU *cpu)
{
    byte address = cpu->PC++;
    tick_clock(cpu);
    return address;
}


byte am_zero_page(struct CPU *cpu)
{
    byte address = fetch_byte(cpu);
    tick_clock(cpu);
    return address;
}


word am_absolute(struct CPU *cpu)
{
    word address = fetch_word(cpu);
    tick_clock(cpu);
    return address;
}


void set_zero_and_negative_flags(struct CPU *cpu, byte reg)
{
	cpu->SR |= ((reg == 0) ? STATUS_ZERO_FLAG : 0);
	cpu->SR |= ((reg & STATUS_NEGATIVE_FLAG > 0) ? STATUS_NEGATIVE_FLAG : 0);
}


void execute(u32 cycles, struct CPU *cpu)
{
    cpu->cycles = cycles;
    while (cpu->cycles > 0)
    {
       byte instruction = fetch_byte(cpu);
       switch (instruction)
       {
            case INS_ADC:
            {

            } break;

            case INS_ADC_ZP:
            {

            } break;

            case INS_ADC_ZPX:
            {

            } break;

            case INS_ADC_ABS:
            {

            } break;

            case INS_ADC_ABSX:
            {

            } break;

            case INS_ADC_ABSY:
            {

            } break;

            case INS_ADC_INDX:
            {

            } break;

            case INS_ADC_INDY:
            {

            } break;

            case INS_AND_IM:
            {

            } break;

            case INS_AND_ZP:
            {

            } break;

            case INS_AND_ZPX:
            {

            } break;

            case INS_AND_ABS:
            {

            } break;

            case INS_AND_ABSX:
            {

            } break;

            case INS_AND_ABSY:
            {

            } break;

            case INS_AND_INDX:
            {

            } break;

            case INS_AND_INDY:
            {

            } break;

            case INS_ASL:
            {

            } break;

            case INS_ASL_ZP:
            {

            } break;

            case INS_ASL_ZPX:
            {

            } break;

            case INS_ASL_ABS:
            {

            } break;

            case INS_ASL_ABSX:
            {

            } break;

            case INS_BPL:
            {

            } break;

            case INS_BMI:
            {

            } break;

            case INS_BVC:
            {

            } break;

            case INS_BVS:
            {

            } break;

            case INS_BCC:
            {

            } break;

            case INS_BCS:
            {

            } break;

            case INS_BNE:
            {

            } break;

            case INS_BEQ:
            {

            } break;

            case INS_BIT_ZP:
            {

            } break;

            case INS_BIT_ABS:
            {

            } break;

            case INS_BRK:
            {

            } break;

            case INS_CLC:
            {

            } break;

            case INS_SEC:
            {

            } break;

            case INS_CLI:
            {

            } break;

            case INS_SEI:
            {

            } break;

            case INS_CLV:
            {

            } break;

            case INS_CLD:
            {

            } break;

            case INS_SED:
            {

            } break;

            case INS_CMP:
            {

            } break;

            case INS_CMP_ZP:
            {

            } break;

            case INS_CMP_ZPX:
            {

            } break;

            case INS_CMP_ABS:
            {

            } break;

            case INS_CMP_ABSX:
            {

            } break;

            case INS_CMP_ABSY:
            {

            } break;

            case INS_CMP_INDX:
            {

            } break;

            case INS_CMP_INDY:
            {

            } break;

            case INS_CPX:
            {

            } break;

            case INS_CPX_ZP:
            {

            } break;

            case INS_CPX_ABS:
            {

            } break;

            case INS_CPY:
            {

            } break;

            case INS_CPY_ZP:
            {

            } break;

            case INS_CPY_ABS:
            {

            } break;

            case INS_DEC_ZP:
            {

            } break;

            case INS_DEC_ZPX:
            {

            } break;

            case INS_DEC_ABS:
            {

            } break;

            case INS_DEC_ABSX:
            {

            } break;

            case INS_TAX:
            {

            } break;

            case INS_TXA:
            {

            } break;

            case INS_DEX:
            {

            } break;

            case INS_INX:
            {

            } break;

            case INS_TAY:
            {

            } break;

            case INS_TYA:
            {

            } break;

            case INS_DEY:
            {

            } break;

            case INS_INY:
            {

            } break;

            case INS_EOR_IM:
            {

            } break;

            case INS_EOR_ZP:
            {

            } break;

            case INS_EOR_ZPX:
            {

            } break;

            case INS_EOR_ABS:
            {

            } break;

            case INS_EOR_ABSX:
            {

            } break;

            case INS_EOR_ABSY:
            {

            } break;

            case INS_EOR_INDX:
            {

            } break;

            case INS_EOR_INDY:
            {

            } break;

            case INS_INC_ZP:
            {

            } break;

            case INS_INC_ZPX:
            {

            } break;

            case INS_INC_ABS:
            {

            } break;

            case INS_INC_ABSX:
            {

            } break;

            case INS_JMP_ABS:
            {

            } break;

            case INS_JMP_IND:
            {

            } break;

            case INS_JSR:
            {

            } break;

            case INS_LDA_IM:
            {
                cpu->A = cpu->memory[am_immeadiate(cpu)];
                set_zero_and_negative_flags(cpu, cpu->A);
            } break;

            case INS_LDA_ZP:
            {
                cpu->A = cpu->memory[am_zero_page(cpu)];
                set_zero_and_negative_flags(cpu, cpu->A);
            } break;

            case INS_LDA_ZPX:
            {

            } break;

            case INS_LDA_ABS:
            {

            } break;

            case INS_LDA_ABSX:
            {

            } break;

            case INS_LDA_ABSY:
            {

            } break;

            case INS_LDA_INDX:
            {

            } break;

            case INS_LDA_INDY:
            {

            } break;

            case INS_LDX_IM:
            {

            } break;

            case INS_LDX_ZP:
            {

            } break;

            case INS_LDX_ZPY:
            {

            } break;

            case INS_LDX_ABS:
            {

            } break;

            case INS_LDX_ABSY:
            {

            } break;

            case INS_LDY_IM:
            {

            } break;

            case INS_LDY_ZP:
            {

            } break;

            case INS_LDY_ZPX:
            {

            } break;

            case INS_LDY_ABS:
            {

            } break;

            case INS_LDY_ABSX:
            {

            } break;

            case INS_LSR:
            {

            } break;

            case INS_LSR_ZP:
            {

            } break;

            case INS_LSR_ZPX:
            {

            } break;

            case INS_LSR_ABS:
            {

            } break;

            case INS_LSR_ABSX:
            {

            } break;

            case INS_NOP:
            {

            } break;

            case INS_ORA_IM:
            {

            } break;

            case INS_ORA_ZP:
            {

            } break;

            case INS_ORA_ZPX:
            {

            } break;

            case INS_ORA_ABS:
            {

            } break;

            case INS_ORA_ABSX:
            {

            } break;

            case INS_ORA_ABSY:
            {

            } break;

            case INS_ORA_INDX:
            {

            } break;

            case INS_ORA_INDY:
            {

            } break;

            case INS_STA_ZP:
            {

            } break;

            case INS_STA_ZPX:
            {

            } break;

            case INS_STA_ABS:
            {

            } break;

            case INS_STA_ABSX:
            {

            } break;

            case INS_STA_ABSY:
            {

            } break;

            case INS_STA_INDX:
            {

            } break;

            case INS_STA_INDY:
            {

            } break;

            case INS_TXS:
            {

            } break;

            case INS_TSX:
            {

            } break;

            case INS_PHA:
            {

            } break;

            case INS_PLA:
            {

            } break;

            case INS_PHP:
            {

            } break;

            case INS_PLP:
            {

            } break;

            case INS_ROL:
            {

            } break;

            case INS_ROL_ZP:
            {

            } break;

            case INS_ROL_ZPX:
            {

            } break;

            case INS_ROL_ABS:
            {

            } break;

            case INS_ROL_ABSX:
            {

            } break;

            case INS_ROR:
            {

            } break;

            case INS_ROR_ZP:
            {

            } break;

            case INS_ROR_ZPX:
            {

            } break;

            case INS_ROR_ABS:
            {

            } break;

            case INS_ROR_ABSX:
            {

            } break;

            case INS_RTI:
            {

            } break;

            case INS_RTS:
            {

            } break;

            case INS_SBC:
            {

            } break;

            case INS_SBC_ZP:
            {

            } break;

            case INS_SBC_ZPX:
            {

            } break;

            case INS_SBC_ABS:
            {

            } break;

            case INS_SBC_ABSX:
            {

            } break;

            case INS_SBC_ABSY:
            {

            } break;

            case INS_SBC_INDX:
            {

            } break;

            case INS_SBC_INDY:
            {

            } break;

            case INS_STX_ZP:
            {

            } break;

            case INS_STX_ZPY:
            {

            } break;

            case INS_STX_ABS:
            {

            } break;

            case INS_STY_ZP:
            {

            } break;

            case INS_STY_ZPX:
            {

            } break;

            case INS_STY_ABS:
            {

            } break;

            default:
            {
                printf("Unknown instruction: 0x%x\n", instruction);
            } break;
        }
    }
}
