#include <stdio.h>
#include <stdlib.h>
#include "6502.h"


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
    cpu->SP = 0xff;
    cpu->SR = 0x0 | STATUS_UNUSED_FLAG;
    cpu->A = cpu->X = cpu->Y = 0;
    cpu->cycles = 0;
    initialise_mem(cpu);
}
		

byte fetch_byte(struct CPU *cpu)
{
	byte data = read_byte(cpu, cpu->PC++);
    return data;
}


word fetch_word(struct CPU *cpu)
{
	word data = read_word(cpu, cpu->PC++);
    return data;
}


byte read_byte(struct CPU *cpu, word address)
{
	byte data = cpu->memory[address];
	cpu->cycles--;
	return data;
}


word read_word(struct CPU *cpu, word address)
{
	byte low_byte = read_byte(cpu, address++);
	byte high_byte = read_byte(cpu, address);
	return low_byte | (high_byte << 8);
}


void write_byte(struct CPU *cpu, byte value, word address)
{
	cpu->memory[address] = value;
	cpu->cycles--;
}


void write_word(struct CPU *cpu, word value, word address)
{
	cpu->memory[address] = value & 0xFF; 
	cpu->memory[address+1] = (value >> 8);
	cpu->cycles -= 2;
}


word am_immediate(struct CPU *cpu)
{
    byte address = cpu->PC++;
    return address;
}


word am_zero_page(struct CPU *cpu)
{
    byte address = fetch_byte(cpu);
    return address;
}


word am_zero_page_x(struct CPU *cpu)
{
    byte address = fetch_byte(cpu);
    address += cpu->X;
    cpu->cycles--;
    return address;
}


word am_zero_page_y(struct CPU *cpu)
{
    byte address = fetch_byte(cpu);
    address += cpu->Y;
    cpu->cycles--;
    return address;
}


word am_absolute(struct CPU *cpu)
{
    word address = fetch_word(cpu);
    return address;
}


word am_absolute_x(struct CPU *cpu, int add_cycle)
{
    word address = fetch_word(cpu);
    word address_x = address + cpu->X;
	int page_crossed = (address & 0xFF00) != (address_x & 0xFF00);
    if (page_crossed & add_cycle) cpu->cycles--; 
    return address_x;
}


word am_absolute_y(struct CPU *cpu, int add_cycle)
{
    word address = fetch_word(cpu);
    word address_y = address + cpu->Y;
	int page_crossed = (address & 0xFF00) != (address_y & 0xFF00);
    if (page_crossed & add_cycle) cpu->cycles--; 
    return address_y;
}


word am_indirect_x(struct CPU *cpu)
{
    byte address = fetch_byte(cpu);
    word address_x = address + cpu->X;
    cpu->cycles--;
    word table_address = read_word(cpu, address_x);
    return table_address;    
}


word am_indirect_y(struct CPU *cpu, int add_cycle)
{
    byte address = fetch_byte(cpu);
    word address_y = address + cpu->Y;
    word table_address = read_word(cpu, address_y);
	int page_crossed = (address & 0xFF00) != (address_y & 0xFF00);
    if (page_crossed & add_cycle) cpu->cycles--; 
    return table_address;  
}


void set_ADC_flags(struct CPU *cpu, byte operand, byte result)
{
    cpu->SR |= ((cpu->A > 0xFF) ? STATUS_CARRY_FLAG : 0);
    cpu->SR |= ((cpu->A == 0) ? STATUS_ZERO_FLAG : 0);
    cpu->SR |= (((cpu->A ^ operand) & 0x80) && ((cpu->A ^ result) & 0x80)) ? STATUS_OVERFLOW_FLAG : 0;
    cpu->SR |= ((cpu->A & 0x80) ? STATUS_NEGATIVE_FLAG : 0);
}


void set_flags_ZN(struct CPU *cpu, byte value)
{
    cpu->SR |= (value == 0) ? STATUS_ZERO_FLAG : 0;
    cpu->SR |= (value & 0x80) ? STATUS_NEGATIVE_FLAG : 0;
}


void set_ASL_flags(struct CPU *cpu, word operand, word result)
{
	cpu->SR |= (operand & 0x80) ? STATUS_CARRY_FLAG : 0;
	cpu->SR |= (cpu->A == 0) ? STATUS_ZERO_FLAG : 0;
	cpu->SR |= (result & 0x80) ? STATUS_NEGATIVE_FLAG : 0;
}


void execute_branch(struct CPU *cpu, int condition)
{
	byte rel = fetch_byte(cpu);
	if (condition)
	{
		word old_page = cpu->PC >> 8; 
		cpu->PC += rel;
		cpu->cycles--;
		word new_page = cpu->PC >> 8; 
		if (old_page != new_page) cpu->cycles--; 
	}
}


void set_BIT_flags(struct CPU *cpu, word target)
{
    cpu->SR |= ((target & cpu->A) == 0) ? STATUS_ZERO_FLAG : 0;
	cpu->SR |= ((target & 0x40) != 0) ? STATUS_OVERFLOW_FLAG : 0;
    cpu->SR |= ((target & 0x80) != 0) ? STATUS_NEGATIVE_FLAG : 0;
}


void set_CMP_CPX_CPY_flags(struct CPU *cpu, byte result, byte reg)
{
	cpu->SR |= (reg >= result) ? STATUS_CARRY_FLAG : 0;
	cpu->SR |= (reg == result) ? STATUS_ZERO_FLAG : 0;
	cpu->SR |= (result & 0x80) ? STATUS_NEGATIVE_FLAG : 0;
}


void set_LSR_flags(struct CPU *cpu, byte old_value, byte result)
{
	cpu->SR |= (old_value & 0x01) ? STATUS_CARRY_FLAG : 0;
	cpu->SR |= (result == 0) ? STATUS_ZERO_FLAG : 0;
    cpu->SR |= (result & 0x80) ? STATUS_NEGATIVE_FLAG : 0;
}


void set_SBC_flags(struct CPU *cpu, byte operand, byte result)
{
    cpu->SR |= (cpu->A < 0xFF);
    cpu->SR |= ((cpu->A == 0) ? STATUS_ZERO_FLAG : 0);
    cpu->SR |= (((cpu->A ^ operand) & 0x80) && ((cpu->A ^ result) & 0x80)) ? STATUS_OVERFLOW_FLAG : 0;
    cpu->SR |= ((cpu->A & 0x80) ? STATUS_NEGATIVE_FLAG : 0);
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
				byte operand = read_byte(cpu, am_immediate(cpu));
				byte result = operand + ((cpu->SR & STATUS_CARRY_FLAG) ? 1 : 0);
				set_ADC_flags(cpu, operand, result);
				cpu->A += result;
			} break;

			case INS_ADC_ZP:
			{
				byte operand = read_byte(cpu, am_zero_page(cpu));
				byte result = operand + ((cpu->SR & STATUS_CARRY_FLAG) ? 1 : 0);
				set_ADC_flags(cpu, operand, result);
				cpu->A += result;
			} break;

			case INS_ADC_ZPX:
			{
				byte operand = read_byte(cpu, am_zero_page_x(cpu));
				byte result = operand + ((cpu->SR & STATUS_CARRY_FLAG) ? 1 : 0);
				set_ADC_flags(cpu, operand, result);
				cpu->A += result;
			} break;

			case INS_ADC_ABS:
			{
				byte operand = read_byte(cpu, am_absolute(cpu));
				byte result = operand + ((cpu->SR & STATUS_CARRY_FLAG) ? 1 : 0);
				set_ADC_flags(cpu, operand, result);
				cpu->A += result;
			} break;

			case INS_ADC_ABSX:
			{
				byte operand = read_byte(cpu, am_absolute_x(cpu, 0));
				byte result = operand + ((cpu->SR & STATUS_CARRY_FLAG) ? 1 : 0);
				set_ADC_flags(cpu, operand, result);
				cpu->A += result;
			} break;

			case INS_ADC_ABSY:
			{
				byte operand = read_byte(cpu, am_absolute_y(cpu, 0));
				byte result = operand + ((cpu->SR & STATUS_CARRY_FLAG) ? 1 : 0);
				set_ADC_flags(cpu, operand, result);
				cpu->A += result;
			} break;

			case INS_ADC_INDX:
			{
				byte operand = read_byte(cpu, am_indirect_x(cpu));
				byte result = operand + ((cpu->SR & STATUS_CARRY_FLAG) ? 1 : 0);
				set_ADC_flags(cpu, operand, result);
				cpu->A += result;
			} break;

			case INS_ADC_INDY:
			{
				byte operand = read_byte(cpu, am_indirect_y(cpu, 0));
				byte result = operand + ((cpu->SR & STATUS_CARRY_FLAG) ? 1 : 0);
				set_ADC_flags(cpu, operand, result);
				cpu->A += result;
			} break;

			case INS_AND_IM:
			{
				byte operand = read_byte(cpu, am_immediate(cpu));
				cpu->A &= operand;
				set_flags_ZN(cpu, cpu->A);
			} break;

			case INS_AND_ZP:
			{
				byte operand = read_byte(cpu, am_zero_page(cpu));
				cpu->A &= operand;
				set_flags_ZN(cpu, cpu->A);
			} break;

			case INS_AND_ZPX:
			{
				byte operand = read_byte(cpu, am_zero_page_x(cpu));
				cpu->A &= operand;
				set_flags_ZN(cpu, cpu->A);
			} break;

			case INS_AND_ABS:
			{
				byte operand = read_byte(cpu, am_absolute(cpu));
				cpu->A &= operand;
				set_flags_ZN(cpu, cpu->A);
			} break;

			case INS_AND_ABSX:
			{
				byte operand = read_byte(cpu, am_absolute_x(cpu, 0));
				cpu->A &= operand;
				set_flags_ZN(cpu, cpu->A);
			} break;

			case INS_AND_ABSY:
			{
				byte operand = read_byte(cpu, am_absolute_y(cpu, 0));
				cpu->A &= operand;
				set_flags_ZN(cpu, cpu->A);
			} break;

			case INS_AND_INDX:
			{
				byte operand = read_byte(cpu, am_indirect_x(cpu));
				cpu->A &= operand;
				set_flags_ZN(cpu, cpu->A);
			} break;

			case INS_AND_INDY:
			{
				byte operand = read_byte(cpu, am_indirect_y(cpu, 0));
				cpu->A &= operand;
				set_flags_ZN(cpu, cpu->A);
			} break;

			case INS_ASL:
			{
				word result = cpu->A << 1;
				cpu->cycles--;
				set_ASL_flags(cpu, cpu->A, result);
				cpu->A = result;
			} break;

			case INS_ASL_ZP:
			{
				word address = am_zero_page(cpu);
				byte operand = read_byte(cpu, address);
				byte result = operand << 1;
				cpu->cycles--;
				write_byte(cpu, result, address);
				set_ASL_flags(cpu, operand, result);
			} break;

			case INS_ASL_ZPX:
			{
				word address = am_zero_page_x(cpu);
				byte operand = read_byte(cpu, address);
				byte result = operand << 1;
				cpu->cycles--;
				write_byte(cpu, result, address);
				set_ASL_flags(cpu, operand, result);
			} break;

			case INS_ASL_ABS:
			{
				word address = am_absolute(cpu);
				byte operand = read_byte(cpu, address);
				byte result = operand << 1;
				cpu->cycles--;
				write_byte(cpu, result, address);
				set_ASL_flags(cpu, operand, result);
			} break;

			case INS_ASL_ABSX:
			{
				word address = am_absolute_x(cpu, 0);
				byte operand = read_byte(cpu, address);
				byte result = operand << 1;
				cpu->cycles--;
				write_byte(cpu, result, address);
				set_ASL_flags(cpu, operand, result);
			} break;

			case INS_BPL:
			{
				execute_branch(cpu, !(cpu->SR & STATUS_NEGATIVE_FLAG));
			} break;

			case INS_BMI:
			{
				execute_branch(cpu, cpu->SR & STATUS_NEGATIVE_FLAG);
			} break;

			case INS_BVC:
			{
				execute_branch(cpu, !(cpu->SR & STATUS_OVERFLOW_FLAG));
			} break;

			case INS_BVS:
			{
				execute_branch(cpu, cpu->SR & STATUS_OVERFLOW_FLAG);
			} break;

			case INS_BCC:
			{
				execute_branch(cpu, !(cpu->SR & STATUS_CARRY_FLAG));
			} break;

			case INS_BCS:
			{
				execute_branch(cpu, cpu->SR & STATUS_CARRY_FLAG);
			} break;

			case INS_BNE:
			{
				execute_branch(cpu, !(cpu->SR & STATUS_ZERO_FLAG));
			} break;

			case INS_BEQ:
			{
				execute_branch(cpu, cpu->SR & STATUS_ZERO_FLAG);
			} break;

			case INS_BIT_ZP:
			{
				word target = read_byte(cpu, am_zero_page(cpu));
				set_BIT_flags(cpu, target);
			} break;

			case INS_BIT_ABS:
			{
				word target = read_byte(cpu, am_absolute(cpu));
				set_BIT_flags(cpu, target);
			} break;

			case INS_BRK:
			{
				write_word(cpu, cpu->PC, cpu->SP);
				cpu->SP -= 2;
				write_byte(cpu, cpu->SR, cpu->SP--);
				cpu->cycles--;
				word irq_vector = read_byte(cpu, 0xFFFE);
				cpu->PC = irq_vector;
				cpu->SR |= STATUS_BREAK_FLAG;
			} break;

			case INS_CLC:
			{
				cpu->SR &= ~STATUS_CARRY_FLAG;
			} break;

			case INS_SEC:
			{
				cpu->SR |= STATUS_CARRY_FLAG;
			} break;

			case INS_CLI:
			{
				cpu->SR &= ~STATUS_INTERRUPT_DISABLE_FLAG;
			} break;

			case INS_SEI:
			{
				cpu->SR |= STATUS_INTERRUPT_DISABLE_FLAG;
			} break;

			case INS_CLV:
			{
				cpu->SR &= ~STATUS_OVERFLOW_FLAG;
			} break;

			case INS_CLD:
			{
				cpu->SR &= ~STATUS_DECIMAL_MODE_FLAG;
			} break;

			case INS_SED:
			{
				cpu->SR |= STATUS_DECIMAL_MODE_FLAG;
			} break;

			case INS_CMP:
			{
				byte operand = read_byte(cpu, am_immediate(cpu));
				set_CMP_CPX_CPY_flags(cpu, operand, cpu->A);
			} break;

			case INS_CMP_ZP:
			{
				byte operand = read_byte(cpu, am_zero_page(cpu));
				set_CMP_CPX_CPY_flags(cpu, operand, cpu->A);
			} break;

			case INS_CMP_ZPX:
			{
				byte operand = read_byte(cpu, am_zero_page_x(cpu));
				set_CMP_CPX_CPY_flags(cpu, operand, cpu->A);
			} break;

			case INS_CMP_ABS:
			{
				byte operand = read_byte(cpu, am_absolute(cpu));
				set_CMP_CPX_CPY_flags(cpu, operand, cpu->A);
			} break;

			case INS_CMP_ABSX:
			{
				byte operand = read_byte(cpu, am_absolute_x(cpu, 0));
				set_CMP_CPX_CPY_flags(cpu, operand, cpu->A);
			} break;

			case INS_CMP_ABSY:
			{
				byte operand = read_byte(cpu, am_absolute_y(cpu, 0));
				set_CMP_CPX_CPY_flags(cpu, operand, cpu->A);
			} break;

			case INS_CMP_INDX:
			{
				byte operand = read_byte(cpu, am_indirect_x(cpu));
				set_CMP_CPX_CPY_flags(cpu, operand, cpu->A);
			} break;

			case INS_CMP_INDY:
			{
				byte operand = read_byte(cpu, am_indirect_y(cpu, 0));
				set_CMP_CPX_CPY_flags(cpu, operand, cpu->A);
			} break;

			case INS_CPX:
			{
				byte operand = read_byte(cpu, am_immediate(cpu));
				set_CMP_CPX_CPY_flags(cpu, operand, cpu->X);
			} break;

			case INS_CPX_ZP:
			{
				byte operand = read_byte(cpu, am_zero_page(cpu));
				set_CMP_CPX_CPY_flags(cpu, operand, cpu->X);
			} break;

			case INS_CPX_ABS:
			{
				byte operand = read_byte(cpu, am_absolute(cpu));
				set_CMP_CPX_CPY_flags(cpu, operand, cpu->X);
			} break;

			case INS_CPY:
			{
				byte operand = read_byte(cpu, am_immediate(cpu));
				set_CMP_CPX_CPY_flags(cpu, operand, cpu->Y);
			} break;

			case INS_CPY_ZP:
			{
				byte operand = read_byte(cpu, am_zero_page(cpu));
				set_CMP_CPX_CPY_flags(cpu, operand, cpu->Y);
			} break;

			case INS_CPY_ABS:
			{
				byte operand = read_byte(cpu, am_absolute(cpu));
				set_CMP_CPX_CPY_flags(cpu, operand, cpu->Y);
			} break;

			case INS_DEC_ZP:
			{
				word address = am_zero_page(cpu);
				byte result = read_byte(cpu, address);
				cpu->cycles--;
				write_byte(cpu, --result, address);
				set_flags_ZN(cpu, result);
			} break;

			case INS_DEC_ZPX:
			{
				word address = am_zero_page_x(cpu);
				byte result = read_byte(cpu, address);
				cpu->cycles--;
				write_byte(cpu, --result, address);
				set_flags_ZN(cpu, result);
			} break;

			case INS_DEC_ABS:
			{
				word address = am_absolute(cpu);
				byte result = read_byte(cpu, address);
				cpu->cycles--;
				write_byte(cpu, --result, address);
				set_flags_ZN(cpu, result);
			} break;

			case INS_DEC_ABSX:
			{
				word address = am_absolute_x(cpu, 1);
				byte result = read_byte(cpu, address);
				cpu->cycles--;
				write_byte(cpu, --result, address);
				set_flags_ZN(cpu, result);
			} break;

			case INS_TAX:
			{
				cpu->X = cpu->A;
				cpu->cycles--;
				set_flags_ZN(cpu, cpu->X);
			} break;

			case INS_TXA:
			{
				cpu->A = cpu->X;
				cpu->cycles--;
				set_flags_ZN(cpu, cpu->A);
			} break;

			case INS_DEX:
			{
				cpu->X--;
				cpu->cycles--;
				set_flags_ZN(cpu, cpu->X);
			} break;

			case INS_INX:
			{
				cpu->X++;
				cpu->cycles--;
				set_flags_ZN(cpu, cpu->X);
			} break;

			case INS_TAY:
			{
				cpu->Y = cpu->A;
				cpu->cycles--;
				set_flags_ZN(cpu, cpu->Y);
			} break;

			case INS_TYA:
			{
				cpu->A = cpu->Y;
				cpu->cycles--;
				set_flags_ZN(cpu, cpu->A);
			} break;

			case INS_DEY:
			{
				cpu->Y--;
				cpu->cycles--;
				set_flags_ZN(cpu, cpu->Y);
			} break;

			case INS_INY:
			{
				cpu->Y++;
				cpu->cycles--;
				set_flags_ZN(cpu, cpu->Y);
			} break;

			case INS_EOR_IM:
			{
				byte operand = read_byte(cpu, am_immediate(cpu));
				cpu->A ^= operand;
				set_flags_ZN(cpu, cpu->A);
			} break;

			case INS_EOR_ZP:
			{
				byte operand = read_byte(cpu, am_zero_page(cpu));
				cpu->A ^= operand;
				set_flags_ZN(cpu, cpu->A);
			} break;

			case INS_EOR_ZPX:
			{
				byte operand = read_byte(cpu, am_zero_page_x(cpu));
				cpu->A ^= operand;
				set_flags_ZN(cpu, cpu->A);
			} break;

			case INS_EOR_ABS:
			{
				byte operand = read_byte(cpu, am_absolute(cpu));
				cpu->A ^= operand;
				set_flags_ZN(cpu, cpu->A);
			} break;

			case INS_EOR_ABSX:
			{
				byte operand = read_byte(cpu, am_absolute_x(cpu, 0));
				cpu->A ^= operand;
				set_flags_ZN(cpu, cpu->A);
			} break;

			case INS_EOR_ABSY:
			{
				byte operand = read_byte(cpu, am_absolute_y(cpu, 0));
				cpu->A ^= operand;
				set_flags_ZN(cpu, cpu->A);
			} break;

			case INS_EOR_INDX:
			{
				byte operand = read_byte(cpu, am_indirect_x(cpu));
				cpu->A ^= operand;
				set_flags_ZN(cpu, cpu->A);
			} break;

			case INS_EOR_INDY:
			{
				byte operand = read_byte(cpu, am_indirect_y(cpu, 0));
				cpu->A ^= operand;
				set_flags_ZN(cpu, cpu->A);
			} break;

			case INS_INC_ZP:
			{
				word address = am_zero_page(cpu);
				byte result = read_byte(cpu, address);
				cpu->cycles--;
				write_byte(cpu, ++result, address);
				set_flags_ZN(cpu, result);
			} break;

			case INS_INC_ZPX:
			{
				word address = am_zero_page_x(cpu);
				byte result = read_byte(cpu, address);
				cpu->cycles--;
				write_byte(cpu, ++result, address);
				set_flags_ZN(cpu, result);
			} break;

			case INS_INC_ABS:
			{
				word address = am_absolute(cpu);
				byte result = read_byte(cpu, address);
				cpu->cycles--;
				write_byte(cpu, ++result, address);
				set_flags_ZN(cpu, result);
			} break;

			case INS_INC_ABSX:
			{
				word address = am_absolute_x(cpu, 1);
				byte result = read_byte(cpu, address);
				cpu->cycles--;
				write_byte(cpu, ++result, address);
				set_flags_ZN(cpu, result);
			} break;

			case INS_JMP_ABS:
			{
				cpu->PC = fetch_word(cpu);
			} break;

			case INS_JMP_IND:
			{
				cpu->PC = read_word(cpu, fetch_word(cpu));
			} break;

			case INS_JSR:
			{
				word address = fetch_word(cpu);
				word return_address = cpu->PC-1;
				write_word(cpu, return_address, cpu->SP);
				cpu->SP -= 2;
				cpu->PC = address;
			} break;

            case INS_LDA_IM:
            {
				write_byte(cpu, cpu->A, am_immediate(cpu));
                set_flags_ZN(cpu, cpu->A);
            } break;

            case INS_LDA_ZP:
            {
				write_byte(cpu, cpu->A, am_zero_page(cpu));
                set_flags_ZN(cpu, cpu->A);
            } break;

            case INS_LDA_ZPX:
            {
				write_byte(cpu, cpu->A, am_zero_page_x(cpu));
                set_flags_ZN(cpu, cpu->A);
            } break;

            case INS_LDA_ABS:
            {
				write_byte(cpu, cpu->A, am_absolute(cpu));
                set_flags_ZN(cpu, cpu->A);
            } break;

            case INS_LDA_ABSX:
            {
				write_byte(cpu, cpu->A, am_absolute_x(cpu, 0));
                set_flags_ZN(cpu, cpu->A);
            } break;

            case INS_LDA_ABSY:
            {
				write_byte(cpu, cpu->A, am_absolute_y(cpu, 0));
                set_flags_ZN(cpu, cpu->A);
            } break;

			case INS_LDA_INDX:
			{
				write_byte(cpu, cpu->A, am_indirect_x(cpu));
                set_flags_ZN(cpu, cpu->A);
			} break;

			case INS_LDA_INDY:
			{
				write_byte(cpu, cpu->A, am_indirect_y(cpu, 0));
                set_flags_ZN(cpu, cpu->A);
			} break;

            case INS_LDX_IM:
            {
				write_byte(cpu, cpu->X, am_immediate(cpu));
                set_flags_ZN(cpu, cpu->X);
            } break;

            case INS_LDX_ZP:
            {
				write_byte(cpu, cpu->X, am_zero_page(cpu));
                set_flags_ZN(cpu, cpu->X);
            } break;

            case INS_LDX_ZPY:
            {
				write_byte(cpu, cpu->X, am_zero_page_y(cpu));
                set_flags_ZN(cpu, cpu->X);
            } break;

            case INS_LDX_ABS:
            {
				write_byte(cpu, cpu->X, am_absolute(cpu));
                set_flags_ZN(cpu, cpu->X);
            } break;

            case INS_LDX_ABSY:
            {
				write_byte(cpu, cpu->X, am_absolute_y(cpu, 0));
                set_flags_ZN(cpu, cpu->X);
            } break;

            case INS_LDY_IM:
            {
				write_byte(cpu, cpu->Y, am_immediate(cpu));
                set_flags_ZN(cpu, cpu->Y);
            } break;

            case INS_LDY_ZP:
            {
				write_byte(cpu, cpu->Y, am_zero_page(cpu));
                set_flags_ZN(cpu, cpu->Y);
            } break;

            case INS_LDY_ZPX:
            {
				write_byte(cpu, cpu->Y, am_zero_page_x(cpu));
                set_flags_ZN(cpu, cpu->Y);
            } break;

            case INS_LDY_ABS:
            {
				write_byte(cpu, cpu->Y, am_absolute(cpu));
                set_flags_ZN(cpu, cpu->Y);
            } break;

            case INS_LDY_ABSX:
            {
				write_byte(cpu, cpu->Y, am_absolute_x(cpu, 0));
                set_flags_ZN(cpu, cpu->Y);
            } break;

			case INS_LSR:
			{
				byte old_value = cpu->A;
				cpu->A >>= 1;
				cpu->cycles--;
				set_LSR_flags(cpu, old_value, cpu->A);
			} break;

			case INS_LSR_ZP:
			{
				word address = am_zero_page(cpu);
				byte old_value = read_byte(cpu, address);
				write_byte(cpu, old_value >> 1, address);
				set_LSR_flags(cpu, old_value, read_byte(cpu, address));
			} break;

			case INS_LSR_ZPX:
			{
				word address = am_zero_page_x(cpu);
				byte old_value = read_byte(cpu, address);
				write_byte(cpu, old_value >> 1, address);
				set_LSR_flags(cpu, old_value, read_byte(cpu, address));
			} break;

			case INS_LSR_ABS:
			{
				word address = am_absolute(cpu);
				byte old_value = read_byte(cpu, address);
				write_byte(cpu, old_value >> 1, address);
				set_LSR_flags(cpu, old_value, read_byte(cpu, address));
			} break;

			case INS_LSR_ABSX:
			{
				word address = am_absolute_x(cpu, 1);
				byte old_value = read_byte(cpu, address);
				write_byte(cpu, old_value >> 1, address);
				set_LSR_flags(cpu, old_value, read_byte(cpu, address));
			} break;

			case INS_NOP:
			{
				cpu->PC++;
				cpu->cycles--;
			} break;

			case INS_ORA_IM:
			{
				byte value = read_byte(cpu, am_immediate(cpu));
				cpu->A ^= value;
				set_flags_ZN(cpu, cpu->A);
			} break;

			case INS_ORA_ZP:
			{
				byte value = read_byte(cpu, am_zero_page(cpu));
				cpu->A ^= value;
				set_flags_ZN(cpu, cpu->A);
			} break;

			case INS_ORA_ZPX:
			{
				byte value = read_byte(cpu, am_zero_page_x(cpu));
				cpu->A ^= value;
				set_flags_ZN(cpu, cpu->A);
			} break;

			case INS_ORA_ABS:
			{
				byte value = read_byte(cpu, am_absolute(cpu));
				cpu->A ^= value;
				set_flags_ZN(cpu, cpu->A);
			} break;

			case INS_ORA_ABSX:
			{
				byte value = read_byte(cpu, am_absolute_x(cpu, 0));
				cpu->A ^= value;
				set_flags_ZN(cpu, cpu->A);
			} break;

			case INS_ORA_ABSY:
			{
				byte value = read_byte(cpu, am_absolute_y(cpu, 0));
				cpu->A ^= value;
				set_flags_ZN(cpu, cpu->A);
			} break;

			case INS_ORA_INDX:
			{
				byte value = read_byte(cpu, am_indirect_x(cpu));
				cpu->A ^= value;
				set_flags_ZN(cpu, cpu->A);
			} break;

			case INS_ORA_INDY:
			{
				byte value = read_byte(cpu, am_indirect_y(cpu, 0));
				cpu->A ^= value;
				set_flags_ZN(cpu, cpu->A);
			} break;

			case INS_STA_ZP:
			{
				write_byte(cpu, cpu->A, am_zero_page(cpu));
			} break;

			case INS_STA_ZPX:
			{
				write_byte(cpu, cpu->A, am_zero_page_x(cpu));
			} break;

			case INS_STA_ABS:
			{
				write_byte(cpu, cpu->A, am_absolute(cpu));
			} break;

			case INS_STA_ABSX:
			{
				write_byte(cpu, cpu->A, am_absolute_x(cpu, 1));
			} break;

			case INS_STA_ABSY:
			{
				write_byte(cpu, cpu->A, am_absolute_y(cpu, 1));
			} break;

			case INS_STA_INDX:
			{
				write_byte(cpu, cpu->A, am_indirect_x(cpu));
			} break;

			case INS_STA_INDY:
			{
				write_byte(cpu, cpu->A, am_indirect_y(cpu, 1));
			} break;

			case INS_TXS:
			{
				cpu->SP = cpu->X;
				cpu->cycles--;
			} break;

			case INS_TSX:
			{
				cpu->X = read_byte(cpu, cpu->SP);
				set_flags_ZN(cpu, cpu->X);
			} break;

			case INS_PHA:
			{
				write_byte(cpu, cpu->A, cpu->SP--);
				cpu->cycles--;
			} break;

			case INS_PLA:
			{
				cpu->A = read_byte(cpu, cpu->SP++);
				cpu->cycles -= 2;
				set_flags_ZN(cpu, cpu->A);
			} break;

			case INS_PHP:
			{
				write_byte(cpu, cpu->SR, cpu->SP--);
				cpu->cycles--;
			} break;

			case INS_PLP:
			{
				cpu->SR = read_byte(cpu, cpu->SP++);
				cpu->cycles -= 2;
			} break;

			case INS_ROL:
			{
				byte old_bit7 = cpu->A | 0x80;
				byte carry_bit = cpu->SR | STATUS_CARRY_FLAG;
				cpu->A = (cpu->A << 1) | carry_bit;
				cpu->SR |= (old_bit7 & 0x80) ? STATUS_CARRY_FLAG : 0;
				cpu->cycles--;
				set_flags_ZN(cpu, cpu->A);
			} break;

			case INS_ROL_ZP:
			{
				word address = am_zero_page(cpu);
				byte value = read_byte(cpu, address);
				byte old_bit7 = value | 0x80;
				byte carry_bit = cpu->SR | STATUS_CARRY_FLAG;
				write_byte(cpu, value << 1 | carry_bit, address);
				cpu->SR |= (old_bit7 & 0x80) ? STATUS_CARRY_FLAG : 0;
				set_flags_ZN(cpu, read_byte(cpu, address));
			} break;

			case INS_ROL_ZPX:
			{
				word address = am_zero_page_x(cpu);
				byte value = read_byte(cpu, address);
				byte old_bit7 = value | 0x80;
				byte carry_bit = cpu->SR | STATUS_CARRY_FLAG;
				write_byte(cpu, value << 1 | carry_bit, address);
				cpu->SR |= (old_bit7 & 0x80) ? STATUS_CARRY_FLAG : 0;
				set_flags_ZN(cpu, read_byte(cpu, address));
			} break;

			case INS_ROL_ABS:
			{
				word address = am_absolute(cpu);
				byte value = read_byte(cpu, address);
				byte old_bit7 = value | 0x80;
				byte carry_bit = cpu->SR | STATUS_CARRY_FLAG;
				write_byte(cpu, value << 1 | carry_bit, address);
				cpu->SR |= (old_bit7 & 0x80) ? STATUS_CARRY_FLAG : 0;
				set_flags_ZN(cpu, read_byte(cpu, address));
			} break;

			case INS_ROL_ABSX:
			{
				word address = am_absolute_x(cpu, 1);
				byte value = read_byte(cpu, address);
				byte old_bit7 = value | 0x80;
				byte carry_bit = cpu->SR | STATUS_CARRY_FLAG;
				write_byte(cpu, value << 1 | carry_bit, address);
				cpu->SR |= (old_bit7 & 0x80) ? STATUS_CARRY_FLAG : 0;
				set_flags_ZN(cpu, read_byte(cpu, address));
			} break;

			case INS_ROR:
			{
				byte old_bit0 = cpu->A | 0x01;
				byte carry_bit = (cpu->SR | STATUS_CARRY_FLAG) ? 0x80 : 0;
				cpu->A = (cpu->A >> 1) | carry_bit;
				cpu->SR |= (old_bit0 & 0x01) ? STATUS_CARRY_FLAG : 0;
				cpu->cycles--;
				set_flags_ZN(cpu, cpu->A);
			} break;

			case INS_ROR_ZP:
			{
				word address = am_zero_page(cpu);
				byte value = read_byte(cpu, address);
				byte old_bit0 = cpu->A | 0x01;
				byte carry_bit = (cpu->SR | STATUS_CARRY_FLAG) ? 0x80 : 0;
				write_byte(cpu, value >> 1 | carry_bit, address);
				cpu->SR |= (old_bit0 & 0x01) ? STATUS_CARRY_FLAG : 0;
				set_flags_ZN(cpu, read_byte(cpu, address));
			} break;

			case INS_ROR_ZPX:
			{
				word address = am_zero_page_x(cpu);
				byte value = read_byte(cpu, address);
				byte old_bit0 = cpu->A | 0x01;
				byte carry_bit = (cpu->SR | STATUS_CARRY_FLAG) ? 0x80 : 0;
				write_byte(cpu, value >> 1 | carry_bit, address);
				cpu->SR |= (old_bit0 & 0x01) ? STATUS_CARRY_FLAG : 0;
				set_flags_ZN(cpu, read_byte(cpu, address));
			} break;

			case INS_ROR_ABS:
			{
				word address = am_absolute(cpu);
				byte value = read_byte(cpu, address);
				byte old_bit0 = cpu->A | 0x01;
				byte carry_bit = (cpu->SR | STATUS_CARRY_FLAG) ? 0x80 : 0;
				write_byte(cpu, value >> 1 | carry_bit, address);
				cpu->SR |= (old_bit0 & 0x01) ? STATUS_CARRY_FLAG : 0;
				set_flags_ZN(cpu, read_byte(cpu, address));
			} break;

			case INS_ROR_ABSX:
			{
				word address = am_absolute_x(cpu, 1);
				byte value = read_byte(cpu, address);
				byte old_bit0 = cpu->A | 0x01;
				byte carry_bit = (cpu->SR | STATUS_CARRY_FLAG) ? 0x80 : 0;
				write_byte(cpu, value >> 1 | carry_bit, address);
				cpu->SR |= (old_bit0 & 0x01) ? STATUS_CARRY_FLAG : 0;
				set_flags_ZN(cpu, read_byte(cpu, address));
			} break;

			case INS_RTI:
			{
				cpu->SR = read_byte(cpu, ++cpu->SP);
				cpu->PC = read_word(cpu, ++cpu->SP);
				cpu->PC++;
				cpu->cycles -= 2;
			} break;

			case INS_RTS:
			{
				cpu->PC = read_word(cpu, ++cpu->SP)+1;
				cpu->cycles -= 3; 
			} break;

			case INS_SBC:
			{
				byte operand = read_byte(cpu, am_immediate(cpu));
				byte result = operand + ((cpu->SR & STATUS_CARRY_FLAG) ? 0 : 1);
				set_ADC_flags(cpu, operand, result);
				cpu->A -= result;
			} break;

			case INS_SBC_ZP:
			{
				byte operand = read_byte(cpu, am_zero_page(cpu));
				byte result = operand + ((cpu->SR & STATUS_CARRY_FLAG) ? 0 : 1);
				set_ADC_flags(cpu, operand, result);
				cpu->A -= result;
			} break;

			case INS_SBC_ZPX:
			{
				byte operand = read_byte(cpu, am_zero_page_x(cpu));
				byte result = operand + ((cpu->SR & STATUS_CARRY_FLAG) ? 0 : 1);
				set_ADC_flags(cpu, operand, result);
				cpu->A -= result;
			} break;

			case INS_SBC_ABS:
			{
				byte operand = read_byte(cpu, am_absolute(cpu));
				byte result = operand + ((cpu->SR & STATUS_CARRY_FLAG) ? 0 : 1);
				set_ADC_flags(cpu, operand, result);
				cpu->A -= result;
			} break;

			case INS_SBC_ABSX:
			{
				byte operand = read_byte(cpu, am_absolute_x(cpu, 0));
				byte result = operand + ((cpu->SR & STATUS_CARRY_FLAG) ? 0 : 1);
				set_ADC_flags(cpu, operand, result);
				cpu->A -= result;
			} break;

			case INS_SBC_ABSY:
			{
				byte operand = read_byte(cpu, am_absolute_y(cpu, 0));
				byte result = operand + ((cpu->SR & STATUS_CARRY_FLAG) ? 0 : 1);
				set_ADC_flags(cpu, operand, result);
				cpu->A -= result;
			} break;

			case INS_SBC_INDX:
			{
				byte operand = read_byte(cpu, am_indirect_x(cpu));
				byte result = operand + ((cpu->SR & STATUS_CARRY_FLAG) ? 0 : 1);
				set_ADC_flags(cpu, operand, result);
				cpu->A -= result;
			} break;

			case INS_SBC_INDY:
			{
				byte operand = read_byte(cpu, am_indirect_y(cpu, 0));
				byte result = operand + ((cpu->SR & STATUS_CARRY_FLAG) ? 0 : 1);
				set_ADC_flags(cpu, operand, result);
				cpu->A -= result;
			} break;

			case INS_STX_ZP:
			{
				write_byte(cpu, cpu->X, am_zero_page(cpu));
			} break;

			case INS_STX_ZPY:
			{
				write_byte(cpu, cpu->X, am_zero_page_y(cpu));
			} break;

			case INS_STX_ABS:
			{
				write_byte(cpu, cpu->X, am_absolute(cpu));
			} break;

			case INS_STY_ZP:
			{
				write_byte(cpu, cpu->Y, am_zero_page(cpu));
			} break;

			case INS_STY_ZPX:
			{
				write_byte(cpu, cpu->Y, am_zero_page_x(cpu));
			} break;

			case INS_STY_ABS:
			{
				write_byte(cpu, cpu->Y, am_absolute(cpu));
			} break;

            default:
            {
                printf("Unknown instruction: 0x%x\n", instruction);
            } break;
        }
    }
}
