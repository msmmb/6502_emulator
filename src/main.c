#include <stdio.h>
#include "6502.h"

int main() {
    struct CPU cpu;
    reset(&cpu);

    FILE *fp = fopen("test_files/6502_functional_test.bin", "rb");
    if (fp == NULL) return 1;

    fread(&cpu.memory[0x000A], 1, 1024 * 64, fp);
    fclose(fp);

    cpu.PC = 0x400;
    
    execute(100000, &cpu);

    return 0;
}