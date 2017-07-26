#ifndef VM_H_
#define VM_H_


#include <stdio.h>
#include <stdlib.h>

#include <stdbool.h>
#include <stddef.h>

#include "compiler.h"


struct VM {

    struct Memory mem;
    
    Instruction_t *instructions;
    size_t num_insns;

    CELL_TYPE pc;
};

void vm_Create(struct VM *vm);
int vm_Compile(struct VM *vm, const char *code, size_t len, bool optimize);
int vm_Step(struct VM *vm);
bool vm_IsDone(struct VM *vm);
void vm_Cleanup(struct VM *vm);

#endif