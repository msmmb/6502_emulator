#include "../inc/6502.h"

int main()
{
    struct CPU cpu;
    struct Memory mem;
    reset(&cpu, &mem);
    mem.data[0xfffc] = INS_LDA_IM;
    mem.data[0xfffd] = 0x42;
	execute(2, &cpu, &mem);
    return 0;
}