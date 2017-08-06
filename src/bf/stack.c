#ifdef __cplusplus
extern "C" {
#endif

#include "stack.h"

void stack_Create(Stack_t *stack) {
    stack->top = 0;
}
void stack_Push(Stack_t *stack, STACK_TYPE value) {
    stack->stack[stack->top++] = value;
}
STACK_TYPE stack_Pop(Stack_t *stack) {
    return stack->stack[--stack->top]; //probably should set to null...
}
STACK_TYPE stack_Peek(Stack_t *stack) {
    return stack->stack[stack->top];
}

#ifdef __cplusplus
}
#endif