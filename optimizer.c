#include "optimizer.h"
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ─── Helper: check if a string is a numeric literal ─── */
static int is_literal(const char *s) {
  if (!s)
    return 0;
  if (*s == '-')
    s++;
  if (*s == '\0')
    return 0;
  while (*s) {
    if (!isdigit((unsigned char)*s))
      return 0;
    s++;
  }
  return 1;
}

/* ─── Pass 1: Constant Folding ─── */
/* If both arg1 and arg2 are integer literals, compute the result at compile
 * time. */
static int constant_folding(TACList *list) {
  int changes = 0;
  for (TACInstr *i = list->head; i; i = i->next) {
    if (i->arg1 && i->arg2 && is_literal(i->arg1) && is_literal(i->arg2)) {
      int a = atoi(i->arg1);
      int b = atoi(i->arg2);
      int result;
      int can_fold = 1;

      switch (i->op) {
      case TAC_ADD:
        result = a + b;
        break;
      case TAC_SUB:
        result = a - b;
        break;
      case TAC_MUL:
        result = a * b;
        break;
      case TAC_DIV:
        if (b == 0) {
          can_fold = 0;
          break;
        }
        result = a / b;
        break;
      case TAC_LT:
        result = (a < b) ? 1 : 0;
        break;
      case TAC_GT:
        result = (a > b) ? 1 : 0;
        break;
      case TAC_EQ:
        result = (a == b) ? 1 : 0;
        break;
      case TAC_NEQ:
        result = (a != b) ? 1 : 0;
        break;
      default:
        can_fold = 0;
        break;
      }

      if (can_fold) {
        /* Convert to simple assignment: result = literal */
        char buf[32];
        snprintf(buf, sizeof(buf), "%d", result);
        free(i->arg1);
        free(i->arg2);
        i->arg1 = strdup(buf);
        i->arg2 = NULL;
        i->op = TAC_ASSIGN;
        changes++;
      }
    }

    /* Fold unary negation of literal */
    if (i->op == TAC_NEG && i->arg1 && is_literal(i->arg1)) {
      int val = -atoi(i->arg1);
      char buf[32];
      snprintf(buf, sizeof(buf), "%d", val);
      free(i->arg1);
      i->arg1 = strdup(buf);
      i->op = TAC_ASSIGN;
      changes++;
    }
  }
  return changes;
}

/* ─── Pass 2: Constant Propagation ─── */
/* If we see "x = <literal>" and then "... x ..." with no reassignment
   to x in between, replace x with the literal. Simple forward scan. */
static int constant_propagation(TACList *list) {
  int changes = 0;

  for (TACInstr *def = list->head; def; def = def->next) {
    /* Find assignments of literal to a variable */
    if (def->op != TAC_ASSIGN)
      continue;
    if (!def->result || !def->arg1)
      continue;
    if (!is_literal(def->arg1))
      continue;

    const char *var = def->result;
    const char *val = def->arg1;

    /* Propagate forward until var is reassigned */
    for (TACInstr *use = def->next; use; use = use->next) {
      /* Stop if var is redefined */
      if (use->result && strcmp(use->result, var) == 0 &&
          use->op != TAC_LABEL) {
        break;
      }
      /* Stop at labels (conservative: control flow merge) */
      if (use->op == TAC_LABEL)
        break;

      /* Replace uses in arg1 */
      if (use->arg1 && strcmp(use->arg1, var) == 0 && use->op != TAC_LABEL) {
        free(use->arg1);
        use->arg1 = strdup(val);
        changes++;
      }
      /* Replace uses in arg2 */
      if (use->arg2 && strcmp(use->arg2, var) == 0) {
        free(use->arg2);
        use->arg2 = strdup(val);
        changes++;
      }
    }
  }
  return changes;
}

/* ─── Pass 3: Dead Code Elimination ─── */
/* Remove assignments to temporaries (t1, t2, ...) that are never used
   after their definition. */
static int is_temp(const char *s) {
  if (!s)
    return 0;
  return (s[0] == 't' && isdigit((unsigned char)s[1]));
}

static int is_used_after(TACInstr *def, const char *var) {
  for (TACInstr *i = def->next; i; i = i->next) {
    if (i->arg1 && strcmp(i->arg1, var) == 0)
      return 1;
    if (i->arg2 && strcmp(i->arg2, var) == 0)
      return 1;
  }
  return 0;
}

static int dead_code_elimination(TACList *list) {
  int changes = 0;
  TACInstr *prev = NULL;

  for (TACInstr *i = list->head; i;) {
    int is_dead = 0;

    /* Only consider assignments to temporaries */
    if ((i->op == TAC_ASSIGN || i->op == TAC_ADD || i->op == TAC_SUB ||
         i->op == TAC_MUL || i->op == TAC_DIV || i->op == TAC_NEG ||
         i->op == TAC_LT || i->op == TAC_GT || i->op == TAC_EQ ||
         i->op == TAC_NEQ) &&
        i->result && is_temp(i->result)) {
      if (!is_used_after(i, i->result)) {
        is_dead = 1;
      }
    }

    if (is_dead) {
      /* Remove this instruction */
      TACInstr *to_remove = i;
      if (prev) {
        prev->next = i->next;
      } else {
        list->head = i->next;
      }
      if (i == list->tail) {
        list->tail = prev;
      }
      i = i->next;
      free(to_remove->result);
      free(to_remove->arg1);
      free(to_remove->arg2);
      free(to_remove);
      changes++;
    } else {
      prev = i;
      i = i->next;
    }
  }
  return changes;
}

/* ─── Main optimization driver ─── */
int optimize(TACList *list) {
  int total_changes = 0;
  int pass_changes;

  /* Iterate until no more changes (fixpoint) */
  do {
    pass_changes = 0;
    pass_changes += constant_folding(list);
    pass_changes += constant_propagation(list);
    pass_changes += dead_code_elimination(list);
    total_changes += pass_changes;
  } while (pass_changes > 0);

  return total_changes;
}