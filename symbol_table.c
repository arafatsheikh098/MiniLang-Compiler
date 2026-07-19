#include "symbol_table.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static Scope *current_scope = NULL;

void sym_init(void)
{
    current_scope = calloc(1, sizeof(Scope));
    if (!current_scope) {
        fprintf(stderr, "sym_init: out of memory\n");
        exit(1);
    }
    current_scope->symbols = NULL;
    current_scope->parent  = NULL;
}

void sym_destroy(void)
{
    while (current_scope) {
        sym_pop_scope();
    }
}

void sym_push_scope(void)
{
    Scope *s = calloc(1, sizeof(Scope));
    if (!s) {
        fprintf(stderr, "sym_push_scope: out of memory\n");
        exit(1);
    }
    s->symbols = NULL;
    s->parent  = current_scope;
    current_scope = s;
}

void sym_pop_scope(void)
{
    if (!current_scope) return;

    Symbol *sym = current_scope->symbols;
    while (sym) {
        Symbol *tmp = sym;
        sym = sym->next;
        free(tmp->name);
        free(tmp);
    }

    Scope *old = current_scope;
    current_scope = current_scope->parent;
    free(old);
}
