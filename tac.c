#include "tac.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ─── Internal helpers ─── */

static char *new_temp(TACList *list)
{
    char buf[32];
    snprintf(buf, sizeof(buf), "t%d", ++list->temp_count);
    return strdup(buf);
}

static char *new_label(TACList *list)
{
    char buf[32];
    snprintf(buf, sizeof(buf), "L%d", ++list->label_count);
    return strdup(buf);
}

static char *int_to_str(int val)
{
    char buf[32];
    snprintf(buf, sizeof(buf), "%d", val);
    return strdup(buf);
}

static TACInstr *make_instr(TACOp op, const char *result,
                            const char *arg1, const char *arg2)
{
    TACInstr *instr = calloc(1, sizeof(TACInstr));
    if (!instr) { fprintf(stderr, "tac: out of memory\n"); exit(1); }
    instr->op     = op;
    instr->result = result ? strdup(result) : NULL;
    instr->arg1   = arg1   ? strdup(arg1)   : NULL;
    instr->arg2   = arg2   ? strdup(arg2)   : NULL;
    instr->next   = NULL;
    return instr;
}

static void emit(TACList *list, TACInstr *instr)
{
    if (!list->head) {
        list->head = list->tail = instr;
    } else {
        list->tail->next = instr;
        list->tail = instr;
    }
}

/* ─── Scope-aware variable naming ─── */
/* Tracks variable → scoped name mapping so inner blocks that shadow
   an outer variable use distinct names (e.g., temp → temp$2). */

#define MAX_SCOPE_DEPTH 64
#define MAX_VARS_PER_SCOPE 64

typedef struct {
    char *original;  /* source name */
    char *scoped;    /* TAC name (may include suffix) */
} VarMapping;

typedef struct {
    VarMapping vars[MAX_VARS_PER_SCOPE];
    int count;
} ScopeFrame;

static ScopeFrame scope_stack[MAX_SCOPE_DEPTH];
static int scope_depth = 0;
static int scope_id_counter = 0;

static void tac_push_scope(void)
{
    if (scope_depth >= MAX_SCOPE_DEPTH) return;
    scope_stack[scope_depth].count = 0;
    scope_depth++;
    scope_id_counter++;
}

static void tac_pop_scope(void)
{
    if (scope_depth <= 0) return;
    scope_depth--;
    ScopeFrame *frame = &scope_stack[scope_depth];
    for (int i = 0; i < frame->count; i++) {
        free(frame->vars[i].original);
        free(frame->vars[i].scoped);
    }
    frame->count = 0;
}

/* Check if a name already exists in any enclosing scope */
static const char *lookup_scoped_name(const char *name)
{
    for (int d = scope_depth - 1; d >= 0; d--) {
        ScopeFrame *frame = &scope_stack[d];
        for (int i = 0; i < frame->count; i++) {
            if (strcmp(frame->vars[i].original, name) == 0) {
                return frame->vars[i].scoped;
            }
        }
    }
    return name;  /* not found = use original name */
}

/* Check if name exists in an outer scope (for shadowing detection) */
static int exists_in_outer_scope(const char *name)
{
    for (int d = scope_depth - 2; d >= 0; d--) {
        ScopeFrame *frame = &scope_stack[d];
        for (int i = 0; i < frame->count; i++) {
            if (strcmp(frame->vars[i].original, name) == 0) {
                return 1;
            }
        }
    }
    return 0;
}

/* Declare a variable in the current scope; returns the scoped name to use */
static const char *declare_scoped_var(const char *name)
{
    if (scope_depth <= 0) return name;
    ScopeFrame *frame = &scope_stack[scope_depth - 1];
    if (frame->count >= MAX_VARS_PER_SCOPE) return name;

    char *scoped;
    if (exists_in_outer_scope(name)) {
        /* Shadowed: create unique name */
        char buf[128];
        snprintf(buf, sizeof(buf), "%s$%d", name, scope_id_counter);
        scoped = strdup(buf);
    } else {
        scoped = strdup(name);
    }

    frame->vars[frame->count].original = strdup(name);
    frame->vars[frame->count].scoped   = scoped;
    frame->count++;
    return scoped;
}

/* ─── Expression code generation ─── */
/* Returns the name of the temp/variable holding the result */

static char *gen_expr(TACList *list, ASTNode *node);
static void gen_stmt(TACList *list, ASTNode *node);

static char *gen_expr(TACList *list, ASTNode *node)
{
    if (!node) return strdup("0");

    switch (node->kind) {
        case NODE_INT_LIT:
            return int_to_str(node->u.ival);

        case NODE_BOOL_LIT:
            return int_to_str(node->u.bval);

        case NODE_IDENT: {
            const char *scoped = lookup_scoped_name(node->u.ident.name);
            return strdup(scoped);
        }

        case NODE_BINOP: {
            char *left  = gen_expr(list, node->u.binop.left);
            char *right = gen_expr(list, node->u.binop.right);
            char *temp  = new_temp(list);

            TACOp op;
            switch (node->u.binop.op) {
                case OP_ADD: op = TAC_ADD; break;
                case OP_SUB: op = TAC_SUB; break;
                case OP_MUL: op = TAC_MUL; break;
                case OP_DIV: op = TAC_DIV; break;
                case OP_LT:  op = TAC_LT;  break;
                case OP_GT:  op = TAC_GT;  break;
                case OP_EQ:  op = TAC_EQ;  break;
                case OP_NEQ: op = TAC_NEQ; break;
            }

            emit(list, make_instr(op, temp, left, right));
            free(left);
            free(right);
            return temp;
        }

        case NODE_UNARY_MINUS: {
            char *operand = gen_expr(list, node->u.unary.operand);
            char *temp    = new_temp(list);
            emit(list, make_instr(TAC_NEG, temp, operand, NULL));
            free(operand);
            return temp;
        }

        default:
            return strdup("0");
    }
}

/* ─── Statement code generation ─── */

static void gen_stmt(TACList *list, ASTNode *node)
{
    if (!node) return;

    switch (node->kind) {
        case NODE_BLOCK:
            tac_push_scope();
            gen_stmt(list, node->u.block.stmts);
            tac_pop_scope();
            break;

        case NODE_STMT_LIST: {
            ASTNode *cur = node;
            while (cur && cur->kind == NODE_STMT_LIST) {
                gen_stmt(list, cur->u.stmt_list.stmt);
                cur = cur->u.stmt_list.next;
            }
            break;
        }

        case NODE_DECL: {
            /* Declare variable in current scope for naming */
            declare_scoped_var(node->u.ident.name);
            break;
        }

        case NODE_DECL_ASSIGN: {
            char *val = gen_expr(list, node->u.ident.init_expr);
            const char *scoped = declare_scoped_var(node->u.ident.name);
            emit(list, make_instr(TAC_ASSIGN, scoped, val, NULL));
            free(val);
            break;
        }

        case NODE_ASSIGN: {
            char *val = gen_expr(list, node->u.assign.expr);
            const char *scoped = lookup_scoped_name(node->u.assign.name);
            emit(list, make_instr(TAC_ASSIGN, scoped, val, NULL));
            free(val);
            break;
        }

        case NODE_IF: {
            char *cond = gen_expr(list, node->u.if_stmt.cond);
            char *lend = new_label(list);

            emit(list, make_instr(TAC_IF_FALSE, lend, cond, NULL));
            free(cond);

            gen_stmt(list, node->u.if_stmt.then_branch);

            emit(list, make_instr(TAC_LABEL, lend, NULL, NULL));
            free(lend);
            break;
        }

        case NODE_IF_ELSE: {
            char *cond  = gen_expr(list, node->u.if_stmt.cond);
            char *lelse = new_label(list);
            char *lend  = new_label(list);

            emit(list, make_instr(TAC_IF_FALSE, lelse, cond, NULL));
            free(cond);

            gen_stmt(list, node->u.if_stmt.then_branch);
            emit(list, make_instr(TAC_GOTO, lend, NULL, NULL));

            emit(list, make_instr(TAC_LABEL, lelse, NULL, NULL));
            free(lelse);

            gen_stmt(list, node->u.if_stmt.else_branch);

            emit(list, make_instr(TAC_LABEL, lend, NULL, NULL));
            free(lend);
            break;
        }

        case NODE_WHILE: {
            char *lstart = new_label(list);
            char *lend   = new_label(list);

            emit(list, make_instr(TAC_LABEL, lstart, NULL, NULL));

            char *cond = gen_expr(list, node->u.while_stmt.cond);
            emit(list, make_instr(TAC_IF_FALSE, lend, cond, NULL));
            free(cond);

            gen_stmt(list, node->u.while_stmt.body);

            emit(list, make_instr(TAC_GOTO, lstart, NULL, NULL));
            emit(list, make_instr(TAC_LABEL, lend, NULL, NULL));
            free(lstart);
            free(lend);
            break;
        }

        case NODE_PRINT: {
            char *val = gen_expr(list, node->u.print_stmt.expr);
            emit(list, make_instr(TAC_PRINT, NULL, val, NULL));
            free(val);
            break;
        }

        default:
            break;
    }
}

/* ─── Public API ─── */

TACList *tac_create(void)
{
    TACList *list = calloc(1, sizeof(TACList));
    if (!list) { fprintf(stderr, "tac_create: out of memory\n"); exit(1); }
    list->head        = NULL;
    list->tail        = NULL;
    list->temp_count  = 0;
    list->label_count = 0;
    return list;
}

TACList *tac_generate(ASTNode *root)
{
    TACList *list = tac_create();
    scope_depth = 0;
    scope_id_counter = 0;
    gen_stmt(list, root);
    return list;
}

const char *tac_op_str(TACOp op)
{
    switch (op) {
        case TAC_ASSIGN:   return "=";
        case TAC_ADD:      return "+";
        case TAC_SUB:      return "-";
        case TAC_MUL:      return "*";
        case TAC_DIV:      return "/";
        case TAC_NEG:      return "neg";
        case TAC_LT:       return "<";
        case TAC_GT:       return ">";
        case TAC_EQ:       return "==";
        case TAC_NEQ:      return "!=";
        case TAC_LABEL:    return "label";
        case TAC_GOTO:     return "goto";
        case TAC_IF_FALSE: return "if_false";
        case TAC_PRINT:    return "print";
    }
    return "?";
}

void tac_print(TACList *list)
{
    if (!list) return;
    int line = 1;
    for (TACInstr *i = list->head; i; i = i->next, line++) {
        printf("  %3d:  ", line);
        switch (i->op) {
            case TAC_LABEL:
                printf("%s:\n", i->result);
                break;
            case TAC_GOTO:
                printf("goto %s\n", i->result);
                break;
            case TAC_IF_FALSE:
                printf("if_false %s goto %s\n", i->arg1, i->result);
                break;
            case TAC_PRINT:
                printf("print %s\n", i->arg1);
                break;
            case TAC_ASSIGN:
                printf("%s = %s\n", i->result, i->arg1);
                break;
            case TAC_NEG:
                printf("%s = -%s\n", i->result, i->arg1);
                break;
            case TAC_ADD:
            case TAC_SUB:
            case TAC_MUL:
            case TAC_DIV:
            case TAC_LT:
            case TAC_GT:
            case TAC_EQ:
            case TAC_NEQ:
                printf("%s = %s %s %s\n",
                       i->result, i->arg1, tac_op_str(i->op), i->arg2);
                break;
        }
    }
}

void tac_write_file(TACList *list, const char *filename)
{
    FILE *fp = fopen(filename, "w");
    if (!fp) {
        fprintf(stderr, "Cannot open '%s' for writing\n", filename);
        return;
    }

    for (TACInstr *i = list->head; i; i = i->next) {
        switch (i->op) {
            case TAC_LABEL:
                fprintf(fp, "%s:\n", i->result);
                break;
            case TAC_GOTO:
                fprintf(fp, "  goto %s\n", i->result);
                break;
            case TAC_IF_FALSE:
                fprintf(fp, "  if_false %s goto %s\n", i->arg1, i->result);
                break;
            case TAC_PRINT:
                fprintf(fp, "  print %s\n", i->arg1);
                break;
            case TAC_ASSIGN:
                fprintf(fp, "  %s = %s\n", i->result, i->arg1);
                break;
            case TAC_NEG:
                fprintf(fp, "  %s = -%s\n", i->result, i->arg1);
                break;
            case TAC_ADD:
            case TAC_SUB:
            case TAC_MUL:
            case TAC_DIV:
            case TAC_LT:
            case TAC_GT:
            case TAC_EQ:
            case TAC_NEQ:
                fprintf(fp, "  %s = %s %s %s\n",
                        i->result, i->arg1, tac_op_str(i->op), i->arg2);
                break;
        }
    }

    fclose(fp);
}

void tac_free(TACList *list)
{
    if (!list) return;
    TACInstr *cur = list->head;
    while (cur) {
        TACInstr *tmp = cur;
        cur = cur->next;
        free(tmp->result);
        free(tmp->arg1);
        free(tmp->arg2);
        free(tmp);
    }
    free(list);
}
