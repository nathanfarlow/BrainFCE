#ifndef COMPILER_H_
#define COMPILER_H_

#include <stddef.h>
#include <stdint.h>

#include "stack.h"

#ifdef __TICE__
#define NUM_CELLS 1048
#define CELL_24
#elif defined(__TI__)
#define NUM_CELLS 1024
#define CELL_16
#else
#define NUM_CELLS 30000
#define CELL_32
#endif

#ifdef CELL_24
#define CELL_TYPE uint24_t
#elif defined(CELL_16)
#define CELL_TYPE uint16_t
#elif defined(CELL_32)
#define CELL_TYPE uint32_t
#endif

#define OPTIMIZE

enum Opcodes {
	OP_ADD_CELL_POINTER,
	OP_SUB_CELL_POINTER,
	OP_ADD_CELL_VALUE,
	OP_SUB_CELL_VALUE,
	OP_PRINT_CELL,
	OP_INPUT_CELL,
	OP_OPEN_BRACKET,
	OP_CLOSE_BRACKET,
	OP_SET_ZERO,
	OP_DONE
};

enum Errors {
	E_SUCCESS,

	E_GENERIC_COMPILE,

	E_STACK_OVERFLOW,
	E_STACK_UNDERFLOW,

	E_CELL_POINTER_OUT_OF_BOUNDS
};

typedef struct Instruction {
	uint8_t opcode;
	CELL_TYPE operand;
} Instruction_t;

/*The brainfuck vm*/
struct VM {
	CELL_TYPE cells[NUM_CELLS];
	CELL_TYPE *cell_ptr;

	Instruction_t *instructions;

	CELL_TYPE pc;
};

void bf_print_cell(CELL_TYPE cell);
CELL_TYPE bf_get_input();

void compile_bytecode(const char *code, Instruction_t **instructions, size_t *instructions_length, int *error);
void compile_native(const Instruction_t *instructions, uint32_t instruction_length, uint8_t **native_code, size_t *native_length, struct VM *vm, int *error);

#endif