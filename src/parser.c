#include <stdio.h>
#include "pnp.h"

// enable flycheck https://www.reddit.com/r/emacs/comments/audffp/tip_how_to_use_a_stable_and_fast_environment_to/

pnp_t init_polish_notation_parser() {

  mpc_parser_t *number = mpc_new("nb");
  mpc_parser_t *operator = mpc_new("op");
  mpc_parser_t *expression = mpc_new("expr");
  mpc_parser_t *program = mpc_new("dlisp");

  pnp_t parser = {
  .number = number,
  .operator = operator,
  .expression = expression,
  .program = program
};

  mpca_lang(MPCA_LANG_DEFAULT,
            "\
nb : /-?[0-9]+/ ;                  \
op : '+' | '-' | '*' | '/' | '%' ; \
expr : <nb> | '(' <op> <expr>+ ')' ; \
dlisp : /^/ <op> <expr>+ /$/ ;     \
",
            number,
            operator,
            expression,
            program
);

    return parser;
}

long eval_op(char op, long x, long y) {
  switch (op) {
    case '+': return x + y;
    case '-': return x - y;
    case '*': return x * y;
    case '/': return y ? x / y : 0;
    case '%': return x % y;
    default: return 0;
  }
}

long eval_expr(mpc_ast_t *expr) {
  if (strstr(expr->tag, "nb"))
    // A number, immedialty parse from the string.
    return atoi(expr->contents);
  else {
    // An operator expression, let's parse it.
    //
    // It's really funny because the following lines feels normal yet
    // so wrong coming back from Rust programming.
    char op = expr->children[1]->contents[0]; // First is '(', we can jump to the second child and get the operator.
    long result = eval_expr(expr->children[2]); // eval first expression.

    for (unsigned int i = 3; expr->children[i]->contents[0] != ')'; ++i) {
      result = eval_op(op, result, eval_expr(expr->children[i]));
    }

    return result;
  }
}

long eval(mpc_ast_t *ast) {
  char op = ast->children[1]->contents[0]; // Move after > / regex
  long result = eval_expr(ast->children[2]); // eval first expression after the operator.

  for (unsigned int i = 3; i < ast->children_num - 1; ++i) { // children_num - 1 because of the encapsulating regex.
    result = eval_op(op, result, eval_expr(ast->children[i]));
  }

  return result;
}

void parse_user_input(pnp_t *pnp, char *input) {
  mpc_result_t r;

  if (mpc_parse("<stdin>", input, pnp->program, &r)) {
    printf("%ld\n", eval(r.output));
    mpc_ast_delete(r.output);
  } else {
    mpc_err_print(r.error);
    mpc_err_delete(r.error);
  }
}

void cleanup_polish_notation_parser(pnp_t *pnp) {
  mpc_cleanup(4, pnp->number, pnp->operator, pnp->expression, pnp->program);
}
