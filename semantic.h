#ifndef SEMANTIC_H
#define SEMANTIC_H

#include "ast.h"

/* Run full semantic analysis on the AST.
   Returns 0 if no errors, non-zero count of errors otherwise. */
int sem_analyse(ASTNode *root);

#endif
