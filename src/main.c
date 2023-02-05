#include <unistd.h>
#include "sep.h"

#define INPUT_SIZE 2048
#define OK 0
#define ERR 1

int main()
{
    sep_t parser = init_polish_notation_parser();
    char input[INPUT_SIZE] = {0};
    char *rd = NULL;

    fputs("d-lisp> ", stdout);

    while ((rd = fgets(input, INPUT_SIZE, stdin)))
    {

        if (strcmp(input, "exit\n") == 0)
        {
            puts("stopping d-lisp interpreter ...");
            break;
        }

        parse_user_input(&parser, input);
        fputs("d-lisp> ", stdout);
    }

    if (rd != OK)
    {
        return ERR;
    }

    cleanup_polish_notation_parser(&parser);

    return OK;
}
