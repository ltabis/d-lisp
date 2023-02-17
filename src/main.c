#include <unistd.h>
#include "sep.h"

#define INPUT_SIZE 2048
#define OK 0
#define ERR 1

int main()
{
    sep_t parser = init_parser();
    char input[INPUT_SIZE] = {0};
    char *rd = NULL;
    lenv_t *env = lenv_new();

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
