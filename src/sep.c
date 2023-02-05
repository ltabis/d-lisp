#include <stdio.h>
#include "sep.h"

sep_t init_polish_notation_parser()
{
  mpc_parser_t *number = mpc_new("number");
  mpc_parser_t *symbol = mpc_new("symbol");
  mpc_parser_t *expr = mpc_new("expr");
  mpc_parser_t *sexpr = mpc_new("sexpr");
  mpc_parser_t *program = mpc_new("dlisp");

  sep_t parser = {
      .number = number,
      .symbol = symbol,
      .expr = expr,
      .sexpr = sexpr,
      .program = program};

  mpca_lang(MPCA_LANG_DEFAULT,
            "                             \
number : /-?[0-9]+/ ;                     \
symbol : '+' | '-' | '*' | '/' | '%' ;    \
sexpr  : '(' <expr>* ')' ;                \
expr   : <number> | <symbol> | <sexpr> ;  \
dlisp  : /^/ <expr>* /$/ ;                \
",
            number,
            symbol,
            sexpr,
            expr,
            program);

  return parser;
}

// Evaluate an operator expression on two values.
lval_t *eval_op(char op, lval_t *lx, lval_t *ly)
{

  if (lx->error != NONE)
    return lx;
  if (ly->error != NONE)
    return ly;

  long x = lx->value;
  long y = ly->value;

  switch (op)
  {
  case '+':
    return lval_ok(x + y);
  case '-':
    return lval_ok(x - y);
  case '*':
    return lval_ok(x * y);
  case '/':
    return y ? lval_ok(x / y) : lval_err(DIV_BY_ZERO);
  case '%':
    return lval_ok(x % y);
  default:
    return lval_err(UNKNOWN_OP);
  }
}

lval_t *eval_expr(mpc_ast_t *expr)
{
  if (strstr(expr->tag, "nb"))
    // A number, immediately parse from the string.
    return lval_ok(atoi(expr->contents));
  else
  {
    // An operator expression, let's parse it.
    //
    // It's really funny because the following lines feels normal yet
    // so wrong coming back from Rust programming.
    char op = expr->children[1]->contents[0];      // First is '(', we can jump to the second child and get the operator.
    lval_t *result = eval_expr(expr->children[2]); // eval first expression.

    if (result->error == NONE)
    {
      for (unsigned int i = 3; expr->children[i]->contents[0] != ')'; ++i)
      {
        lval_t *next = eval_expr(expr->children[i]);

        if (next->error == NONE)
        {
          result = eval_op(op, result, next);
        }
        else
        {
          return next;
        }
      }
    }

    return result;
  }
}

lval_t *eval(mpc_ast_t *ast)
{
  char op = ast->children[1]->contents[0];      // Move after > / regex
  lval_t *result = eval_expr(ast->children[2]); // eval first expression after the operator.

  if (result->error == NONE)
  {
    for (unsigned int i = 3; i < ast->children_num - 1; ++i)
    {
      lval_t *next = eval_expr(ast->children[i]);

      if (next->error == NONE)
      {
        result = eval_op(op, result, next);
      }
      else
      {
        return next;
      }
    }
  }

  return result;
}

void parse_user_input(sep_t *parser, char *input)
{
  mpc_result_t r;

  if (mpc_parse("<stdin>", input, parser->program, &r))
  {
    lval_t *result = eval(r.output);

    if (result->error == NONE)
    {
      printf("%ld\n", result->value);
    }
    else
    {
      printf("An error occurred during evaluation: %s.\n", interpret_lval_error(result->error));
    }

    free(result);
    mpc_ast_delete(r.output);
  }
  else
  {
    mpc_err_print(r.error);
    mpc_err_delete(r.error);
  }
}

void cleanup_polish_notation_parser(sep_t *parser)
{
  mpc_cleanup(5, parser->number, parser->symbol, parser->expr, parser->sexpr, parser->program);
}
