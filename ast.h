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

ASTNode* create_int_literal(int val, int line);
ASTNode* create_bool_literal(int val, int line);
ASTNode* create_identifier(char *name, int line);
ASTNode* create_binary_op(Operator op, ASTNode *left, ASTNode *right, int line);
ASTNode* create_assign(char *name, ASTNode *expr, int line);
ASTNode* create_var_decl(DataType type, char *name, int line);
ASTNode* create_if(ASTNode *cond, ASTNode *body, ASTNode *else_body, int line);
ASTNode* create_while(ASTNode *cond, ASTNode *body, int line);
ASTNode* create_print(ASTNode *expr, int line);
ASTNode* create_block(ASTNode *stmt_list, int line);
ASTNode* create_program(ASTNode *stmt_list, int line);

void append_stmt(ASTNode **list, ASTNode *stmt);

#endif
