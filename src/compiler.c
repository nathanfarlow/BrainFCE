#ifdef __cplusplus
extern "C" {
#endif

#include <stdlib.h> //for malloc
#include <string.h> //for memset

#include "compiler.h"

#include "stack.h"

void mem_Create(struct Memory *mem) {
	memset(mem->cells, 0, sizeof(mem->cells));
	mem->cell_ptr = mem->cells;
}

/*
	This function basically takes strings of the same type of insn and changes it into one instruction

	>><><<>>> = OP_ADD_CELL_POINTER 3
	++-+-+++- = OP_ADD_CELL_VALUE -1 (but turns into an unsigned number)
*/
void scan_for_operand(const char *code, size_t len, unsigned int index, char add, char sub, CELL_TYPE *operand, size_t *consumed) {

	*operand = 0;
	*consumed = 0;

	for(; index < len; index++) {
		char c = code[index];

		//this switch will allow us to ignore non-bf characters
		switch (c) {
		case '>':
		case '<':
		case '+':
		case '-':
		case '.':
		case ',':
		case '[':
		case ']':
			if (c == add)
				(*operand)++;
			else if (c == sub)
				(*operand)--;
			else
				return;
		}

		(*consumed)++;
	}
}


#define index_equ(check_index, character) (check_index < len && code[check_index] == character)

Instruction_t next_insn(const char *code, size_t len, unsigned int index, bool optimize, size_t *consumed) {
	Instruction_t insn = {0, 0};

	unsigned int loops = 0;

	*consumed = 1;

loop:
	switch (code[index]) {
	case '>':
	case '<':
		insn.opcode = OP_ADD_CELL_POINTER;

		if (optimize)
			scan_for_operand(code, len, index, '>', '<', &insn.operand, consumed);
		else
			insn.operand = code[index] == '>' ? 1 : -1;

		break;
	case '+':
	case '-':
		insn.opcode = OP_ADD_CELL_VALUE;

		if(optimize)
			scan_for_operand(code, len, index, '+', '-', &insn.operand, consumed);
		else
			insn.operand = code[index] == '+' ? 1 : -1;

		break;
	case '.':
		insn.opcode = OP_PRINT_CELL;
		break;
	case ',':
		insn.opcode = OP_INPUT_CELL;
		break;
	case '[':
		insn.opcode = OP_OPEN_BRACKET;

		if (index_equ(index + 1, '-') && index_equ(index + 2, ']')) {
			insn.opcode = OP_SET_ZERO;
			*consumed = 3;
		}

		break;
	case ']':
		insn.opcode = OP_CLOSE_BRACKET;
		break;
	default:
		index++;
		loops++;
		goto loop;
	}

	*consumed += loops;

	return insn;
}

void compile_bytecode(const char *code, size_t len, bool optimize, Instruction_t **instructions_ret, size_t *instructions_length, int *error) {

	size_t i;
	
	Stack_t stack;

	Instruction_t *instructions;
	size_t num_insns = 0;

	unsigned int current_insn = 0;

	if (code == NULL || instructions_ret == NULL) {
		if (error != NULL)
			*error = E_GENERIC_COMPILE;
		return;
	}

	stack_Create(&stack);

	//calculate the amount of instructions
	i = 0;
	while (i < len) {
		size_t consumed;

		next_insn(code, len, i, optimize, &consumed);

		num_insns++;

		i += consumed;
	}

	instructions = malloc(num_insns * sizeof(Instruction_t));

	i = 0;
	while (i < len) {
		
		size_t consumed;

		//have to split these two lines up due to a compiler error lol
		//P3: Internal Error(0x83BAF1): \ Please contact Technical Support \ make: *** [obj/compiler.obj] Error -1
		Instruction_t insn;
		insn = next_insn(code, len, i, optimize, &consumed);

		switch (insn.opcode) {
		case OP_OPEN_BRACKET:

			if (stack.top >= MAX_STACK_SIZE) {
				if (error != NULL)
					*error = E_STACK_OVERFLOW;
				return;
			}

			stack_Push(&stack, current_insn);
			break;
		case OP_CLOSE_BRACKET:
			if (stack.top <= 0) {
				if (error != NULL)
					*error = E_STACK_UNDERFLOW;
				return;
			}

			//set the operand of this close bracket to the index of the corresponding open bracket + 1
			insn.operand = stack_Pop(&stack) + 1;

			if (instructions[insn.operand - 1].opcode != OP_OPEN_BRACKET) {
				if (error != NULL)
					*error = E_GENERIC_COMPILE; //something is terribly wrong here in our code, not the bf code
				return;
			}

			//set the operand of the corresponding open bracket to the index right after this close bracket
			instructions[insn.operand - 1].operand = current_insn + 1;
			break;
		}


		instructions[current_insn++] = insn;
		i += consumed;
	}
	
	
	*instructions_ret = instructions;

	if (instructions_length != NULL)
		*instructions_length = current_insn;

	if (error != NULL)
		*error = E_SUCCESS;
}


#define op(opcode) {if(pc >= memory) {*error = E_OUT_OF_MEMORY; return;} code[pc++] = opcode;}

#define op_2_bytes(integer)	{op(((unsigned int)(integer) >> 8) & 0xFF);/*high byte of integer*/				\
							op(((unsigned int)(integer) >> 0) & 0xFF); /*low byte of integer*/}

#define op_3_bytes(integer) {op(((unsigned int)(integer) >> 16) & 0xFF); /*highest byte of integer*/		\
							op(((unsigned int)(integer) >> 8) & 0xFF); /*middle byte of integer*/ 			\
							op(((unsigned int)(integer) >> 0) & 0xFF); /*lowest byte of integer*/}


#define op_2_bytes_little(integer)	op(((unsigned int)(integer) >> 0) & 0xFF); /*low byte of integer*/ 		\
									op(((unsigned int)(integer) >> 8) & 0xFF)  /*high byte of integer*/

#define op_3_bytes_little(integer)	{op(((unsigned int)(integer) >> 0) & 0xFF); /*lowest byte of integer*/ 	\
									op(((unsigned int)(integer) >> 8) & 0xFF); /*middle byte of integer*/ 	\
									op(((unsigned int)(integer) >> 16) & 0xFF); /*highest byte of integer*/}

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
	else if(increment == (CELL_TYPE)1) {op(0x23);}						\
	else if (increment == (CELL_TYPE)2) {op(0x23); op(0x23);}			\
	else if (increment == (CELL_TYPE)3) {op(0x23); op(0x23); op(0x23);} \
	else if (increment == (CELL_TYPE)-1) {op(0x2B);}					\
	else if (increment == (CELL_TYPE)-2) {op(0x2B); op(0x2B);}			\
	else if (increment == (CELL_TYPE)-3) {op(0x2B); op(0x2B); op(0x2B);}\
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
	else if(increment == (CELL_TYPE)1) {op(0x13);}						\
	else if (increment == (CELL_TYPE)2) {op(0x13); op(0x13);}			\
	else if (increment == (CELL_TYPE)3) {op(0x13); op(0x13); op(0x13);}	\
	else if (increment == (CELL_TYPE)-1) {op(0x1B);}					\
	else if (increment == (CELL_TYPE)-2) {op(0x1B); op(0x1B);}			\
	else if (increment == (CELL_TYPE)-3) {op(0x1B); op(0x1B); op(0x1B);}\
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
			op_load_hl_address(&mem->cell_ptr);	\
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

	ld.sis ($1234), hl, loads 16-bit HL into [MB, $34, $12]

	To be exact: it stores L in MB3512 and H in MB3513

	we are in adl mode
	we want lis

	256 KB of RAM (154 KB user accessible), 4 MB of Flash ROM (3 MB user accessible)
	wtf? why can't we malloc() more?

	ld (hl),e \ inc hl \ ld (hl),d



	ld hl, current_cell_ptr
	ld de, (hl)
	ex de, hl
	ld bc, increment
	add hl, bc
	ex de, hl
	ld (hl), de

	hl = current_cell_ptr
	de = current value


	ex de, ,hl \ add hl, bc \ ex de, hl \ ld (hl), de
*/

uint8_t insns[45644];

void compile_native(const Instruction_t *instructions, uint32_t instruction_length, uint8_t **native_code, size_t *native_length, struct Memory *mem, int *error) {
	uint32_t i = 0;
	uint32_t pc = 0;

	uint8_t *code;

	Stack_t stack;
	uint32_t pc_backup = 0, corresponding = 0;

	enum reg_state hl = JUNK, de = JUNK;

	//45644 bytes needed for fractal O.o
	size_t memory = 45644;// 1024;

	//*native_code = malloc(memory);
	*native_code = insns;

	if(native_code <= 0) {
		*error = E_OUT_OF_MEMORY;
		return;
	}

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
				op_load_hl_address(&mem->cell_ptr);
			op_add_hl(instruction.operand * sizeof(CELL_TYPE), false);
			op_write_hl_to_address(&mem->cell_ptr);

			hl = CELL_PTR;
			de = JUNK;
			break;
/*
		case OP_SUB_CELL_POINTER:
			if(hl != CELL_PTR)
				op_load_hl_address(&mem->cell_ptr);
			op_add_hl(-1 * instruction.operand * sizeof(CELL_TYPE), false);
			op_write_hl_to_address(&mem->cell_ptr);

			hl = CELL_PTR;
			de = JUNK;
			break;
*/
		case OP_ADD_CELL_VALUE:

			
			op_load_current_cell_value_into_de();

			op_add_de(instruction.operand, true);

			op_2_bytes(0xED1F); //ld (hl), de

			hl = CURRENT_CELL;
			de = CELL_VALUE;
			break;
/*
		case OP_SUB_CELL_VALUE:
		
			op_load_current_cell_value_into_de();

			op_add_de(-1 * instruction.operand, true);

			op_2_bytes(0xED1F); //ld (hl), de

			hl = CURRENT_CELL;
			de = CELL_VALUE;
			break;
*/
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
			op_load_hl_address(&mem->cell_ptr);

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
				op_load_hl_address(&mem->cell_ptr);

			op_load_de(0);

			op_2_bytes(0xED1F); //ld (hl), de

			hl = CURRENT_CELL;
			de = CELL_VALUE; //TODO: further optimizations here

			break;
		}
	}

	op(0xD1); //pop de
	op(0xE1); //pop hl
	op(0xC9); //ret

	*native_length = pc;
	
}


#ifdef __cplusplus
}
#endif