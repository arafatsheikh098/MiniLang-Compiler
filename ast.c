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
