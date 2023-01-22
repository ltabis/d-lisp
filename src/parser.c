#include <stdio.h>
#include "mpc.h"

// enable flycheck https://www.reddit.com/r/emacs/comments/audffp/tip_how_to_use_a_stable_and_fast_environment_to/

// Polish Notation Parser.
typedef struct pnp_s {
  mpc_parser_t *number;
  mpc_parser_t *operator;
  mpc_parser_t *expression;
  mpc_parser_t *program;
} pnp_t;

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
op : '+' | '-' | '*' | '/' ;       \
expr : <nb> | '(' <op> <nb>+ ')' ; \
dlisp : /^/ <op> <expr>+ /$/ ;     \
",
            number,
            operator,
            expression,
            program
);

    return parser;
}

void cleanup_polish_notation_parser(pnp_t *pnp) {
  mpc_cleanup(4, pnp->number, pnp->operator, pnp->expression, pnp->program);
}

int main(int argc, char **argv) {
  pnp_t pnp = init_polish_notation_parser();

  cleanup_polish_notation_parser(&pnp);
  return 0;
}
