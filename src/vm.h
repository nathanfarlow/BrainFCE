#ifndef VM_H_
#define VM_H_

#include <string.h>

#include <stdio.h>
#include <stdlib.h>

#include <stdbool.h>
#include <stddef.h>

#include "compiler.h"

void vm_Create(struct VM *vm);
int vm_Compile(struct VM *vm, const char *code);
int vm_Step(struct VM *vm);
bool vm_IsDone(struct VM *vm);
void vm_Cleanup(struct VM *vm);

#endif