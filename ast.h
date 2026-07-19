#ifndef AST_H
#define AST_H
#include <stdlib.h>

typedef enum {
  TYPE_INT,
  TYPE_BOOL,
  TYPE_UNKNOWN
} DataType;

typedef enum {
  NODE_INT_LIT,
  NODE_BOOL_LIT,
  NODE_IDENT,
  NODE_DECL,
  NODE_DECL_ASSIGN,
  NODE_ASSIGN,
  NODE_BINOP,
  NODE_UNARY_MINUS,
  NODE_IF,
  NODE_IF_ELSE,
  NODE_WHILE,
  NODE_PRINT,
  NODE_BLOCK,
  NODE_STMT_LIST
} NodeKind;

typedef enum {
  OP_ADD,
  OP_SUB,
  OP_MUL,
  OP_DIV,
  OP_LT,
  OP_GT,
  OP_EQ,
  OP_NEQ
} BinOp;

typedef struct ASTNode ASTNode;

struct ASTNode {
  NodeKind kind;
  DataType dtype;
  int lineno;

  union {
    int ival;

    int bval;

    struct {
      char *name;
      DataType decl_type;
      ASTNode *init_expr;
    } ident;

    struct {
      char *name;
      ASTNode *expr;
    } assign;

    struct {
      BinOp op;
      ASTNode *left;
      ASTNode *right;
    } binop;

    struct {
      ASTNode *operand;
    } unary;

    struct {
      ASTNode *cond;
      ASTNode *then_branch;
      ASTNode *else_branch;
    } if_stmt;

    struct {
      ASTNode *cond;
      ASTNode *body;
    } while_stmt;

    struct {
      ASTNode *expr;
    } print_stmt;

    struct {
      ASTNode *stmts;
    } block;

    struct {
      ASTNode *stmt;
      ASTNode *next;
    } stmt_list;

  } u;
};

#endif
