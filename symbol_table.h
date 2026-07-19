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

/* ─── Scope (one scope per block) ─── */
typedef struct Scope {
    Symbol       *symbols;  /* head of symbol list in this scope */
    struct Scope *parent;   /* enclosing scope (NULL for global) */
} Scope;

/* ─── API ─── */

/* Create the global scope. Must be called once before analysis. */
void sym_init(void);

/* Destroy all scopes and free memory. Call at the end. */
void sym_destroy(void);

/* Push a new scope (entering a block). */
void sym_push_scope(void);

/* Pop the current scope (leaving a block). */
void sym_pop_scope(void);

#endif
