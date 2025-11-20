#ifndef GBAVM_H
#define GBAVM_H

#include <tonc.h>

#define MAX_OBJECTS_PER_SCENE 64

typedef int Inst;

#define OPC(inst) ((inst >> 24) & 0xFF)
#define ARG(inst) (inst & 0x00FFFFFF)

#define ARG_X(a) (((a) >> 16) & 0xFF)
#define ARG_Y(a) (((a) >> 8) & 0xFF)
#define ARG_Z(a) ((a) & 0xFF)

typedef struct Script
{
} Script;

typedef struct Object
{
    int pos_x;
    int pos_y;
} Object;

typedef struct Scene
{
    Object *objects[MAX_OBJECTS_PER_SCENE];
} Scene;

typedef struct
{
    const Inst *prog;
    int ip;
    int running;
} VM;

void vm_step(VM *vm, const u16 *palette)
{
    Inst inst = vm->prog[vm->ip++];
    u8 op = OPC(inst);

    switch (op)
    {
    case 0x31: // DRAW_DOT
    {
        u32 arg = ARG(inst);
        int x = ARG_X(arg);
        int y = ARG_Y(arg);
        u16 clr = palette[ARG_Z(arg)];

        m3_plot(x, y, clr);
    }
    break;

    case 0x10: // JUMP
        vm->ip = ARG(inst);
        break;

    case 0xFF: // END
        vm->running = 0;
        break;

        // Add more ops later…
    }
}

#endif