#ifdef __cplusplus
extern "C" {
#endif

#include "vm.h"

void vm_Create(struct VM *vm) {
    mem_Create(&vm->mem);

    vm->instructions = NULL;
    vm->pc = 0;
}

int vm_Compile(struct VM *vm, const char *code, size_t len, bool optimize) {
    Compiler_t compiler;

    comp_Create(&compiler, code, len);
    comp_CompileBytecode(&compiler, optimize);

    vm->instructions = compiler.code.bytecode;
    vm->num_insns = compiler.code_length;

    return compiler.error;
}

void vm_Cleanup(struct VM *vm) {
    if(vm->instructions != NULL)
        free((void*)vm->instructions);
}

//http://esolangs.org/wiki/Brainfuck
int vm_Step(struct VM *vm) {
    Instruction_t insn = vm->instructions[vm->pc];

    switch(insn.opcode) {
    case OP_ADD_CELL_POINTER:
        vm->mem.cell_ptr += insn.operand;// *sizeof(CELL_TYPE);

        if(vm->mem.cell_ptr - vm->mem.cells >= NUM_CELLS)
            return E_CELL_POINTER_OUT_OF_BOUNDS;

        break;
    case OP_ADD_CELL_VALUE:
        *vm->mem.cell_ptr += insn.operand;
        break;
    case OP_PRINT_CELL:
        bf_print_cell(*vm->mem.cell_ptr);
        break;
    case OP_INPUT_CELL:
        *vm->mem.cell_ptr = bf_get_input();
        break;
    case OP_OPEN_BRACKET:
        if(*vm->mem.cell_ptr == 0)
            vm->pc = insn.operand - 1; //have to subtract 1 because pc is incremented at end of switch
        break;
    case OP_CLOSE_BRACKET:
        if(*vm->mem.cell_ptr != 0)
            vm->pc = insn.operand - 1; //have to subtract 1 because pc is incremented at end of switch
        break;
    case OP_SET_ZERO:
        *vm->mem.cell_ptr = 0;
        break;
    default:
        return E_INVALID_OPCODE;
    }

    vm->pc++;

    return E_SUCCESS;
}

bool vm_IsDone(struct VM *vm) {
    return vm->pc >= vm->num_insns;
}

#ifdef __cplusplus
}
#endif