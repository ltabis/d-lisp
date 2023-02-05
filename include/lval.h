#ifndef LVAL_H_
#define LVAL_H_

#include <stdlib.h>

typedef enum
{
  VALUE,
  ERROR
} lval_type_t;

typedef enum
{
  NONE,
  DIV_BY_ZERO,
  UNKNOWN_OP
} lval_error_t;

typedef struct lval_s
{
  lval_type_t type;
  long value;
  lval_error_t error;
  char *symbol;

  size_t count;
  struct lval_s **cell;
} lval_t;

lval_t *lval_num(long);
lval_t *lval_err(lval_error_t);
lval_t *lval_sym(char *);
char *interpret_lval_error(lval_error_t);

#endif
