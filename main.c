#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ast.h"
#include "semantic.h"
#include "symbol_table.h"
#include "tac.h"
#include "optimizer.h"
#include "vm.h"
#include "codegen.h"

extern int   yyparse(void);
extern FILE *yyin;
extern int   yylineno;

extern int g_dump_tokens;

extern ASTNode *parse_root;

static void print_banner(void)
{
    printf("╔══════════════════════════════════════════════════════════════╗\n");
    printf("║          MiniLang Compiler  (Full 6-Phase Pipeline)          ║\n");
    printf("║    Dept. of CSE, University of Chittagong                    ║\n");
    printf("║    CSE 712 — Compiler Design Lab                             ║\n");
    printf("║    Phases: Lexer → Parser → Semantic → TAC → Opt → CodeGen  ║\n");
    printf("╚══════════════════════════════════════════════════════════════╝\n\n");
}

int main(int argc, char *argv[])
{
    print_banner();

    char *source_file = NULL;

    for (int i = 1; i < argc; i++) {
        if (argv[i][0] != '-') {
            source_file = argv[i];
        } else {
            fprintf(stderr, "Unknown option: %s\n", argv[i]);
            fprintf(stderr, "Usage: %s <source.ml>\n", argv[0]);
            return 1;
        }
    }

    if (!source_file) {
        fprintf(stderr, "Usage: %s <source.ml>\n", argv[0]);
        return 1;
    }

    yyin = fopen(source_file, "r");
    if (!yyin) {
        perror(source_file);
        return 1;
    }

    printf("Source file : %s\n\n", source_file);

    /* ═══════════════════════════════════════════════════════════════════
       PHASE 1 : LEXICAL ANALYSIS (Flex)
       ═══════════════════════════════════════════════════════════════════ */
    printf("╔══════════════════════════════════════════════════════════════╗\n");
    printf("║              PHASE 1 : LEXICAL ANALYSIS                     ║\n");
    printf("╚══════════════════════════════════════════════════════════════╝\n\n");
    printf("  %-4s  %-6s  %-20s  %s\n", "No.", "Line", "Token Type", "Lexeme");
    printf("  %-4s  %-6s  %-20s  %s\n",
           "----", "------", "--------------------", "----------");

    g_dump_tokens = 1;

    int parse_result = yyparse();
    fclose(yyin);

    printf("\n");

    if (parse_result != 0 || !parse_root) {
        fprintf(stderr, "\n  ✗  Parsing failed — aborting.\n");
        return 1;
    }

    printf("  ✓  Lexical analysis complete — %s tokenized.\n\n", source_file);

    /* ═══════════════════════════════════════════════════════════════════
       PHASE 2 : SYNTAX ANALYSIS — AST Construction (Bison)
       ═══════════════════════════════════════════════════════════════════ */
    printf("╔══════════════════════════════════════════════════════════════╗\n");
    printf("║              PHASE 2 : SYNTAX ANALYSIS (AST)                ║\n");
    printf("╚══════════════════════════════════════════════════════════════╝\n\n");
    printf("  Abstract Syntax Tree:\n");
    printf("  (indentation = nesting depth; 2 spaces per level)\n\n");

    ast_dump(parse_root, 1);

    printf("\n  ✓  Syntax analysis complete — AST constructed.\n\n");

    /* ═══════════════════════════════════════════════════════════════════
       PHASE 3 : SEMANTIC ANALYSIS (Symbol Table + Type Checking)
       ═══════════════════════════════════════════════════════════════════ */
    printf("╔══════════════════════════════════════════════════════════════╗\n");
    printf("║              PHASE 3 : SEMANTIC ANALYSIS                    ║\n");
    printf("╚══════════════════════════════════════════════════════════════╝\n\n");

    int sem_errors = sem_analyse(parse_root);

    if (sem_errors > 0) {
        fprintf(stderr, "\n  ✗  Semantic analysis failed with %d error(s).\n",
                sem_errors);
        ast_free(parse_root);
        return 1;
    }

    printf("  Checks performed:\n");
    printf("    • Undeclared variable detection\n");
    printf("    • Multiple declarations in same scope\n");
    printf("    • Type mismatch in expressions and assignments\n");
    printf("    • Invalid conditional expressions (if/while require bool)\n");
    printf("    • Arithmetic operand validation (must be int)\n");
    printf("    • Division by zero detection\n\n");

    printf("  ✓  Semantic analysis complete — no errors found.\n\n");
    
    sym_dump();

    /* ═══════════════════════════════════════════════════════════════════
       PHASE 4 : INTERMEDIATE CODE GENERATION (TAC)
       ═══════════════════════════════════════════════════════════════════ */
    printf("╔══════════════════════════════════════════════════════════════╗\n");
    printf("║        PHASE 4 : INTERMEDIATE CODE GENERATION (TAC)         ║\n");
    printf("╚══════════════════════════════════════════════════════════════╝\n\n");

    TACList *tac = codegen_tac(parse_root, "output.tac");

    printf("  Three-Address Code (written to output.tac):\n\n");
    tac_print(tac);
    printf("\n");
    printf("  ✓  TAC generation complete → output.tac\n\n");

    /* ═══════════════════════════════════════════════════════════════════
       PHASE 5 : CODE OPTIMIZATION
       ═══════════════════════════════════════════════════════════════════ */
    printf("╔══════════════════════════════════════════════════════════════╗\n");
    printf("║              PHASE 5 : CODE OPTIMIZATION                    ║\n");
    printf("╚══════════════════════════════════════════════════════════════╝\n\n");

    printf("  Optimizations applied:\n");
    printf("    • Constant folding\n");
    printf("    • Constant propagation\n");
    printf("    • Dead code elimination\n");
    printf("    • Redundant temporary removal\n\n");

    int opt_count = optimize(tac);

    printf("  Optimized TAC (%d optimization(s) applied):\n\n", opt_count);
    tac_print(tac);
    printf("\n");

    tac_write_file(tac, "output_optimized.tac");
    printf("  ✓  Optimized TAC → output_optimized.tac\n\n");

    /* ═══════════════════════════════════════════════════════════════════
       PHASE 6 : TARGET CODE GENERATION (Stack-Machine)
       ═══════════════════════════════════════════════════════════════════ */
    printf("╔══════════════════════════════════════════════════════════════╗\n");
    printf("║        PHASE 6 : TARGET CODE GENERATION (Stack Machine)     ║\n");
    printf("╚══════════════════════════════════════════════════════════════╝\n\n");

    VMProgram *vm_prog = codegen_target(tac);

    printf("  Stack-Machine Instructions:\n\n");
    vm_print(vm_prog);
    printf("\n");
    printf("  ✓  Target code generated (%d instructions).\n\n", vm_prog->count);

    /* ═══════════════════════════════════════════════════════════════════
       EXECUTION : Run on Stack Machine
       ═══════════════════════════════════════════════════════════════════ */
    printf("╔══════════════════════════════════════════════════════════════╗\n");
    printf("║              PROGRAM EXECUTION (VM Output)                   ║\n");
    printf("╚══════════════════════════════════════════════════════════════╝\n\n");
    printf("  Output:\n  ");

    vm_execute(vm_prog);

    printf("\n");
    printf("  ✓  Execution complete.\n\n");

    /* ═══════════════════════════════════════════════════════════════════
       DONE
       ═══════════════════════════════════════════════════════════════════ */
    printf("╔══════════════════════════════════════════════════════════════╗\n");
    printf("║   All 6 compilation phases completed successfully.          ║\n");
    printf("╚══════════════════════════════════════════════════════════════╝\n");

    /* Cleanup */
    vm_free(vm_prog);
    tac_free(tac);
    ast_free(parse_root);
    sym_destroy();

    return 0;
}
