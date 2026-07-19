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
