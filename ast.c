#include "ast.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static ASTNode *alloc_node(NodeKind kind, int lineno)
{
    ASTNode *n = calloc(1, sizeof(ASTNode));
    if (!n) { fprintf(stderr, "ast: out of memory\n"); exit(1); }
    n->kind   = kind;
    n->dtype  = TYPE_UNKNOWN;
    n->lineno = lineno;
    return n;
}

ASTNode *ast_int_lit(int val, int lineno)
{
    ASTNode *n  = alloc_node(NODE_INT_LIT, lineno);
    n->u.ival   = val;
    n->dtype    = TYPE_INT;
    return n;
}

ASTNode *ast_bool_lit(int val, int lineno)
{
    ASTNode *n  = alloc_node(NODE_BOOL_LIT, lineno);
    n->u.bval   = val;
    n->dtype    = TYPE_BOOL;
    return n;
}

ASTNode *ast_ident(const char *name, int lineno)
{
    ASTNode *n        = alloc_node(NODE_IDENT, lineno);
    n->u.ident.name   = strdup(name);
    return n;
}

ASTNode *ast_decl(const char *name, DataType dt, int lineno)
{
    ASTNode *n            = alloc_node(NODE_DECL, lineno);
    n->u.ident.name       = strdup(name);
    n->u.ident.decl_type  = dt;
    n->u.ident.init_expr  = NULL;
    return n;
}

ASTNode *ast_decl_assign(const char *name, DataType dt, ASTNode *init, int lineno)
{
    ASTNode *n            = alloc_node(NODE_DECL_ASSIGN, lineno);
    n->u.ident.name       = strdup(name);
    n->u.ident.decl_type  = dt;
    n->u.ident.init_expr  = init;
    return n;
}

ASTNode *ast_assign(const char *name, ASTNode *expr, int lineno)
{
    ASTNode *n       = alloc_node(NODE_ASSIGN, lineno);
    n->u.assign.name = strdup(name);
    n->u.assign.expr = expr;
    return n;
}

ASTNode *ast_binop(BinOp op, ASTNode *left, ASTNode *right, int lineno)
{
    ASTNode *n        = alloc_node(NODE_BINOP, lineno);
    n->u.binop.op     = op;
    n->u.binop.left   = left;
    n->u.binop.right  = right;
    return n;
}

ASTNode *ast_unary_minus(ASTNode *operand, int lineno)
{
    ASTNode *n          = alloc_node(NODE_UNARY_MINUS, lineno);
    n->u.unary.operand  = operand;
    return n;
}

ASTNode *ast_if(ASTNode *cond, ASTNode *then_b, int lineno)
{
    ASTNode *n               = alloc_node(NODE_IF, lineno);
    n->u.if_stmt.cond        = cond;
    n->u.if_stmt.then_branch = then_b;
    n->u.if_stmt.else_branch = NULL;
    return n;
}

ASTNode *ast_if_else(ASTNode *cond, ASTNode *then_b, ASTNode *else_b, int lineno)
{
    ASTNode *n               = alloc_node(NODE_IF_ELSE, lineno);
    n->u.if_stmt.cond        = cond;
    n->u.if_stmt.then_branch = then_b;
    n->u.if_stmt.else_branch = else_b;
    return n;
}

ASTNode *ast_while(ASTNode *cond, ASTNode *body, int lineno)
{
    ASTNode *n              = alloc_node(NODE_WHILE, lineno);
    n->u.while_stmt.cond   = cond;
    n->u.while_stmt.body   = body;
    return n;
}

ASTNode *ast_print_stmt(ASTNode *expr, int lineno)
{
    ASTNode *n             = alloc_node(NODE_PRINT, lineno);
    n->u.print_stmt.expr   = expr;
    return n;
}

ASTNode *ast_block(ASTNode *stmts, int lineno)
{
    ASTNode *n        = alloc_node(NODE_BLOCK, lineno);
    n->u.block.stmts  = stmts;
    return n;
}

ASTNode *ast_stmt_list(ASTNode *stmt, ASTNode *next)
{
    ASTNode *n           = alloc_node(NODE_STMT_LIST, stmt ? stmt->lineno : 0);
    n->u.stmt_list.stmt  = stmt;
    n->u.stmt_list.next  = next;
    return n;
}

const char *binop_str(BinOp op)
{
    switch (op) {
        case OP_ADD: return "+";
        case OP_SUB: return "-";
        case OP_MUL: return "*";
        case OP_DIV: return "/";
        case OP_LT:  return "<";
        case OP_GT:  return ">";
        case OP_EQ:  return "==";
        case OP_NEQ: return "!=";
    }
    return "?";
}

const char *datatype_str(DataType dt)
{
    switch (dt) {
        case TYPE_INT:     return "int";
        case TYPE_BOOL:    return "bool";
        case TYPE_UNKNOWN: return "<unknown>";
    }
    return "?";
}

static void indent_print(int depth)
{
    for (int i = 0; i < depth * 2; i++) putchar(' ');
}

void ast_dump(ASTNode *node, int indent)
{
    if (!node) return;

    indent_print(indent);

    switch (node->kind) {
        case NODE_INT_LIT:
            printf("IntLit(%d) [%s]\n", node->u.ival, datatype_str(node->dtype));
            break;

        case NODE_BOOL_LIT:
            printf("BoolLit(%s) [%s]\n",
                   node->u.bval ? "true" : "false",
                   datatype_str(node->dtype));
            break;

        case NODE_IDENT:
            printf("Ident(%s) [%s]\n", node->u.ident.name, datatype_str(node->dtype));
            break;

        case NODE_DECL:
            printf("Decl(%s : %s)\n",
                   node->u.ident.name,
                   datatype_str(node->u.ident.decl_type));
            break;

        case NODE_DECL_ASSIGN:
            printf("DeclAssign(%s : %s)\n",
                   node->u.ident.name,
                   datatype_str(node->u.ident.decl_type));
            ast_dump(node->u.ident.init_expr, indent + 1);
            break;

        case NODE_ASSIGN:
            printf("Assign(%s)\n", node->u.assign.name);
            ast_dump(node->u.assign.expr, indent + 1);
            break;

        case NODE_BINOP:
            printf("BinOp(%s) [%s]\n", binop_str(node->u.binop.op), datatype_str(node->dtype));
            ast_dump(node->u.binop.left,  indent + 1);
            ast_dump(node->u.binop.right, indent + 1);
            break;

        case NODE_UNARY_MINUS:
            printf("UnaryMinus [%s]\n", datatype_str(node->dtype));
            ast_dump(node->u.unary.operand, indent + 1);
            break;

        case NODE_IF:
            printf("If\n");
            indent_print(indent + 1); printf("[cond]\n");
            ast_dump(node->u.if_stmt.cond,        indent + 2);
            indent_print(indent + 1); printf("[then]\n");
            ast_dump(node->u.if_stmt.then_branch,  indent + 2);
            break;

        case NODE_IF_ELSE:
            printf("IfElse\n");
            indent_print(indent + 1); printf("[cond]\n");
            ast_dump(node->u.if_stmt.cond,        indent + 2);
            indent_print(indent + 1); printf("[then]\n");
            ast_dump(node->u.if_stmt.then_branch,  indent + 2);
            indent_print(indent + 1); printf("[else]\n");
            ast_dump(node->u.if_stmt.else_branch,  indent + 2);
            break;

        case NODE_WHILE:
            printf("While\n");
            indent_print(indent + 1); printf("[cond]\n");
            ast_dump(node->u.while_stmt.cond,  indent + 2);
            indent_print(indent + 1); printf("[body]\n");
            ast_dump(node->u.while_stmt.body,  indent + 2);
            break;

        case NODE_PRINT:
            printf("Print\n");
            ast_dump(node->u.print_stmt.expr, indent + 1);
            break;

        case NODE_BLOCK:
            printf("Block\n");
            ast_dump(node->u.block.stmts, indent + 1);
            break;

        case NODE_STMT_LIST: {
            ASTNode *cur = node;
            while (cur && cur->kind == NODE_STMT_LIST) {
                ast_dump(cur->u.stmt_list.stmt, indent);
                cur = cur->u.stmt_list.next;
            }
            break;
        }
    }
}

void ast_free(ASTNode *node)
{
    if (!node) return;

    switch (node->kind) {
        case NODE_INT_LIT:
        case NODE_BOOL_LIT:
            break;

        case NODE_IDENT:
            free(node->u.ident.name);
            break;

        case NODE_DECL:
            free(node->u.ident.name);
            break;

        case NODE_DECL_ASSIGN:
            free(node->u.ident.name);
            ast_free(node->u.ident.init_expr);
            break;

        case NODE_ASSIGN:
            free(node->u.assign.name);
            ast_free(node->u.assign.expr);
            break;

        case NODE_BINOP:
            ast_free(node->u.binop.left);
            ast_free(node->u.binop.right);
            break;

        case NODE_UNARY_MINUS:
            ast_free(node->u.unary.operand);
            break;

        case NODE_IF:
        case NODE_IF_ELSE:
            ast_free(node->u.if_stmt.cond);
            ast_free(node->u.if_stmt.then_branch);
            ast_free(node->u.if_stmt.else_branch);
            break;

        case NODE_WHILE:
            ast_free(node->u.while_stmt.cond);
            ast_free(node->u.while_stmt.body);
            break;

        case NODE_PRINT:
            ast_free(node->u.print_stmt.expr);
            break;

        case NODE_BLOCK:
            ast_free(node->u.block.stmts);
            break;

        case NODE_STMT_LIST:
            ast_free(node->u.stmt_list.stmt);
            ast_free(node->u.stmt_list.next);
            break;
    }
    free(node);
}
