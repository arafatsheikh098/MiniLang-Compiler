#include <stdlib.h>
#include <string.h>
#include "ast.h"

ASTNode* create_node(NodeType type, int line) {
    ASTNode *node = (ASTNode*)malloc(sizeof(ASTNode));
    memset(node, 0, sizeof(ASTNode));
    node->type = type;
    node->line_num = line;
    node->dataType = TYPE_VOID;
    return node;
}

ASTNode* create_int_literal(int val, int line) {
    ASTNode *n = create_node(NODE_INT_LITERAL, line);
    n->int_val = val;
    n->dataType = TYPE_INT;
    return n;
}

ASTNode* create_bool_literal(int val, int line) {
    ASTNode *n = create_node(NODE_BOOL_LITERAL, line);
    n->bool_val = val;
    n->dataType = TYPE_BOOL;
    return n;
}

ASTNode* create_identifier(char *name, int line) {
    ASTNode *n = create_node(NODE_IDENTIFIER, line);
    n->name = strdup(name);
    return n;
}

ASTNode* create_binary_op(Operator op, ASTNode *left, ASTNode *right, int line) {
    ASTNode *n = create_node(NODE_BINARY_OP, line);
    n->op = op;
    n->left = left;
    n->right = right;
    return n;
}

ASTNode* create_assign(char *name, ASTNode *expr, int line) {
    ASTNode *n = create_node(NODE_ASSIGN, line);
    n->name = strdup(name);
    n->right = expr;
    return n;
}

ASTNode* create_var_decl(DataType type, char *name, int line) {
    ASTNode *n = create_node(NODE_VAR_DECL, line);
    n->dataType = type;
    n->name = strdup(name);
    return n;
}

ASTNode* create_if(ASTNode *cond, ASTNode *body, ASTNode *else_body, int line) {
    ASTNode *n = create_node(NODE_IF, line);
    n->cond = cond;
    n->body = body;
    n->else_body = else_body;
    return n;
}

ASTNode* create_while(ASTNode *cond, ASTNode *body, int line) {
    ASTNode *n = create_node(NODE_WHILE, line);
    n->cond = cond;
    n->body = body;
    return n;
}

ASTNode* create_print(ASTNode *expr, int line) {
    ASTNode *n = create_node(NODE_PRINT, line);
    n->left = expr;
    return n;
}

ASTNode* create_block(ASTNode *stmt_list, int line) {
    ASTNode *n = create_node(NODE_BLOCK, line);
    n->body = stmt_list;
    return n;
}

ASTNode* create_program(ASTNode *stmt_list, int line) {
    ASTNode *n = create_node(NODE_PROGRAM, line);
    n->body = stmt_list;
    return n;
}

void append_stmt(ASTNode **list, ASTNode *stmt) {
    if (*list == NULL) {
        *list = stmt;
    } else {
        ASTNode *cur = *list;
        while (cur->next != NULL) {
            cur = cur->next;
        }
        cur->next = stmt;
    }
}
