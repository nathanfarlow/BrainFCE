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

#ifdef __TICE__
    #define CHAR_GREATER        0x6C
    #define CHAR_LESS           0x6B
    #define CHAR_PLUS           0x70
    #define CHAR_MINUS          0x71
    #define CHAR_PERIOD         0x3A
    #define CHAR_COMMA          0x2B
    #define CHAR_OPEN_BRACKET   0x06
    #define CHAR_CLOSE_BRACKET  0x07
#else
    #define CHAR_GREATER        '>'
    #define CHAR_LESS           '<'
    #define CHAR_PLUS           '+'
    #define CHAR_MINUS          '-'
    #define CHAR_PERIOD         '.'
    #define CHAR_COMMA          ','
    #define CHAR_OPEN_BRACKET   '['
    #define CHAR_CLOSE_BRACKET  ']'
#endif

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

extern const char *error_strings[8];
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

enum RegState {
    JUNK,
    CELL_PTR,
    CELL_VALUE,
    NUMBER
};

typedef struct Compiler {
    const char *program;
    size_t program_length;

    int error;

    unsigned int pc;

	enum RegState hl, de, bc;
    unsigned int bc_val;

    Stack_t stack;

    size_t code_length;
    union {
        Instruction_t *bytecode;
        uint8_t *native;
	} code;
} Compiler_t;

void comp_Create(Compiler_t *c, const char *program, size_t program_length);

void comp_CompileBytecode(Compiler_t *c, bool optimize);
void comp_CompileNative(Compiler_t *c, struct Memory *mem, bool optimize);

void comp_CleanupBytecode(Compiler_t *c);
void comp_CleanupNative(Compiler_t *c);

//Functions that are called from the native and interpreted program
void bf_print_cell(CELL_TYPE cell);
CELL_TYPE bf_get_input();

#endif