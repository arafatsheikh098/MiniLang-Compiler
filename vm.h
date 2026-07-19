#ifndef VM_H
#define VM_H

#include "tac.h"

/* ─── VM Bytecode Operations ─── */
typedef enum {
    VM_PUSH,          /* Push immediate value onto stack */
    VM_LOAD,          /* Load variable value onto stack */
    VM_STORE,         /* Pop top of stack into variable */
    VM_ADD,           /* Pop two, push sum */
    VM_SUB,           /* Pop two, push difference */
    VM_MUL,           /* Pop two, push product */
    VM_DIV,           /* Pop two, push quotient */
    VM_NEG,           /* Negate top of stack */
    VM_CMP_LT,       /* Pop two, push 1 if a < b else 0 */
    VM_CMP_GT,       /* Pop two, push 1 if a > b else 0 */
    VM_CMP_EQ,       /* Pop two, push 1 if a == b else 0 */
    VM_CMP_NEQ,      /* Pop two, push 1 if a != b else 0 */
    VM_JUMP,          /* Unconditional jump to address */
    VM_JUMP_FALSE,    /* Pop top; jump if zero */
    VM_PRINT,         /* Pop and print top of stack */
    VM_HALT,          /* Stop execution */
} VMOp;

/* ─── Single VM Instruction ─── */
typedef struct {
    VMOp  op;
    int   operand;    /* immediate value, variable index, or jump target */
    char *label;      /* label name (for unresolved jumps, NULL after resolve) */
    char *var_name;   /* variable name (for LOAD/STORE, NULL otherwise) */
} VMInstr;

/* ─── VM Program ─── */
typedef struct {
    VMInstr *code;
    int      count;
    int      capacity;
} VMProgram;

/* ─── API ─── */

/* Generate VM bytecode from TAC */
VMProgram *vm_generate(TACList *tac);

/* Print VM bytecode listing */
void vm_print(VMProgram *prog);

/* Execute the VM program */
void vm_execute(VMProgram *prog);

/* Free VM program memory */
void vm_free(VMProgram *prog);

#endif
