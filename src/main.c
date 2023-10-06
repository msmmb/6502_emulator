#include "../inc/6502.h"


int main()
{
    struct CPU cpu;
    reset(&cpu);
    cpu.memory[0xfffc] = INS_LDA_IM;
    cpu.memory[0xfffd] = 0x42;
    int cycles = 2;
	execute(2, &cpu);
    return 0;
}