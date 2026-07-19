#include "vm.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define VM_STACK_SIZE 1024
#define VM_MAX_VARS   256

/* ─── Variable storage for VM execution ─── */
typedef struct {
    char *name;
    int   value;
} VMVar;

/* ─── Helper: check if string is a numeric literal ─── */
static int vm_is_literal(const char *s)
{
    if (!s) return 0;
    if (*s == '-') s++;
    if (*s == '\0') return 0;
    while (*s) {
        if (!isdigit((unsigned char)*s)) return 0;
        s++;
    }
    return 1;
}

/* ─── VM Program construction helpers ─── */

static VMProgram *vm_create(void)
{
    VMProgram *prog = calloc(1, sizeof(VMProgram));
    if (!prog) { fprintf(stderr, "vm: out of memory\n"); exit(1); }
    prog->capacity = 256;
    prog->code = calloc(prog->capacity, sizeof(VMInstr));
    if (!prog->code) { fprintf(stderr, "vm: out of memory\n"); exit(1); }
    prog->count = 0;
    return prog;
}

static void vm_emit(VMProgram *prog, VMOp op, int operand,
                    const char *label, const char *var_name)
{
    if (prog->count >= prog->capacity) {
        prog->capacity *= 2;
        prog->code = realloc(prog->code, prog->capacity * sizeof(VMInstr));
        if (!prog->code) { fprintf(stderr, "vm: out of memory\n"); exit(1); }
    }
    VMInstr *instr = &prog->code[prog->count++];
    instr->op       = op;
    instr->operand  = operand;
    instr->label    = label    ? strdup(label)    : NULL;
    instr->var_name = var_name ? strdup(var_name) : NULL;
}

/* ─── Emit code to load a TAC operand onto the stack ─── */
static void emit_load_operand(VMProgram *prog, const char *operand)
{
    if (!operand) return;
    if (vm_is_literal(operand)) {
        vm_emit(prog, VM_PUSH, atoi(operand), NULL, NULL);
    } else {
        vm_emit(prog, VM_LOAD, 0, NULL, operand);
    }
}

/* ─── Generate VM bytecode from TAC ─── */
VMProgram *vm_generate(TACList *tac)
{
    VMProgram *prog = vm_create();

    for (TACInstr *i = tac->head; i; i = i->next) {
        switch (i->op) {
            case TAC_LABEL:
                /* Labels become markers; we record the label for jump resolution */
                vm_emit(prog, VM_HALT, 0, i->result, NULL);
                /* Mark last instruction as label placeholder (op overwritten below) */
                prog->code[prog->count - 1].op = (VMOp)-1;  /* sentinel */
                break;

            case TAC_GOTO:
                vm_emit(prog, VM_JUMP, 0, i->result, NULL);
                break;

            case TAC_IF_FALSE:
                emit_load_operand(prog, i->arg1);
                vm_emit(prog, VM_JUMP_FALSE, 0, i->result, NULL);
                break;

            case TAC_PRINT:
                emit_load_operand(prog, i->arg1);
                vm_emit(prog, VM_PRINT, 0, NULL, NULL);
                break;

            case TAC_ASSIGN:
                emit_load_operand(prog, i->arg1);
                vm_emit(prog, VM_STORE, 0, NULL, i->result);
                break;

            case TAC_NEG:
                emit_load_operand(prog, i->arg1);
                vm_emit(prog, VM_NEG, 0, NULL, NULL);
                vm_emit(prog, VM_STORE, 0, NULL, i->result);
                break;

            case TAC_ADD:
                emit_load_operand(prog, i->arg1);
                emit_load_operand(prog, i->arg2);
                vm_emit(prog, VM_ADD, 0, NULL, NULL);
                vm_emit(prog, VM_STORE, 0, NULL, i->result);
                break;

            case TAC_SUB:
                emit_load_operand(prog, i->arg1);
                emit_load_operand(prog, i->arg2);
                vm_emit(prog, VM_SUB, 0, NULL, NULL);
                vm_emit(prog, VM_STORE, 0, NULL, i->result);
                break;

            case TAC_MUL:
                emit_load_operand(prog, i->arg1);
                emit_load_operand(prog, i->arg2);
                vm_emit(prog, VM_MUL, 0, NULL, NULL);
                vm_emit(prog, VM_STORE, 0, NULL, i->result);
                break;

            case TAC_DIV:
                emit_load_operand(prog, i->arg1);
                emit_load_operand(prog, i->arg2);
                vm_emit(prog, VM_DIV, 0, NULL, NULL);
                vm_emit(prog, VM_STORE, 0, NULL, i->result);
                break;

            case TAC_LT:
                emit_load_operand(prog, i->arg1);
                emit_load_operand(prog, i->arg2);
                vm_emit(prog, VM_CMP_LT, 0, NULL, NULL);
                vm_emit(prog, VM_STORE, 0, NULL, i->result);
                break;

            case TAC_GT:
                emit_load_operand(prog, i->arg1);
                emit_load_operand(prog, i->arg2);
                vm_emit(prog, VM_CMP_GT, 0, NULL, NULL);
                vm_emit(prog, VM_STORE, 0, NULL, i->result);
                break;

            case TAC_EQ:
                emit_load_operand(prog, i->arg1);
                emit_load_operand(prog, i->arg2);
                vm_emit(prog, VM_CMP_EQ, 0, NULL, NULL);
                vm_emit(prog, VM_STORE, 0, NULL, i->result);
                break;

            case TAC_NEQ:
                emit_load_operand(prog, i->arg1);
                emit_load_operand(prog, i->arg2);
                vm_emit(prog, VM_CMP_NEQ, 0, NULL, NULL);
                vm_emit(prog, VM_STORE, 0, NULL, i->result);
                break;
        }
    }

    /* Add HALT at end */
    vm_emit(prog, VM_HALT, 0, NULL, NULL);

    /* ─── Resolve labels ─── */
    /* First, build a label → address map */
    typedef struct { char *name; int addr; } LabelEntry;
    LabelEntry labels[512];
    int label_count = 0;

    for (int j = 0; j < prog->count; j++) {
        if ((int)prog->code[j].op == -1 && prog->code[j].label) {
            labels[label_count].name = prog->code[j].label;
            labels[label_count].addr = j;
            label_count++;
        }
    }

    /* Resolve JUMP and JUMP_FALSE targets */
    for (int j = 0; j < prog->count; j++) {
        VMInstr *instr = &prog->code[j];
        if ((instr->op == VM_JUMP || instr->op == VM_JUMP_FALSE) && instr->label) {
            for (int k = 0; k < label_count; k++) {
                if (strcmp(instr->label, labels[k].name) == 0) {
                    instr->operand = labels[k].addr;
                    break;
                }
            }
        }
    }

    /* Remove label sentinel instructions by converting them to no-ops.
       We keep them in-place so jump targets remain valid —
       the executor simply skips them. */

    return prog;
}

/* ─── Print VM bytecode listing ─── */

static const char *vm_op_name(VMOp op)
{
    switch (op) {
        case VM_PUSH:       return "PUSH";
        case VM_LOAD:       return "LOAD";
        case VM_STORE:      return "STORE";
        case VM_ADD:        return "ADD";
        case VM_SUB:        return "SUB";
        case VM_MUL:        return "MUL";
        case VM_DIV:        return "DIV";
        case VM_NEG:        return "NEG";
        case VM_CMP_LT:    return "CMP_LT";
        case VM_CMP_GT:    return "CMP_GT";
        case VM_CMP_EQ:    return "CMP_EQ";
        case VM_CMP_NEQ:   return "CMP_NEQ";
        case VM_JUMP:       return "JUMP";
        case VM_JUMP_FALSE: return "JUMP_FALSE";
        case VM_PRINT:      return "PRINT";
        case VM_HALT:       return "HALT";
    }
    return "???";
}

void vm_print(VMProgram *prog)
{
    if (!prog) return;
    for (int j = 0; j < prog->count; j++) {
        VMInstr *instr = &prog->code[j];

        /* Skip label sentinels in listing — show them as labels */
        if ((int)instr->op == -1) {
            printf("  %3d:  %s:\n", j, instr->label ? instr->label : "???");
            continue;
        }

        printf("  %3d:  %-12s", j, vm_op_name(instr->op));

        switch (instr->op) {
            case VM_PUSH:
                printf(" %d", instr->operand);
                break;
            case VM_LOAD:
            case VM_STORE:
                printf(" %s", instr->var_name ? instr->var_name : "?");
                break;
            case VM_JUMP:
            case VM_JUMP_FALSE:
                printf(" @%d", instr->operand);
                if (instr->label) printf("  (%s)", instr->label);
                break;
            case VM_HALT:
                break;
            default:
                break;
        }
        printf("\n");
    }
}

/* ─── Execute VM Program ─── */

void vm_execute(VMProgram *prog)
{
    if (!prog || prog->count == 0) return;

    int stack[VM_STACK_SIZE];
    int sp = -1;  /* stack pointer */

    VMVar vars[VM_MAX_VARS];
    int var_count = 0;

    int pc = 0;  /* program counter */

    while (pc < prog->count) {
        VMInstr *instr = &prog->code[pc];

        /* Skip label sentinels */
        if ((int)instr->op == -1) {
            pc++;
            continue;
        }

        switch (instr->op) {
            case VM_PUSH:
                stack[++sp] = instr->operand;
                break;

            case VM_LOAD: {
                int idx = -1;
                for (int v = 0; v < var_count; v++) {
                    if (strcmp(vars[v].name, instr->var_name) == 0) { idx = v; break; }
                }
                if (idx < 0) {
                    idx = var_count++;
                    vars[idx].name = strdup(instr->var_name);
                    vars[idx].value = 0;
                }
                stack[++sp] = vars[idx].value;
                break;
            }

            case VM_STORE: {
                int idx = -1;
                for (int v = 0; v < var_count; v++) {
                    if (strcmp(vars[v].name, instr->var_name) == 0) { idx = v; break; }
                }
                if (idx < 0) {
                    idx = var_count++;
                    vars[idx].name = strdup(instr->var_name);
                    vars[idx].value = 0;
                }
                vars[idx].value = stack[sp--];
                break;
            }

            case VM_ADD: {
                int b = stack[sp--];
                int a = stack[sp--];
                stack[++sp] = a + b;
                break;
            }

            case VM_SUB: {
                int b = stack[sp--];
                int a = stack[sp--];
                stack[++sp] = a - b;
                break;
            }

            case VM_MUL: {
                int b = stack[sp--];
                int a = stack[sp--];
                stack[++sp] = a * b;
                break;
            }

            case VM_DIV: {
                int b = stack[sp--];
                int a = stack[sp--];
                if (b == 0) {
                    fprintf(stderr, "Runtime error: division by zero\n");
                    goto cleanup;
                }
                stack[++sp] = a / b;
                break;
            }

            case VM_NEG:
                stack[sp] = -stack[sp];
                break;

            case VM_CMP_LT: {
                int b = stack[sp--];
                int a = stack[sp--];
                stack[++sp] = (a < b) ? 1 : 0;
                break;
            }

            case VM_CMP_GT: {
                int b = stack[sp--];
                int a = stack[sp--];
                stack[++sp] = (a > b) ? 1 : 0;
                break;
            }

            case VM_CMP_EQ: {
                int b = stack[sp--];
                int a = stack[sp--];
                stack[++sp] = (a == b) ? 1 : 0;
                break;
            }

            case VM_CMP_NEQ: {
                int b = stack[sp--];
                int a = stack[sp--];
                stack[++sp] = (a != b) ? 1 : 0;
                break;
            }

            case VM_JUMP:
                pc = instr->operand;
                continue;  /* skip pc++ */

            case VM_JUMP_FALSE: {
                int val = stack[sp--];
                if (val == 0) {
                    pc = instr->operand;
                    continue;  /* skip pc++ */
                }
                break;
            }

            case VM_PRINT:
                printf("%d\n", stack[sp--]);
                break;

            case VM_HALT:
                goto cleanup;
        }
        pc++;
    }

cleanup:
    /* Free variable names */
    for (int v = 0; v < var_count; v++) {
        free(vars[v].name);
    }
}

/* ─── Free VM Program ─── */

void vm_free(VMProgram *prog)
{
    if (!prog) return;
    for (int j = 0; j < prog->count; j++) {
        free(prog->code[j].label);
        free(prog->code[j].var_name);
    }
    free(prog->code);
    free(prog);
}
