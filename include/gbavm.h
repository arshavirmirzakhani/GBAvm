#ifndef GBAVM_H
#define GBAVM_H

#include <tonc.h>

#define MAX_OBJECTS_PER_SCENE 64
#define MAX_LOCAL_VARIABLES 16
#define MAX_SCENES 512
#define VM_NUM_REGISTERS 8

typedef int Inst;

// --- Opcode decoding macros ---
#define OPC(inst) (((inst) >> 24) & 0xFF)
#define ARG(inst) ((inst) & 0x00FFFFFF)

#define ARG_Rd(a) (((a) >> 16) & 0xFF)
#define ARG_Rs(a) (((a) >> 8) & 0xFF)
#define ARG_IMM(a) ((a) & 0xFF)

// --- Opaque types ---
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
		int registers[VM_NUM_REGISTERS];
} Object;

typedef struct Scene {
		Object* objects[MAX_OBJECTS_PER_SCENE];
} Scene;

typedef struct VM {
		int running;
} VM;

// --- Global state ---
static Scene* CURRENT_SCENE = NULL;

// --- Helper functions ---
static inline void load_scene(Scene* scene) { CURRENT_SCENE = scene; }
static inline void scene_add_object(Scene* s, Object* o, int index) {
	if (s && o && index >= 0 && index < MAX_OBJECTS_PER_SCENE) s->objects[index] = o;
}

// --- Opcode handlers ---

static void op_nop(int* reg, u32 arg) {
	(void)reg;
	(void)arg;
}
static void op_mov(int* reg, u32 arg) {
	int rd = ARG_Rd(arg), imm = ARG_IMM(arg);
	if (rd < VM_NUM_REGISTERS) reg[rd] = imm;
}
static void op_add(int* reg, u32 arg) {
	int rd = ARG_Rd(arg), rs = ARG_Rs(arg);
	if (rd < VM_NUM_REGISTERS && rs < VM_NUM_REGISTERS) reg[rd] += reg[rs];
}
static void op_sub(int* reg, u32 arg) {
	int rd = ARG_Rd(arg), rs = ARG_Rs(arg);
	if (rd < VM_NUM_REGISTERS && rs < VM_NUM_REGISTERS) reg[rd] -= reg[rs];
}
static void op_load(Object* obj, u32 arg) {
	int rd = ARG_Rd(arg), idx = ARG_IMM(arg);
	if (rd < VM_NUM_REGISTERS && idx < MAX_LOCAL_VARIABLES) obj->registers[rd] = obj->local_variables[idx];
}
static void op_store(Object* obj, u32 arg) {
	int rs = ARG_Rs(arg), idx = ARG_IMM(arg);
	if (rs < VM_NUM_REGISTERS && idx < MAX_LOCAL_VARIABLES) obj->local_variables[idx] = obj->registers[rs];
}
static void op_jump(Script* scr, u32 arg) {
	int addr = ARG_IMM(arg);
	if (addr >= 0 && addr < scr->length) scr->ip = addr;
}
static void op_jz(int* reg, Script* scr, u32 arg) {
	int rd = ARG_Rd(arg), addr = ARG_IMM(arg);
	if (rd < VM_NUM_REGISTERS && reg[rd] == 0 && addr >= 0 && addr < scr->length) scr->ip = addr;
}
static void op_jnz(int* reg, Script* scr, u32 arg) {
	int rd = ARG_Rd(arg), addr = ARG_IMM(arg);
	if (rd < VM_NUM_REGISTERS && reg[rd] != 0 && addr >= 0 && addr < scr->length) scr->ip = addr;
}
static void op_draw_dot(int* reg, u32 arg, const u16* palette) {
	int rx = ARG_Rd(arg), ry = ARG_Rs(arg), clr = ARG_IMM(arg);
	if (rx < VM_NUM_REGISTERS && ry < VM_NUM_REGISTERS) m3_plot(reg[rx], reg[ry], palette[clr % 256]);
}
static void op_clear_screen(void) {
	m3_fill(0); // Fill with color 0;
}

// --- VM main interpreter ---
static void vm_step(VM* vm, const u16* palette) {
	if (!CURRENT_SCENE || !vm || !palette || !vm->running) return;
	for (int i = 0; i < MAX_OBJECTS_PER_SCENE; i++) {
		Object* obj = CURRENT_SCENE->objects[i];
		if (!obj || !obj->script || !obj->script->program) continue;
		Script* scr = obj->script;
		if (scr->ip >= scr->length || !vm->running) continue;

		Inst inst = scr->program[scr->ip++];
		u8 op	  = OPC(inst);
		u32 arg	  = ARG(inst);

		int* reg = obj->registers;

		switch (op) {
		case 0x00:
			op_nop(reg, arg);
			break;
		case 0x01:
			op_mov(reg, arg);
			break;
		case 0x02:
			op_add(reg, arg);
			break;
		case 0x03:
			op_sub(reg, arg);
			break;
		case 0x04:
			op_load(obj, arg);
			break;
		case 0x05:
			op_store(obj, arg);
			break;
		case 0x10:
			op_jump(scr, arg);
			break;
		case 0x11:
			op_jz(reg, scr, arg);
			break;
		case 0x12:
			op_jnz(reg, scr, arg);
			break;
		case 0x31:
			op_draw_dot(reg, arg, palette);
			break;
		case 0x32:
			op_clear_screen();
			break;
		case 0xFF:
			vm->running = 0;
			break;
		default:
			break; // Unknown opcode: ignore
		}
	}
}

#endif