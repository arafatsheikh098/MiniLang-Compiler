#ifndef AST_H
#define AST_H

typedef enum {
    TYPE_INT,
    TYPE_BOOL,
    TYPE_VOID
} DataType;

typedef enum {
    NODE_INT_LITERAL,
    NODE_BOOL_LITERAL,
    NODE_IDENTIFIER,
    NODE_BINARY_OP,
    NODE_ASSIGN,
    NODE_VAR_DECL,
    NODE_IF,
    NODE_WHILE,
    NODE_PRINT,
    NODE_BLOCK,
    NODE_PROGRAM
} NodeType;

typedef enum {
    OP_ADD,
    OP_SUB,
    OP_MUL,
    OP_DIV,
    OP_EQ,
    OP_NEQ,
    OP_LT,
    OP_GT,
    OP_LE,
    OP_GE,
    OP_AND,
    OP_OR
} Operator;

typedef struct ASTNode {
    NodeType type;
    DataType dataType;
    int line_num;
    
    // For Int Literal
    int int_val;
    // For Bool Literal
    int bool_val;
    // For Identifier or Var Decl
    char *name;
    
    // For Binary Operations
    Operator op;
    
    struct ASTNode *left;
    struct ASTNode *right;
    struct ASTNode *cond;
    struct ASTNode *body;
    struct ASTNode *else_body;
    
    struct ASTNode *next; // For linked lists of statements
} ASTNode;
