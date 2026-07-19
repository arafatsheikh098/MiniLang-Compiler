#ifndef CODEGEN_H
#define CODEGEN_H

#include "ast.h"
#include "tac.h"
#include "vm.h"

/* ─── Phase 4: Intermediate Code Generation ─── */

/* Generate TAC from a semantically-valid AST.
   Also writes the TAC to the given output file.
   Returns the generated TAC list (caller must free). */
TACList *codegen_tac(ASTNode *root, const char *output_file);

/* ─── Phase 6: Target Code Generation ─── */

/* Generate stack-machine target code from (optimized) TAC.
   Returns the VM program (caller must free). */
VMProgram *codegen_target(TACList *tac);

#endif
