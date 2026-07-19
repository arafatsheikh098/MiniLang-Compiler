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

// Generate TAC and return the temporary variable holding the result (if expression)
char* emit_node(ASTNode *node) {
    if (!node) return NULL;
    
    switch (node->type) {
        case NODE_PROGRAM:
        case NODE_BLOCK:
            emit_node(node->body);
            break;
            
        case NODE_VAR_DECL:
            // Handled during allocation normally, emitting an explicit alloc if needed
            // fprintf(tac_out, "ALLOC %s\n", node->name);
            break;
            
        case NODE_ASSIGN: {
            char *rhs = emit_node(node->right);
            fprintf(tac_out, "%s = %s\n", node->name, rhs);
            free(rhs);
            break;
        }
            
        case NODE_IDENTIFIER:
            return strdup(node->name);
            
        case NODE_INT_LITERAL: {
            char *t = new_temp();
            fprintf(tac_out, "%s = %d\n", t, node->int_val);
            return t;
        }
            
        case NODE_BOOL_LITERAL: {
            char *t = new_temp();
            fprintf(tac_out, "%s = %d\n", t, node->bool_val);
            return t;
        }
            
        case NODE_BINARY_OP: {
            char *left_t = emit_node(node->left);
            char *right_t = emit_node(node->right);
            char *res_t = new_temp();
            fprintf(tac_out, "%s = %s %s %s\n", res_t, left_t, op_to_str(node->op), right_t);
            free(left_t); free(right_t);
            return res_t;
        }
            
        case NODE_IF: {
            char *cond_t = emit_node(node->cond);
            char *L_false = new_label();
            char *L_end = node->else_body ? new_label() : NULL;
            
            fprintf(tac_out, "ifFalse %s goto %s\n", cond_t, L_false);
            free(cond_t);
            
            emit_node(node->body);
            
            if (node->else_body) {
                fprintf(tac_out, "goto %s\n", L_end);
            }
            
            fprintf(tac_out, "%s:\n", L_false);
            if (node->else_body) {
                emit_node(node->else_body);
                fprintf(tac_out, "%s:\n", L_end);
                free(L_end);
            }
            free(L_false);
            break;
        }
            
        case NODE_WHILE: {
            char *L_start = new_label();
            char *L_end = new_label();
            
            fprintf(tac_out, "%s:\n", L_start);
            char *cond_t = emit_node(node->cond);
            fprintf(tac_out, "ifFalse %s goto %s\n", cond_t, L_end);
            free(cond_t);
            
            emit_node(node->body);
            fprintf(tac_out, "goto %s\n", L_start);
            
            fprintf(tac_out, "%s:\n", L_end);
            
            free(L_start); free(L_end);
            break;
        }
            
        case NODE_PRINT: {
            char *expr_t = emit_node(node->left);
            fprintf(tac_out, "print %s\n", expr_t);
            free(expr_t);
            break;
        }
    }
    
    if (node->next) {
        emit_node(node->next);
    }
    
    return NULL;
}

void generate_tac(ASTNode *root) {
    if (!tac_out) tac_out = stdout;
    emit_node(root);
}
