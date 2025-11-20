#include "gbavm.h"
#include "metr.h"

const Inst program[] = {
    // opcode=1 (draw), args=(x<<16)|(y<<8)|colorIndex
    (0x31 << 24) | (10 << 16) | (10 << 8) | 0,
    (0x31 << 24) | (50 << 16) | (50 << 8) | 1,

    // Jump back to instruction 0
    (0x10 << 24) | 0,
};

const u16 palette[256] = {
    CLR_RED,
    CLR_GREEN,
    CLR_BLUE,
};

int main()
{
    REG_DISPCNT = DCNT_MODE3 | DCNT_BG2;

    VM vm = {
        .prog = program,
        .ip = 0,
        .running = 1};

    irq_init(NULL);
    irq_enable(II_VBLANK);

    while (1)
    {
        VBlankIntrWait();

        for (int i = 0; i < 3 && vm.running; i++)
            vm_step(&vm, palette);
    }
}
