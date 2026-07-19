#include "symbol_table.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static Scope *root_scope = NULL;
static Scope *current_scope = NULL;
static int scope_counter = 1;

void sym_init(void)
{
    root_scope = calloc(1, sizeof(Scope));
    if (!root_scope) {
        fprintf(stderr, "sym_init: out of memory\n");
        exit(1);
    }
    root_scope->scope_name = strdup("Global");
    root_scope->symbols = NULL;
    root_scope->parent  = NULL;
    root_scope->capacity_children = 4;
    root_scope->num_children = 0;
    root_scope->children = calloc(root_scope->capacity_children, sizeof(Scope*));
    current_scope = root_scope;
}

static void free_scope(Scope *s) {
    if (!s) return;
    Symbol *sym = s->symbols;
    while (sym) {
        Symbol *tmp = sym;
        sym = sym->next;
        free(tmp->name);
        free(tmp);
    }
    for (int i = 0; i < s->num_children; i++) {
        free_scope(s->children[i]);
    }
    free(s->children);
    free(s->scope_name);
    free(s);
}

void sym_destroy(void)
{
    free_scope(root_scope);
    root_scope = NULL;
    current_scope = NULL;
}

void sym_push_scope(void)
{
    Scope *s = calloc(1, sizeof(Scope));
    if (!s) {
        fprintf(stderr, "sym_push_scope: out of memory\n");
        exit(1);
    }
    char buf[32];
    sprintf(buf, "Block_%d", scope_counter++);
    s->scope_name = strdup(buf);
    s->symbols = NULL;
    s->parent  = current_scope;
    s->capacity_children = 4;
    s->num_children = 0;
    s->children = calloc(s->capacity_children, sizeof(Scope*));
    
    // Add to parent's children
    if (current_scope->num_children == current_scope->capacity_children) {
        current_scope->capacity_children *= 2;
        current_scope->children = realloc(current_scope->children, current_scope->capacity_children * sizeof(Scope*));
    }
    current_scope->children[current_scope->num_children++] = s;
    
    current_scope = s;
}

void sym_pop_scope(void)
{
    if (!current_scope || !current_scope->parent) return;
    current_scope = current_scope->parent;
}

int sym_declare(const char *name, DataType type, int lineno)
{
    /* Check for duplicate in same scope */
    if (sym_lookup_local(name)) {
        return -1;  /* duplicate */
    }

    Symbol *sym = calloc(1, sizeof(Symbol));
    if (!sym) {
        fprintf(stderr, "sym_declare: out of memory\n");
        exit(1);
    }
    sym->name   = strdup(name);
    sym->type   = type;
    sym->lineno = lineno;
    
    // Insert at end so order matches declaration order in output (or just push front, but push front prints in reverse)
    // Actually, pushing front is easier for C linked lists. The python dictionary prints in insertion order (python 3.7+).
    // Let's just push front for simplicity, since that's what it did before.
    sym->next   = current_scope->symbols;
    current_scope->symbols = sym;
    return 0;
}

Symbol *sym_lookup(const char *name)
{
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

Symbol *sym_lookup_local(const char *name)
{
    if (!current_scope) return NULL;
    Symbol *sym = current_scope->symbols;
    while (sym) {
        if (strcmp(sym->name, name) == 0)
            return sym;
        sym = sym->next;
    }
    return NULL;
}

static void print_scope_recursive(Scope *s, int depth) {
    if (!s) return;
    char indent[64] = {0};
    for(int i = 0; i < depth * 2 && i < 63; i++) {
        indent[i] = ' ';
    }
    
    Symbol *sym = s->symbols;
    // To print in original declaration order, we'd reverse the list, 
    // but printing in reverse declaration order is fine.
    while(sym) {
        char scope_display[64];
        if (depth == 0) {
            snprintf(scope_display, sizeof(scope_display), "%s", s->scope_name);
        } else {
            snprintf(scope_display, sizeof(scope_display), "Local (%s)", s->scope_name);
        }
        
        // Use an offset trick to print name with indent properly padded
        int name_padding = 15 - (depth * 2);
        if (name_padding < 0) name_padding = 0;
        
        printf("%s%-*s | %-10s | %-20s\n", 
            indent, name_padding, sym->name, datatype_str(sym->type), scope_display);
        sym = sym->next;
    }
    
    for(int i = 0; i < s->num_children; i++) {
        print_scope_recursive(s->children[i], depth + 1);
    }
}

void sym_dump(void) {
    printf("\n  Symbol Table Tree:\n\n");
    printf("%-15s | %-10s | %-20s\n", "Name", "Type", "Scope");
    printf("--------------------------------------------------\n");
    print_scope_recursive(root_scope, 0);
    printf("--------------------------------------------------\n\n");
}
