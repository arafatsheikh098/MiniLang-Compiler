#include "semantic.h"
#include "symbol_table.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

static int error_count = 0;

static void sem_error(int lineno, const char *fmt, ...)
{
    va_list args;
    fprintf(stderr, "Semantic error (line %d): ", lineno);
    va_start(args, fmt);
    vfprintf(stderr, fmt, args);
    va_end(args);
    fprintf(stderr, "\n");
    error_count++;
}

/* Forward declaration */
static void analyse_node(ASTNode *node);
static DataType analyse_expr(ASTNode *node);

static DataType analyse_expr(ASTNode *node)
{
    if (!node) return TYPE_UNKNOWN;

    switch (node->kind) {
        case NODE_INT_LIT:
            node->dtype = TYPE_INT;
            return TYPE_INT;

        case NODE_BOOL_LIT:
            node->dtype = TYPE_BOOL;
            return TYPE_BOOL;

        case NODE_IDENT: {
            Symbol *sym = sym_lookup(node->u.ident.name);
            if (!sym) {
                sem_error(node->lineno,
                          "undeclared identifier '%s'", node->u.ident.name);
                node->dtype = TYPE_UNKNOWN;
                return TYPE_UNKNOWN;
            }
            node->dtype = sym->type;
            return sym->type;
        }

        case NODE_BINOP: {
            DataType lt = analyse_expr(node->u.binop.left);
            DataType rt = analyse_expr(node->u.binop.right);

            BinOp op = node->u.binop.op;

            if (op == OP_ADD || op == OP_SUB || op == OP_MUL || op == OP_DIV) {
                /* Arithmetic: both must be int, result is int */
                if (lt != TYPE_INT && lt != TYPE_UNKNOWN)
                    sem_error(node->lineno,
                              "left operand of '%s' must be int, got %s",
                              binop_str(op), datatype_str(lt));
                if (rt != TYPE_INT && rt != TYPE_UNKNOWN)
                    sem_error(node->lineno,
                              "right operand of '%s' must be int, got %s",
                              binop_str(op), datatype_str(rt));

                /* Check literal division by zero */
                if (op == OP_DIV && node->u.binop.right->kind == NODE_INT_LIT
                    && node->u.binop.right->u.ival == 0) {
                    sem_error(node->lineno, "division by zero");
                }
                node->dtype = TYPE_INT;
                return TYPE_INT;
            }

            if (op == OP_LT || op == OP_GT || op == OP_EQ || op == OP_NEQ) {
                /* Relational: both must be int, result is bool */
                if (lt != TYPE_INT && lt != TYPE_UNKNOWN)
                    sem_error(node->lineno,
                              "left operand of '%s' must be int, got %s",
                              binop_str(op), datatype_str(lt));
                if (rt != TYPE_INT && rt != TYPE_UNKNOWN)
                    sem_error(node->lineno,
                              "right operand of '%s' must be int, got %s",
                              binop_str(op), datatype_str(rt));
                node->dtype = TYPE_BOOL;
                return TYPE_BOOL;
            }

            node->dtype = TYPE_UNKNOWN;
            return TYPE_UNKNOWN;
        }

        case NODE_UNARY_MINUS: {
            DataType t = analyse_expr(node->u.unary.operand);
            if (t != TYPE_INT && t != TYPE_UNKNOWN)
                sem_error(node->lineno,
                          "operand of unary '-' must be int, got %s",
                          datatype_str(t));
            node->dtype = TYPE_INT;
            return TYPE_INT;
        }

        default:
            return TYPE_UNKNOWN;
    }
}

static void analyse_node(ASTNode *node)
{
    if (!node) return;

    switch (node->kind) {
        case NODE_BLOCK:
            sym_push_scope();
            analyse_node(node->u.block.stmts);
            sym_pop_scope();
            break;

        case NODE_STMT_LIST: {
            ASTNode *cur = node;
            while (cur && cur->kind == NODE_STMT_LIST) {
                analyse_node(cur->u.stmt_list.stmt);
                cur = cur->u.stmt_list.next;
            }
            break;
        }

        case NODE_DECL: {
            const char *name = node->u.ident.name;
            DataType dt = node->u.ident.decl_type;
            if (sym_declare(name, dt, node->lineno) < 0) {
                sem_error(node->lineno,
                          "duplicate declaration of '%s' in same scope", name);
            }
            break;
        }

        case NODE_DECL_ASSIGN: {
            const char *name = node->u.ident.name;
            DataType dt = node->u.ident.decl_type;
            DataType init_type = analyse_expr(node->u.ident.init_expr);

            if (init_type != TYPE_UNKNOWN && init_type != dt) {
                sem_error(node->lineno,
                          "type mismatch in declaration of '%s': "
                          "declared %s but initializer is %s",
                          name, datatype_str(dt), datatype_str(init_type));
            }

            if (sym_declare(name, dt, node->lineno) < 0) {
                sem_error(node->lineno,
                          "duplicate declaration of '%s' in same scope", name);
            }
            break;
        }

        case NODE_ASSIGN: {
            const char *name = node->u.assign.name;
            Symbol *sym = sym_lookup(name);
            if (!sym) {
                sem_error(node->lineno,
                          "assignment to undeclared variable '%s'", name);
            } else {
                DataType expr_type = analyse_expr(node->u.assign.expr);
                if (expr_type != TYPE_UNKNOWN && expr_type != sym->type) {
                    sem_error(node->lineno,
                              "type mismatch in assignment to '%s': "
                              "variable is %s but expression is %s",
                              name, datatype_str(sym->type),
                              datatype_str(expr_type));
                }
            }
            break;
        }

        case NODE_IF:
        case NODE_IF_ELSE: {
            DataType cond_type = analyse_expr(node->u.if_stmt.cond);
            if (cond_type != TYPE_BOOL && cond_type != TYPE_UNKNOWN)
                sem_error(node->lineno,
                          "condition of 'if' must be bool, got %s",
                          datatype_str(cond_type));
            analyse_node(node->u.if_stmt.then_branch);
            if (node->u.if_stmt.else_branch)
                analyse_node(node->u.if_stmt.else_branch);
            break;
        }

        case NODE_WHILE: {
            DataType cond_type = analyse_expr(node->u.while_stmt.cond);
            if (cond_type != TYPE_BOOL && cond_type != TYPE_UNKNOWN)
                sem_error(node->lineno,
                          "condition of 'while' must be bool, got %s",
                          datatype_str(cond_type));
            analyse_node(node->u.while_stmt.body);
            break;
        }

        case NODE_PRINT:
            analyse_expr(node->u.print_stmt.expr);
            break;

        default:
            break;
    }
}

int sem_analyse(ASTNode *root)
{
    error_count = 0;
    sym_init();
    analyse_node(root);
    return error_count;
}
