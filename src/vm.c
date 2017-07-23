#ifdef __cplusplus
extern "C" {
#endif

#include "vm.h"

void vm_Create(struct VM *vm) {
	memset(vm->cells, 0, sizeof(vm->cells));

	vm->pc = 0;
	vm->cell_ptr = vm->cells;
}

int vm_Compile(struct VM *vm, const char *code) {
	int error;
	compile_bytecode(code, &vm->instructions, NULL, &error);
	return error;
}

void vm_Cleanup(struct VM *vm) {
	free((void*)vm->instructions);
}

//http://esolangs.org/wiki/Brainfuck
int vm_Step(struct VM *vm) {
	Instruction_t insn = vm->instructions[vm->pc];

	switch(insn.opcode) {
	case OP_ADD_CELL_POINTER:
		vm->cell_ptr += insn.operand * sizeof(CELL_TYPE);

		if(vm->cell_ptr - vm->cells >= NUM_CELLS)
			return E_CELL_POINTER_OUT_OF_BOUNDS;
		break;
	case OP_SUB_CELL_POINTER:
		vm->cell_ptr -= insn.operand * sizeof(CELL_TYPE);
		if(vm->cell_ptr - vm->cells < 0) //it's unsigned, so we can check like this
			return E_CELL_POINTER_OUT_OF_BOUNDS;
		break;
	case OP_ADD_CELL_VALUE:
		*vm->cell_ptr += insn.operand;
		break;
	case OP_SUB_CELL_VALUE:
		*vm->cell_ptr -= insn.operand;
		break;
	case OP_PRINT_CELL:
		bf_print_cell(*vm->cell_ptr);
		break;
	case OP_INPUT_CELL:
		*vm->cell_ptr = bf_get_input();
		break;
	case OP_OPEN_BRACKET:
		if(*vm->cell_ptr == 0)
			vm->pc = insn.operand - 1; //have to subtract 1 because pc is incremented at end of switch
		break;
	case OP_CLOSE_BRACKET:
		if(*vm->cell_ptr != 0)
			vm->pc = insn.operand - 1; //have to subtract 1 because pc is incremented at end of switch
		break;
#ifdef OPTIMIZE
	case OP_SET_ZERO:
		*vm->cell_ptr = 0;
		break;
#endif
	case OP_DONE:
		vm->pc--;
		break;
	}
	vm->pc++;

	return E_SUCCESS;
}

bool vm_IsDone(struct VM *vm) {
	return vm->instructions[vm->pc].opcode == OP_DONE;
}

#ifdef __cplusplus
}
#endif