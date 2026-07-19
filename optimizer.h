#ifndef OPTIMIZER_H
#define OPTIMIZER_H

#include "tac.h"

/* Apply all optimization passes to the TAC list (in-place).
   Returns the number of optimizations applied. */
int optimize(TACList *list);

#endif
