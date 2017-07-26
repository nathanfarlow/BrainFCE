#ifndef COMPILER_H_
#define COMPILER_H_

#include <stddef.h>
#include <stdint.h>

#include "stack.h"

#ifdef __TICE__
#define NUM_CELLS 1024
#define CELL_TYPE uint24_t
#define CELL_24
#else
#define NUM_CELLS 30000
#define CELL_32
#define CELL_TYPE uint32_t
#endif

#include <stdbool.h>

/*Opcodes for bytecode that represent BF instructions*/
enum Opcodes {
    OP_ADD_CELL_POINTER,
    OP_ADD_CELL_VALUE,
    OP_PRINT_CELL,
    OP_INPUT_CELL,
    OP_OPEN_BRACKET,
    OP_CLOSE_BRACKET,
    OP_SET_ZERO,
};

enum Errors {
    
    /*Generic*/
    E_SUCCESS,
    E_OUT_OF_MEMORY,

    /*Compilation*/
    E_GENERIC_COMPILE,  //Something went wrong with our compilation algorithms
    E_STACK_OVERFLOW,   //Too many leading [ without ]
    E_STACK_UNDERFLOW,  //Unbalanced ]

    /*Runtime*/
    E_CELL_POINTER_OUT_OF_BOUNDS,   //cell_ptr exceeds cells[]
    E_INVALID_OPCODE,               //A bytecode with an invalid opcode... something wrong with our algs
    E_PC_EXCEEDS_PROGRAM            //When vm_Step() is called when there are no more instructions to execute check vm_IsDone()
};

typedef struct {
    uint8_t opcode;
    CELL_TYPE operand;
} Instruction_t;

/*Holds the memory of a bf program*/
struct Memory {
    CELL_TYPE cells[NUM_CELLS];
    CELL_TYPE *cell_ptr;
};

void mem_Create(struct Memory *mem);

//Functions that are called from the native and interpreted program
void bf_print_cell(CELL_TYPE cell);
CELL_TYPE bf_get_input();

void compile_bytecode(const char *code, size_t length, bool optimize, Instruction_t **instructions_ret, size_t *instructions_length, int *error);
void compile_native(const Instruction_t *instructions, uint32_t instruction_length, uint8_t **native_code, size_t *native_length, struct Memory *mem, int *error);

#endif