
#include "symbol_table.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static Scope *current_scope = NULL;

void sym_init(void) {
  current_scope = calloc(1, sizeof(Scope));
  if (!current_scope) {
    fprintf(stderr, "sym_init: out of memory\n");
    exit(1);
  }
  current_scope->symbols = NULL;
  current_scope->parent = NULL;
}

void sym_destroy(void) {
  while (current_scope) {
    sym_pop_scope();
  }
}

void sym_push_scope(void) {
  Scope *s = calloc(1, sizeof(Scope));
  if (!s) {
    fprintf(stderr, "sym_push_scope: out of memory\n");
    exit(1);
  }
  s->symbols = NULL;
  s->parent = current_scope;
  current_scope = s;
}

void sym_pop_scope(void) {
  if (!current_scope)
    return;

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

int sym_declare(const char *name, DataType type, int lineno) {
  /* Check for duplicate in same scope */
  if (sym_lookup_local(name)) {
    return -1; /* duplicate */
  }

  Symbol *sym = calloc(1, sizeof(Symbol));
  if (!sym) {
    fprintf(stderr, "sym_declare: out of memory\n");
    exit(1);
  }
  sym->name = strdup(name);
  sym->type = type;
  sym->lineno = lineno;
  sym->next = current_scope->symbols;
  current_scope->symbols = sym;
  return 0;
}

Symbol *sym_lookup(const char *name) {
  Scope *scope = current_scope;
  while (scope) {
    Symbol *sym = scope->symbols;
    while (sym) {
      if (strcmp(sym->name, name) == 0)
        return sym;
      sym = sym->next;
    }
    scope = scope->parent;
  }
  return NULL;
}

Symbol *sym_lookup_local(const char *name) {
  if (!current_scope)
    return NULL;
  Symbol *sym = current_scope->symbols;
  while (sym) {
    if (strcmp(sym->name, name) == 0)
      return sym;
    sym = sym->next;
  }
  return NULL;
}
