#ifndef SYMBOL_TABLE_H
#define SYMBOL_TABLE_H

#include "ast.h"

/* ─── Symbol Entry ─── */
typedef struct Symbol {
    char     *name;
    DataType  type;
    int       lineno;       /* line where declared */
    struct Symbol *next;    /* linked list for chaining in same scope */
} Symbol;

/* Create the global scope. Must be called once before analysis. */
void sym_init(void);

/* Destroy all scopes and free memory. Call at the end. */
void sym_destroy(void);

#endif
