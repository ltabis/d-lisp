#ifndef LVAL_H_
#define LVAL_H_

#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdbool.h>
#include "mpc.h"

#define COMPOUND_CHAR_COUNT 4

#define LASSERT(args, cond, err) \
  if (!(cond)) { lval_del(args); return lval_err(err); }

typedef enum
{
  NUMBER,
  SYMBOL,
  SEXPR,
  QEXPR,
  ERROR
} lval_type_t;

typedef struct lval_s
{
  lval_type_t type;
  long number;
  char *error;
  char *symbol;

  size_t count;
  struct lval_s **cell;
} lval_t;

lval_t *lval_num(long);
lval_t *lval_sym(const char *);
lval_t *lval_sexpr();
lval_t *lval_err(const char *);

lval_t *lval_read_num(const mpc_ast_t *);
lval_t *lval_read(const mpc_ast_t *);
lval_t *lval_add(lval_t *, lval_t *);
lval_t *lval_pop(lval_t *, unsigned int);
lval_t *lval_take(lval_t *, unsigned int);

lval_t *lval_eval(lval_t *);
lval_t *lval_eval_expr(lval_t *);
lval_t *builtin_op(lval_t *, char *);
lval_t *builtin_head(lval_t *);
lval_t *builtin_tail(lval_t *);
lval_t *builtin_list(lval_t *);
lval_t *builtin_eval(lval_t *);

void lval_println(lval_t *);

void lval_del(lval_t *);

#endif
