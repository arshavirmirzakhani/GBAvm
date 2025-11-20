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

int main() {
	REG_DISPCNT = DCNT_MODE3 | DCNT_BG2;

	Script main_script = {.program = program, .ip = 0, .length = sizeof(program) / sizeof(program[0])};

	VM vm = {.running = 1};

	Scene main_scene = {0};
	Object obj;
	obj.script = &main_script;
	scene_add_object(&main_scene, &obj, 0);

	CURRENT_SCENE = &main_scene;

	irq_init(NULL);
	irq_enable(II_VBLANK);

	while (1) {
		VBlankIntrWait();

		for (int i = 0; i < 3 && vm.running; i++)
			vm_step(&vm, palette);
	}
}
