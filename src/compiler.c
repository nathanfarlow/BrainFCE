#ifdef __cplusplus
extern "C" {
#endif

#include "compiler.h"

#include <stdlib.h>
#include <stdio.h>
#include <stddef.h>
#include <stdbool.h>

#include <string.h>

#include "stack.h"

#ifdef OPTIMIZE
CELL_TYPE consecutive(const char *code, size_t index, char c) {

	CELL_TYPE consec = 0;
	for (; index < strlen(code); index++) {
		if (code[index] == c)
			consec++;
		else break;
	}

	return consec;
}
#endif

void compile_bytecode(const char *code, Instruction_t **instructions_ret, size_t *instructions_length, int *error) {

	size_t i;

	uint32_t current_insn = 0;
	size_t insn_len = 0;

	Stack_t stack;

	Instruction_t *instructions;

	if(code == NULL || instructions_ret == NULL) {
		if(error != NULL)
			*error = E_GENERIC_COMPILE;
		return;
	}

	stack_Create(&stack);

	//lazy optimization here, this only removes non insn characters.
	//when optimize flag is set, we need to do better than this to save space.
	for (i = 0; i < strlen(code); i++) {
		switch (code[i]) {
		case '>':
		case '<':
		case '+':
		case '-':
		case '.':
		case ',':
		case '[':
		case ']':
			insn_len++;
			break;
		}
	}

	instructions = malloc(sizeof(Instruction_t) * (insn_len + 1));

	for(i = 0; i < strlen(code) + 1; i++) {

		CELL_TYPE consec;
		Instruction_t insn = { -1, -1 };
		
		switch(code[i]) {
		case '>':
#ifdef OPTIMIZE
			consec = consecutive(code, i, '>');
			insn.opcode = OP_ADD_CELL_POINTER; insn.operand = consec;
			i += consec - 1;
#else
			insn.opcode = OP_ADD_CELL_POINTER; insn.operand = 1;
#endif
			break;
		case '<':
#ifdef OPTIMIZE
			consec = consecutive(code, i, '<');
			insn.opcode = OP_SUB_CELL_POINTER; insn.operand = consec;
			i += consec - 1;
#else
			insn.opcode = OP_SUB_CELL_POINTER; insn.operand = 1;
#endif
			break;
		case '+':
#ifdef OPTIMIZE
			consec = consecutive(code, i, '+');
			insn.opcode = OP_ADD_CELL_VALUE; insn.operand = consec;
			i += consec - 1;
#else
			insn.opcode = OP_ADD_CELL_VALUE; insn.operand = 1;
#endif
			break;
		case '-':
#ifdef OPTIMIZE
			consec = consecutive(code, i, '-');
			insn.opcode = OP_SUB_CELL_VALUE; insn.operand = consec;
			i += consec - 1;
#else
			insn.opcode = OP_SUB_CELL_VALUE; insn.operand = 1;
#endif
			break;
		case '.':
			insn.opcode = OP_PRINT_CELL; insn.operand = 1;
			break;
		case ',':
			insn.opcode = OP_INPUT_CELL; insn.operand = 1;
			break;
		case '[':
#ifdef OPTIMIZE
			if(i + 2 < strlen(code)) {
				if(code[i + 1] == '-' && code[i + 2] == ']') {
					insn.opcode = OP_SET_ZERO;
					insn.operand = 0;
					i += 2;
					break;
				}
			}

#endif

			if(stack.top >= MAX_STACK_SIZE) {
				if(error != NULL)
					*error = E_STACK_OVERFLOW;
				return;
			}

			insn.opcode = OP_OPEN_BRACKET;
			insn.operand = 0; //This will be fixed later in the ]

			stack_Push(&stack, current_insn);
			break;
		case ']':

			if(stack.top <= 0) {
				if(error != NULL)
					*error = E_STACK_UNDERFLOW;
				return;
			}

			insn.opcode = OP_CLOSE_BRACKET;
			//set the operand of this close bracket to the index of the corresponding open bracket + 1
			insn.operand = stack_Pop(&stack) + 1;

			if (instructions[insn.operand - 1].opcode != OP_OPEN_BRACKET) {
				if(error != NULL)
					*error = E_GENERIC_COMPILE; //something is terribly wrong here in our code, not the bf code
				return;
			}

			//set the operand of the corresponding open bracket to the index right after this close bracket
			instructions[insn.operand - 1].operand = current_insn + 1;
			break;
		case '\0':
			insn.opcode = OP_DONE;
			break;
		default:
			continue;
		}

		instructions[current_insn++] = insn;
	}

	*instructions_ret = instructions;

	if(instructions_length != NULL)
		*instructions_length = current_insn;

	if(error != NULL)
		*error = E_SUCCESS;
}

#define op(opcode) code[pc++] = opcode

#define op_2_bytes(integer)	{op(((uint32_t)(integer) >> 8) & 0xFF);/*high byte of integer*/				\
							op(((uint32_t)(integer) >> 0) & 0xFF); /*low byte of integer*/}

#define op_3_bytes(integer) {op(((uint32_t)(integer) >> 16) & 0xFF); /*highest byte of integer*/		\
							op(((uint32_t)(integer) >> 8) & 0xFF); /*middle byte of integer*/ 			\
							op(((uint32_t)(integer) >> 0) & 0xFF); /*lowest byte of integer*/}


#define op_2_bytes_little(integer)	op(((uint32_t)(integer) >> 0) & 0xFF); /*low byte of integer*/ 		\
									op(((uint32_t)(integer) >> 8) & 0xFF)  /*high byte of integer*/

#define op_3_bytes_little(integer)	{op(((uint32_t)(integer) >> 0) & 0xFF); /*lowest byte of integer*/ 	\
									op(((uint32_t)(integer) >> 8) & 0xFF); /*middle byte of integer*/ 	\
									op(((uint32_t)(integer) >> 16) & 0xFF); /*highest byte of integer*/}

/* ld hl, (***) */
#define op_load_hl_address(integer) {op(0x2A); op_3_bytes_little(integer);}
/*ld hl, *** */
#define op_load_hl(integer) {op(0x21); op_3_bytes_little(integer);}

#define op_write_hl_to_address(address) {op(0x22); op_3_bytes_little(address)}

/* ld de, *** */
#define op_load_de(integer) {op(0x11); op_3_bytes_little(integer);}

#define op_add_hl(increment, preserve_de) {								\
	if(increment == 0);													\
	/*yeah too lazy for for loops, probably a better way. curse c89*/ 	\
	else if(increment == 1) op(0x23);									\
	else if (increment == 2) {op(0x23); op(0x23);}						\
	else if (increment == 3) {op(0x23); op(0x23); op(0x23);}			\
	else if (increment == -1) op(0x2B);									\
	else if (increment == -2) {op(0x2B); op(0x2B);}						\
	else if (increment == -3) {op(0x2B); op(0x2B); op(0x2B);}			\
	else {																\
		if(preserve_de) op(0xD5); /*push de*/							\
		op_load_de(increment);	/*10 cycles*/							\
		op(0x19); /*add hl, de 11 cycles*/								\
		if(preserve_de) op(0xD1); /*pop de*/							\
	}																	\
}

#define op_add_de(increment, preserve_hl) {								\
	if(increment == 0);													\
	/*yeah too lazy for for loops, probably a better way. curse c89*/ 	\
	else if(increment == 1) op(0x13);									\
	else if (increment == 2) {op(0x13); op(0x13);}						\
	else if (increment == 3) {op(0x13); op(0x13); op(0x13);}			\
	else if (increment == -1) op(0x1B);									\
	else if (increment == -2) {op(0x1B); op(0x1B);}						\
	else if (increment == -3) {op(0x1B); op(0x1B); op(0x1B);}			\
	else {																\
		if(preserve_hl) op(0xE5); /*push hl*/							\
		op_load_hl(increment);	/*10 cycles*/							\
		op(0x19); /*add hl, de 11 cycles*/								\
		op(0xEB); /*ex de, hl*/											\
		if(preserve_hl) op(0xE1); /*pop hl*/							\
	}																	\
}

/*
	AFTER:
	hl = current cell address
	de = current cell value
*/
#define op_load_current_cell_value_into_de()	\
	if(hl != CURRENT_CELL) {					\
		if(hl != CELL_PTR)						\
			op_load_hl_address(&vm->cell_ptr);	\
		op_2_bytes(0xED17); /*ld de, (hl)*/		\
	}

enum reg_state {
	ZERO,
	CELL_PTR,
	CURRENT_CELL,
	CELL_VALUE,
	JUNK
};

/*
	.SIS 0x40 nothing(?)
	.LIS 0x49 writes 3 bytes
	.SIL 0x52 nothing(?)
	.LIL 0x5B writes 3 bytes

	we are in adl mode
	we want lis
*/

void compile_native(const Instruction_t *instructions, uint32_t instruction_length, uint8_t **native_code, size_t *native_length, struct VM *vm, int *error) {
	uint32_t i = 0;
	uint32_t pc = 0;

	uint8_t *code;

	Stack_t stack;
	uint32_t pc_backup = 0, corresponding = 0;

	enum reg_state hl = JUNK, de = JUNK;

	*native_code = malloc(1024);

	stack_Create(&stack);

	code = *native_code;

	op(0xE5); //push hl
	op(0xD5); //push de

	for (; i < instruction_length; i++) {
		Instruction_t instruction = instructions[i];
		//op(0);
		switch (instruction.opcode) {

		case OP_ADD_CELL_POINTER:

			if(hl != CELL_PTR)
				op_load_hl_address(&vm->cell_ptr);
			op_add_hl(instruction.operand * sizeof(CELL_TYPE), false);
			op_write_hl_to_address(&vm->cell_ptr);

			hl = CELL_PTR;
			de = JUNK;
			break;
		case OP_SUB_CELL_POINTER:
			if(hl != CELL_PTR)
				op_load_hl_address(&vm->cell_ptr);
			op_add_hl(-1 * instruction.operand * sizeof(CELL_TYPE), false);
			op_write_hl_to_address(&vm->cell_ptr);

			hl = CELL_PTR;
			de = JUNK;
			break;
		case OP_ADD_CELL_VALUE:

			
			op_load_current_cell_value_into_de();

			op_add_de(instruction.operand, true);

			op_2_bytes(0xED1F); //ld (hl), de

			hl = CURRENT_CELL;
			de = CELL_VALUE;
			break;
		case OP_SUB_CELL_VALUE:
		
			op_load_current_cell_value_into_de();

			op_add_de(-1 * instruction.operand, true);

			op_2_bytes(0xED1F); //ld (hl), de

			hl = CURRENT_CELL;
			de = CELL_VALUE;
			break;
		case OP_PRINT_CELL:

			op_load_current_cell_value_into_de();

			op(0xD5); //push de
			op(0xCD); //call ***
			op_3_bytes_little(bf_print_cell);
			op(0xD1); //pop de

			hl = JUNK;
			de = CELL_VALUE;
			
			break;
		case OP_INPUT_CELL:
			op(0xCD); //call ***
			op_3_bytes_little(bf_get_input);

			op(0xEB); //ex de, hl
			op_load_hl_address(&vm->cell_ptr);

			op_2_bytes(0xED1F); //ld (hl), de
			
			hl = CURRENT_CELL;
			de = CELL_VALUE;

			break;
		case OP_OPEN_BRACKET:
			if(stack.top >= MAX_STACK_SIZE) {
				if(error != NULL)
					*error = E_STACK_OVERFLOW;
				return;
			}

			op_load_current_cell_value_into_de();

			op(0xEB); //ex de, hl

			op_load_de(0); //seriously can we optimize this

			op(0xB7); //or a (reset the carry flag)

			op_2_bytes(0xED52); //sbc hl, de

			op(0xCA); //jp z, ***
			op_3_bytes_little(0); //placeholder

			stack_Push(&stack, (uint32_t)(code + pc));

			hl = CELL_VALUE;
			de = ZERO;

			break;
		case OP_CLOSE_BRACKET:

			if(stack.top <= 0) {
				if(error != NULL)
					*error = E_STACK_UNDERFLOW;
				return;
			}

			corresponding = stack_Pop(&stack);

			op_load_current_cell_value_into_de();

			op(0xEB); //ex de, hl

			op_load_de(0); //seriously can we optimize this

			op(0xB7); //or a (reset the carry flag)
			op_2_bytes(0xED52); //sbc hl, de

			op(0xC2); //jp nz, ***
			op_3_bytes_little(corresponding);

			pc_backup = pc;
			pc = corresponding - (uint32_t)code - 3;

			op_3_bytes_little((uint32_t)code + pc_backup);

			pc = pc_backup;

			hl = CELL_VALUE;
			de = ZERO;

			break;
		case OP_SET_ZERO:
			if(hl != CELL_PTR)
				op_load_hl_address(&vm->cell_ptr);

			op_load_de(0);

			op_2_bytes(0xED1F); //ld (hl), de

			hl = CURRENT_CELL;
			de = CELL_VALUE; //TODO: further optimizations here

			break;
		case OP_DONE:
			op(0xD1); //pop de
			op(0xE1); //pop hl
			op(0xC9); //ret
			break;
		}
	}

	*native_length = pc;
	
}


#ifdef __cplusplus
}
#endif