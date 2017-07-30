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


#define op(opcode) {if(pc >= MAX_INSN) {*error = E_OUT_OF_MEMORY; return;} (*native_code)[pc++] = opcode;}


#define op_2_bytes(integer)         {op(((unsigned int)(integer) >> 8) & 0xFF);     /*high byte of integer*/    \
                                    op(((unsigned int)(integer) >> 0) & 0xFF);}     /*low byte of integer*/

#define op_3_bytes(integer)         {op(((unsigned int)(integer) >> 16) & 0xFF);    /*highest byte of integer*/ \
                                    op(((unsigned int)(integer) >> 8) & 0xFF);      /*middle byte of integer*/  \
                                    op(((unsigned int)(integer) >> 0) & 0xFF);}     /*lowest byte of integer*/

#define op_2_bytes_little(integer)  {op(((unsigned int)(integer) >> 0) & 0xFF);     /*low byte of integer*/     \
                                    op(((unsigned int)(integer) >> 8) & 0xFF)}      /*high byte of integer*/

#define op_3_bytes_little(integer)  {op(((unsigned int)(integer) >> 0) & 0xFF);     /*lowest byte of integer*/  \
                                    op(((unsigned int)(integer) >> 8) & 0xFF);      /*middle byte of integer*/  \
                                    op(((unsigned int)(integer) >> 16) & 0xFF);}    /*highest byte of integer*/


#define op_jp_z(jump_address)   {op(0xCA); op_3_bytes_little(jump_address);}        /*jp z, *** */
#define op_jp_nz(jump_address)  {op(0xC2); op_3_bytes_little(jump_address);}        /*jp nz, *** */

#define op_load_hl_address(integer) {op(0x2A); op_3_bytes_little(integer);}         /* ld hl, (***) */
#define op_load_hl(integer) {op(0x21); op_3_bytes_little(integer);}                 /*ld hl, *** */
    
#define op_write_hl_to_address(address) {op(0x22); op_3_bytes_little(address)}      /*ld (***), hl*/
    
#define op_load_de(integer) {op(0x11); op_3_bytes_little(integer);}                 /* ld de, *** */

/*We can do optimizations on this, but the toolchain compiler does not support a line that long...*/
#define op_load_bc(integer) {op(0x01); op_3_bytes_little(integer);}                 /* ld bc, *** */

#define op_inc_hl() op(0x23) /*inc hl*/
#define op_dec_hl() op(0x2B) /*dec hl*/

#define op_inc_de() op(0x13) /*inc de*/
#define op_dec_de() op(0x1B) /*dec de*/


/*
Adds increment to hl register.
BEFORE:
    hl = starting value
AFTER:
    hl = starting value + increment
    bc = increment (unless preserve_bc)
*/
#define op_add_hl(increment, preserve_bc) {                                         \
    if(increment == 0);                                                             \
    /*yeah too lazy for for loops, probably a better way. curse c89*/               \
    else if(increment == (CELL_TYPE)1) {op_inc_hl();}                               \
    else if (increment == (CELL_TYPE)2) {op_inc_hl(); op_inc_hl();}                 \
    else if (increment == (CELL_TYPE)3) {op_inc_hl(); op_inc_hl(); op_inc_hl();}    \
    else if (increment == (CELL_TYPE)-1) {op_dec_hl()}                              \
    else if (increment == (CELL_TYPE)-2) {op_dec_hl(); op_dec_hl();}                \
    else if (increment == (CELL_TYPE)-3) {op_dec_hl(); op_dec_hl(); op_dec_hl();}   \
    else {                                                                          \
        if(preserve_bc) op(0xC5); /*push bc*/                                       \
        op_load_bc(increment);  /*10 cycles*/                                       \
        op(0x09); /*add hl, bc 11 cycles*/                                          \
        if(preserve_bc) op(0xC1); /*pop bc*/                                        \
    }                                                                               \
}

/*
Adds increment to de register.
BEFORE:
    de = starting value
AFTER:
    de = starting value + increment
    bc = increment (unless preserve_bc)
*/
#define op_add_de(increment, preserve_bc) {                                         \
    if(increment == 0);                                                             \
    /*yeah too lazy for for loops, probably a better way. curse c89*/               \
    else if(increment == (CELL_TYPE)1) {op_inc_de();}                               \
    else if (increment == (CELL_TYPE)2) {op_inc_de(); op_inc_de();}                 \
    else if (increment == (CELL_TYPE)3) {op_inc_de(); op_inc_de(); op_inc_de();}    \
    else if (increment == (CELL_TYPE)-1) {op_dec_de();}                             \
    else if (increment == (CELL_TYPE)-2) {op_dec_de(); op_dec_de();}                \
    else if (increment == (CELL_TYPE)-3) {op_dec_de(); op_dec_de(); op_dec_de();}   \
    else {                                                                          \
        op(0xEB); /*ex de, hl*/                                                     \
        op_add_hl(increment, preserve_bc);                                          \
        op(0xEB); /*ex de, hl*/                                                     \
    }                                                                               \
}

/*
Sets z flag if hl is zero
BEFORE:
    hl = value to check
*/
#define op_check_hl_zero() {            \
    op(0x19);           /*add hl, de*/  \
    op(0xB7);           /*or a*/        \
    op_2_bytes(0xED52); /*sbc hl, de*/  \
}

/*
Sets z flag if de is zero
BEFORE:
    de = value to check
*/
#define op_check_de_zero() {    \
    op(0xEB); /*ex de, hl*/     \
    op_check_hl_zero();         \
    op(0xEB); /*ex de, hl*/     \
}

/*
Loads the current cell address into hl
AFTER:
    hl = current cell address
*/
#define op_load_cell_address_hl() { \
    if(hl != CELL_PTR) {                    \
        op_load_hl_address(&mem->cell_ptr); \
        hl = CELL_PTR;                      \
    }                                       \
}

/*
Loads the value of the cell pointed to in hl into de
BEFORE:
    hl = current cell address
AFTER:
    de = current cell value
*/
#define op_load_cell_value_de() {           \
    if(de != CELL_VALUE) {                  \
        op_2_bytes(0xED17); /*ld de, (hl)*/ \
        de = CELL_VALUE;                    \
    }                                       \
}

/*
Loads the value of the cell pointed to in hl into de
BEFORE:
    hl = current cell address
AFTER:
    hl = current cell value
*/
#define op_load_cell_value_hl() {       \
    if(hl != CELL_VALUE) {                      \
        op_2_bytes(0xED27); /*ld hl, (hl)*/     \
        hl = CELL_VALUE;                        \
    }                                           \
}

/*
Saves de into (hl)
BEFORE:
    hl = current cell address
    de = cell value
*/
#define op_save_cell_value_de() op_2_bytes(0xED1F); /*ld (hl), de*/

enum reg_state {
    JUNK,
    CELL_PTR,
    CELL_VALUE
};


#define MAX_INSN 45644
uint8_t insns[MAX_INSN]; //because malloc() can't allocate this much apparently

/*
    .SIS 0x40
    .LIS 0x49
    .SIL 0x52
    .LIL 0x5B
*/
void compile_native(const char *code, size_t len, bool optimize, uint8_t **native_code, size_t *native_length, struct Memory *mem, int *error) {
	unsigned int i = 0, pc = 0;

    enum reg_state hl = JUNK, de = JUNK;
    unsigned int bc;

    Stack_t stack;
    stack_Create(&stack);

    *native_code = insns;

    while(i < len) {
        size_t consumed;

        //have to split these two lines up due to a compiler error lol
        //P3: Internal Error(0x83BAF1): \ Please contact Technical Support \ make: *** [obj/compiler.obj] Error -1
        Instruction_t insn;
        insn = next_insn(code, len, i, optimize, &consumed);

        switch (insn.opcode) {
        case OP_ADD_CELL_POINTER:

            op_load_cell_address_hl();
            op_add_hl(insn.operand * sizeof(CELL_TYPE), false);
            op_write_hl_to_address(&mem->cell_ptr);

            hl = CELL_PTR;
            de = JUNK;

            break;
        case OP_ADD_CELL_VALUE:

            if(hl == CELL_VALUE) {
                op_add_hl(insn.operand, false);
                op(0xEB); //ex de, hl
                hl = de;
                op_load_cell_address_hl();
            } else if(de == CELL_VALUE) {
                op_load_cell_address_hl();
                op_add_de(insn.operand, false);
            } else {
                op_load_cell_address_hl();
                op_load_cell_value_de();
                op_add_de(insn.operand, false);
            }

            op_save_cell_value_de();

            hl = CELL_PTR;
            de = CELL_VALUE;

            break;
        case OP_PRINT_CELL:

            op_load_cell_address_hl();
            op_load_cell_value_de();

            op(0xD5); //push de
            op(0xCD); //call ***
            op_3_bytes_little(bf_print_cell);
            op(0xD1); //pop de

            hl = JUNK;
            de = CELL_VALUE;

            break;
        case OP_INPUT_CELL:

            //call the get input and store the value in de
            op(0xCD); //call ***
            op_3_bytes_little(bf_get_input);

            op(0xEB); //ex de, hl

            //get the current cell ptr and store de in it
            op_load_cell_address_hl();
            op_save_cell_value_de();
            
            hl = CELL_PTR;
            de = CELL_VALUE;

            break;
        case OP_OPEN_BRACKET:

            if(stack.top >= MAX_STACK_SIZE) {
                if(error != NULL)
                    *error = E_STACK_OVERFLOW;
                return;
            }

            if(de == CELL_VALUE) {
                op(0xEB); //ex de, hl
            } else if(hl != CELL_VALUE) {
                op_load_cell_address_hl();
                op_load_cell_value_hl();
            }
            
            op_check_hl_zero();
            op_jp_z(0);

            //push this address so the corresponding ] can fix the 3 byte jump address
            //stack_Push(&stack, de);
            stack_Push(&stack, (unsigned int)*native_code + pc);

            //these values have to remain the same as the corresponding ] for when we jump
            hl = CELL_VALUE;
            //de = JUNK;

            break;
        case OP_CLOSE_BRACKET: {

            unsigned int pc_backup = 0, corresponding = 0;

            if(stack.top <= 0) {
                if(error != NULL)
                    *error = E_STACK_UNDERFLOW;
                return;
            }

            corresponding = stack_Pop(&stack);

            if(de == CELL_VALUE) {
                op(0xEB); //ex de, hl
            } else if(hl != CELL_VALUE) {
                op_load_cell_address_hl();
                op_load_cell_value_hl();
            }
            
            op_check_hl_zero();
            op_jp_nz(corresponding);

            //Go back and correct the 3 jump bytes of the corresponding [
            pc_backup = pc;
            pc = corresponding - (unsigned int)*native_code - 3;
            op_3_bytes_little((unsigned int)*native_code + pc_backup);
            pc = pc_backup;

            //these values have to remain the same as the corresponding [ for when we jump
            hl = CELL_VALUE;
            //de = JUNK;

            break;
        }
        case OP_SET_ZERO:

            if(hl != CELL_PTR)
                op_load_hl_address(&mem->cell_ptr);
            op_load_de(0);
            op_2_bytes(0xED1F); //ld (hl), de

            hl = CELL_PTR;
            de = CELL_VALUE;

            break;
        }

        i += consumed;
    }

    op(0xC9); //ret

    *native_length = pc;
    *error = E_SUCCESS;
}

#ifdef __cplusplus
}
#endif