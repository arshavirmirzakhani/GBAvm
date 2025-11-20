#include "gbavm.h"
#include "metr.h"

const Inst program[] = {
    // MOV R0, 10          // X position
    (0x01 << 24) | (0 << 16) | (10),
    // MOV R1, 70          // Y position
    (0x01 << 24) | (1 << 16) | (70),
    // MOV R2, 0           // Color index = 0 (CLR_RED)
    (0x01 << 24) | (2 << 16) | (0),
    // MOV R3, 1           // Step size (+1)
    (0x01 << 24) | (3 << 16) | (1),
    // MOV R4, 240         // Screen width for wrap
    (0x01 << 24) | (4 << 16) | (240),
    // --- Main Loop ---
    // DRAW_DOT R0, R1, R2
    (0x31 << 24) | (0 << 16) | (1 << 8) | (2),
    // ADD R0, R3          // R0 += R3
    (0x02 << 24) | (0 << 16) | (3 << 8),
    // // Wrapping logic for X: if (R0 >= R4) R0 = 10
    // SUB R5, R0, R4      // R5 = R0 - R4
    (0x03 << 24) | (5 << 16) | (4 << 8),
    // CLEAR SCREEN
    (0x32 << 24),
    // JUMP 5              // Always jump to drawing instruction (could add conditional logic if you add 'JGE' opcodes)
    (0x10 << 24) | (5),
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

	obj.registers[0] = 10; // X
	obj.registers[1] = 70; // Y
	obj.registers[2] = 0;  // color index (CLR_RED)
	obj.registers[3] = 1;  // step size (+1 in X per frame)
	obj.registers[4] = 240;

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
