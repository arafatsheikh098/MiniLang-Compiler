#include <stdlib.h>
#include <string.h>
#include "codegen.h"

static FILE *tac_out = NULL;
static int temp_count = 1;
static int label_count = 1;

void init_codegen(FILE *out) {
    tac_out = out;
    temp_count = 1;
    label_count = 1;
}

static char* new_temp() {
    char *t = (char*)malloc(16);
    sprintf(t, "T%d", temp_count++);
    return t;
}

static char* new_label() {
    char *l = (char*)malloc(16);
    sprintf(l, "L%d", label_count++);
    return l;
}

static char* op_to_str(Operator op) {
    switch (op) {
        case OP_ADD: return "+";
        case OP_SUB: return "-";
        case OP_MUL: return "*";
        case OP_DIV: return "/";
        case OP_EQ:  return "==";
        case OP_NEQ: return "!=";
        case OP_LT:  return "<";
        case OP_GT:  return ">";
        case OP_LE:  return "<=";
        case OP_GE:  return ">=";
        case OP_AND: return "&&";
        case OP_OR:  return "||";
        default: return "?";
    }
}
