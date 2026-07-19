#ifndef TAC_H
#define TAC_H

#include "ast.h"

/* ─── TAC Operation Codes ─── */
typedef enum {
    TAC_ASSIGN,       /* result = arg1                   */
    TAC_ADD,          /* result = arg1 + arg2            */
    TAC_SUB,          /* result = arg1 - arg2            */
    TAC_MUL,          /* result = arg1 * arg2            */
    TAC_DIV,          /* result = arg1 / arg2            */
    TAC_NEG,          /* result = -arg1                  */
    TAC_LT,           /* result = arg1 < arg2            */
    TAC_GT,           /* result = arg1 > arg2            */
    TAC_EQ,           /* result = arg1 == arg2           */
    TAC_NEQ,          /* result = arg1 != arg2           */
    TAC_LABEL,        /* label result:                   */
    TAC_GOTO,         /* goto result                     */
    TAC_IF_FALSE,     /* if_false arg1 goto result       */
    TAC_PRINT,        /* print arg1                      */
} TACOp;

/* ─── TAC Instruction (linked list) ─── */
typedef struct TACInstr {
    TACOp   op;
    char   *result;
    char   *arg1;
    char   *arg2;
    struct TACInstr *next;
} TACInstr;

/* ─── TAC List (head + tail for efficient append) ─── */
typedef struct {
    TACInstr *head;
    TACInstr *tail;
    int       temp_count;   /* counter for temporaries t1, t2, ... */
    int       label_count;  /* counter for labels L1, L2, ... */
} TACList;

/* ─── API ─── */

/* Create a new empty TAC list */
TACList *tac_create(void);

/* Generate TAC from AST. Returns the TAC list. */
TACList *tac_generate(ASTNode *root);

/* Print all TAC instructions to stdout */
void tac_print(TACList *list);

/* Write TAC to a file */
void tac_write_file(TACList *list, const char *filename);

/* Free all memory used by the TAC list */
void tac_free(TACList *list);

/* Get op name as string (for debugging) */
const char *tac_op_str(TACOp op);

#endif
