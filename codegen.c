#include "codegen.h"
#include <stdio.h>

/* ═══════════════════════════════════════════════════════════════
   Phase 4: Intermediate Code Generation (AST → TAC)
   ═══════════════════════════════════════════════════════════════ */

TACList *codegen_tac(ASTNode *root, const char *output_file)
{
    /* Generate Three-Address Code from AST */
    TACList *tac = tac_generate(root);

    /* Write to file */
    if (output_file) {
        tac_write_file(tac, output_file);
    }

    return tac;
}

/* ═══════════════════════════════════════════════════════════════
   Phase 6: Target Code Generation (TAC → Stack-Machine Code)
   ═══════════════════════════════════════════════════════════════ */

VMProgram *codegen_target(TACList *tac)
{
    return vm_generate(tac);
}
