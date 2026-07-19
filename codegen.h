#ifndef CODEGEN_H
#define CODEGEN_H

#include "ast.h"
#include <stdio.h>

void init_codegen(FILE *out);
void generate_tac(ASTNode *root);

#endif
