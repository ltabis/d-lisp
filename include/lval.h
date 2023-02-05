#ifndef LVAL_H_
#define LVAL_H_

#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "mpc.h"

typedef enum
{
  NUMBER,
  SYMBOL,
  SEXPR,
  ERROR
} lval_type_t;

typedef enum
{
  NONE,
  DIV_BY_ZERO,
  NUM_CONVERSION,
  UNKNOWN_OP
} lval_error_t;

typedef struct lval_s
{
  lval_type_t type;
  long number;
  lval_error_t error;
  char *symbol;

  size_t count;
  struct lval_s **cell;
} lval_t;

lval_t *lval_num(long);
lval_t *lval_sym(const char *);
lval_t *lval_sexpr();
lval_t *lval_err(lval_error_t);

lval_t *lval_read_num(const mpc_ast_t *);
lval_t *lval_read(const mpc_ast_t *);
lval_t *lval_add(lval_t *, lval_t *);

void lval_println(lval_t *);

void lval_del(lval_t *);
char *lval_interpret_error(lval_error_t);

#endif
