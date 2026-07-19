# MiniLang Compiler

> **Course:** CSE 712 — Compiler Design Lab  
> **University of Chittagong, Dept. of Computer Science & Engineering**  
> **Scope:** Complete 6-Phase Compiler Pipeline  

This project implements a **complete 6-phase compiler** for **MiniLang**, a simple statically-typed, block-scoped imperative language.

## Project Overview

The compiler features 6 distinct phases:
1. **Lexical Analysis** (Flex)
2. **Syntax Analysis / AST** (Bison)
3. **Semantic Analysis** (Type checking & Scoping)
4. **Intermediate Code Generation** (TAC)
5. **Code Optimization** (Constant Folding/Propagation, Dead Code Elimination)
6. **Code Generation & Execution** (Stack-Based VM)

---

## Prerequisites

To build and run the compiler, you need:

```bash
flex --version     # >= 2.6
bison --version    # >= 3.0
gcc --version      # >= 9
make --version
```

> On macOS:
> ```bash
> brew install flex bison
> xcode-select --install
> ```

---
### Project Structure

```
MiniCompiler/
├── lexer.l              ← Arafat
├── parser.y             ← Adnan
├── ast.c / ast.h        ← (shared)
├── symbol_table.c/.h    ← Arafat
├── semantic.c/.h        ← Joyesh
├── tac.c/.h             ← Mahdi
├── codegen.c/.h         ← Mahdi
├── optimizer.c/.h       ← Mahdi
├── vm.c/.h              ← Mahdi
├── main.c               ← (shared)
├── Makefile
└── testcases/
    ├── test1.ml ... test6.ml
    └── error_*.ml
```
---

## Build

To compile the compiler, simply run:

```bash
make
```

This will produce the `./minicompiler` executable.

Equivalent manual commands (if you don't use `make`):
```bash
flex lexer.l
bison -d parser.y
gcc lex.yy.c parser.tab.c *.c -o minicompiler
```

---

## Run

To compile and run a MiniLang program:

```bash
./minicompiler testcases/test1.ml
```

Shortcut to build and run `test1.ml`:
```bash
make run
```

---

## Output

The compiler prints the results of all 6 phases:

1. **Phase 1: Lexical Analysis** — Token table
2. **Phase 2: Syntax Analysis** — Abstract Syntax Tree
3. **Phase 3: Semantic Analysis** — Type checking results
4. **Phase 4: Intermediate Code (TAC)** — Three-Address Code → `output.tac`
5. **Phase 5: Code Optimization** — Optimized TAC → `output_optimized.tac`
6. **Phase 6: Target Code Generation** — Stack-machine bytecode
7. **Execution** — Program output from VM

---

## Test Cases

Several test cases are provided in the `testcases/` directory:

```bash
./minicompiler testcases/test1.ml    # Sum 1..10 (while loop)
./minicompiler testcases/test2.ml    # If-else branching
./minicompiler testcases/test3.ml    # Arithmetic + block scoping
./minicompiler testcases/test4.ml    # Relational operators
./minicompiler testcases/test5.ml    # Factorial computation
./minicompiler testcases/test6.ml    # All features combined
```

**Error handling tests:**
```bash
./minicompiler testcases/error_lexical.ml    # Lexical error
./minicompiler testcases/error_syntax.ml     # Syntax error
./minicompiler testcases/error_semantic.ml   # Semantic errors
```

---

## MiniLang Quick Reference

```c
int x = 42;              // integer declaration
bool flag = true;        // boolean declaration
x = x + 1;               // assignment
if (x > 0) { ... }       // conditional
if (x > 0) { } else { }  // if-else
while (x < 10) { ... }   // loop
print x;                 // output

// comment               // single-line
/* comment */            // block comment
```

---

## Clean

To remove all compiled and generated files:

```bash
make clean
```
