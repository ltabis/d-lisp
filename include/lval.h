#ifndef LVAL_H_
#define LVAL_H_

typedef enum { VALUE, ERROR } lval_type_t;
typedef enum { NONE, DIV_BY_ZERO, UNKNOWN_OP } lval_error_t;

typedef struct {
  long value;
  int error;
} lval_t;

lval_t new_lval_ok(long) ;
lval_t new_lval_err(lval_error_t);
char *interpret_lval_error(lval_error_t);

#endif
