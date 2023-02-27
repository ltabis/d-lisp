#include <unistd.h>
#include "lval.h"

#define INPUT_SIZE 2048
#define OK 0
#define ERR 1

void parse_user_input(lenv_t *env, sep_t *parser, char *input)
{
  mpc_result_t r;

  if (mpc_parse("<stdin>", input, parser->program, &r))
  {
    lval_t *lval = lval_eval(env, lval_read(r.output));
    lval_println(lval);
    lval_del(lval);
    mpc_ast_delete(r.output);
  }
  else
  {
    mpc_err_print(r.error);
    mpc_err_delete(r.error);
  }
}

int main()
{
    sep_t parser = init_parser();
    char input[INPUT_SIZE] = {0};
    char *rd = NULL;
    lenv_t *env = lenv_new(&parser);

    lenv_add_builtins(env);

    fputs("d-lisp> ", stdout);

    while ((rd = fgets(input, INPUT_SIZE, stdin)))
    {

        if (strcmp(input, "exit\n") == 0)
        {
            puts("stopping d-lisp interpreter ...");
            break;
        }

        parse_user_input(env, &parser, input);
        fputs("d-lisp> ", stdout);
    }

    if (rd != OK)
    {
        return ERR;
    }

    cleanup_parser(&parser);
    lenv_del(env);

    return OK;
}