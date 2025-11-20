#ifndef GBAVM_H
#define GBAVM_H

#include <tonc.h>

#define MAX_OBJECTS_PER_SCENE 64
#define MAX_LOCAL_VARIABLES 16
#define MAX_SCENES 512

typedef int Inst;

#define OPC(inst) ((inst >> 24) & 0xFF)
#define ARG(inst) (inst & 0x00FFFFFF)

#define ARG_X(a) (((a) >> 16) & 0xFF)
#define ARG_Y(a) (((a) >> 8) & 0xFF)
#define ARG_Z(a) ((a) & 0xFF)

typedef struct Script {
		const Inst* program;
		int ip;
		int length;
} Script;

typedef struct Object {
		Script* script;
		int pos_x;
		int pos_y;
		int local_variables[MAX_LOCAL_VARIABLES];
} Object;

typedef struct Scene {
		Object* objects[MAX_OBJECTS_PER_SCENE];
} Scene;

static Scene* CURRENT_SCENE;

typedef struct VM {
		int running;
} VM;

void load_scene(Scene* scene) { CURRENT_SCENE = scene; }

void scene_add_object(Scene* s, Object* o, int index) {
	if (index >= 0 && index < MAX_OBJECTS_PER_SCENE) s->objects[index] = o;
}

void vm_step(VM* vm, const u16* palette) {

	for (int i = 0; i < MAX_OBJECTS_PER_SCENE; i++) {
		Object* obj = CURRENT_SCENE->objects[i];
		if (!obj || !obj->script || !obj->script->program) continue;

		Script* scr = obj->script;

		Inst inst = scr->program[scr->ip++];
		u8 op	  = OPC(inst);

		switch (op) {
		case 0x00: // NOP
			break;
		case 0x31: // DRAW_DOT
		{
			u32 arg = ARG(inst);
			int x	= ARG_X(arg);
			int y	= ARG_Y(arg);
			u16 clr = palette[ARG_Z(arg)];

			m3_plot(x, y, clr);
		} break;

		case 0x10: // JUMP
			CURRENT_SCENE->objects[i]->script->ip = ARG(inst);
			break;

		case 0xFF: // END
			vm->running = 0;
			break;
		}
	}
}

#endif