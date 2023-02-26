#ifndef LVAL_H_
#define LVAL_H_

#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdbool.h>
#include "mpc.h"

#define COMPOUND_CHAR_COUNT 4

#define LASSERT(args, cond, fmt, ...) \
  if (!(cond)) { \
    lval_t *err = lval_err(fmt, ##__VA_ARGS__); \
    lval_del(args); \
    return err; \
  }

#define LASSERT_NUM_PARAMS(function, args, expected) \
  LASSERT( \
    args, \
    args->count == expected, \
    "function '%s' expected %i parameters, got %i", function, expected, args->count \
  )

#define LASSERT_TYPE(function, arg, expected) \
  LASSERT( \
    arg, \
    arg->type == expected, \
    "function '%s' expected value to be of type '%s', not '%s'", \
      function,\
      lval_type_name(expected), \
      lval_type_name(arg->type) \
  )

#define LASSERT_CHILDREN_TYPE(function, args, children, expected) \
  LASSERT( \
    args, \
    args->cell[children]->type == expected, \
    "function '%s' expected children at index %i to be of type '%s', not '%s'", \
      function,\
      children,\
      lval_type_name(expected), \
      lval_type_name(args->cell[children]->type) \
  )

typedef struct lval_s lval_t;
typedef struct lenv_s lenv_t;

typedef lval_t *(*lbuiltin)(lenv_t *, lval_t *);

typedef enum
{
  NUMBER,
  STRING,
  SYMBOL,
  FUN,
  SEXPR,
  QEXPR,
  ERROR,
} lval_type_t;

// Used to store any d-lisp value;
typedef struct lval_s
{
  lval_type_t type;

  long number;
  char *string;
  char *symbol;
  char *error;

  lbuiltin builtin;
  lenv_t *env;
  lval_t *formals;
  lval_t *body;

  size_t count;
  lval_t **cell;
} lval_t;

// Used to keep track of the variables names and their associated lval.
struct lenv_s {
  lenv_t *parent;
  size_t count;
  char **syms;
  lval_t **vals;
};

lenv_t *lenv_new();
lval_t *lval_num(long);
lval_t *lval_string(const char *);
lval_t *lval_sym(const char *);
lval_t *lval_sexpr();
lval_t *lval_qexpr();
lval_t *lval_fun(lbuiltin);
lval_t *lval_lambda(lval_t *, lval_t *);
lval_t *lval_err(const char *, ...);

lval_t *lenv_get(lenv_t *, const char *);
void lenv_push(lenv_t *, lval_t *, lval_t *);
void lenv_def(lenv_t *, lval_t *, lval_t *);
void lenv_add_builtin(lenv_t *, const char *, lbuiltin);
void lenv_add_builtins(lenv_t *);

lval_t *lval_read_num(const mpc_ast_t *);
lval_t *lval_read(const mpc_ast_t *);
lval_t *lval_add(lval_t *, lval_t *);
lval_t *lval_pop(lval_t *, unsigned int);
lval_t *lval_take(lval_t *, unsigned int);

lval_t *lval_eval(lenv_t *, lval_t *);
lval_t *lval_eval_sexpr(lenv_t *, lval_t *);
lval_t *lval_call(lenv_t *, lval_t *, lval_t *);
int lval_eq(lval_t *, lval_t *);
int lval_cmp(lval_t *, lval_t *);
lval_t *builtin_op(lenv_t *, lval_t *, char *);
lval_t *builtin_op_add(lenv_t *, lval_t *);
lval_t *builtin_op_sub(lenv_t *, lval_t *);
lval_t *builtin_op_div(lenv_t *, lval_t *);
lval_t *builtin_op_mul(lenv_t *, lval_t *);
lval_t *builtin_op_mod(lenv_t *, lval_t *);
lval_t *builtin_op_greater(lenv_t *, lval_t *);
lval_t *builtin_op_greater_equal(lenv_t *, lval_t *);
lval_t *builtin_op_lesser(lenv_t *, lval_t *);
lval_t *builtin_op_lesser_equal(lenv_t *, lval_t *);
lval_t *builtin_cmp(lenv_t *, lval_t *, const char *);
lval_t *builtin_cmp_eq(lenv_t *, lval_t *);
lval_t *builtin_cmp_neq(lenv_t *, lval_t *);
lval_t *builtin_head(lenv_t *, lval_t *);
lval_t *builtin_tail(lenv_t *, lval_t *);
lval_t *builtin_list(lenv_t *, lval_t *);
lval_t *builtin_eval(lenv_t *, lval_t *);
lval_t *builtin_join(lenv_t *, lval_t *);
lval_t *builtin_var(lenv_t *, lval_t *, const char *);
lval_t *builtin_def(lenv_t *, lval_t *);
lval_t *builtin_push(lenv_t *, lval_t *);
lval_t *builtin_lambda(lenv_t *, lval_t *);
lval_t *builtin_fn(lenv_t *, lval_t *);
lval_t *builtin_if(lenv_t *, lval_t *);

void lval_println(lval_t *);
void lval_print_string(const lval_t *);
char *lval_type_name(unsigned int t);

lval_t *lval_clone(lval_t *);
lenv_t *lenv_clone(lenv_t *);
void lval_del(lval_t *);
void lenv_del(lenv_t *);

#endif

