#include "lval.h"

// TODO: fix memory leaks.

// Return a new valid lval with a number.
lval_t *lval_num(long value)
{
    lval_t *lval = malloc(sizeof(lval_t));

    if (!lval)
        return NULL;

    lval->type = NUMBER;
    lval->number = value;

    return lval;
}

// Return a new valid lval.
lval_t *lval_sym(const char *symbol)
{
    lval_t *lval = malloc(sizeof(lval_t));

    if (!lval)
        return NULL;

    lval->type = SYMBOL;
    lval->symbol = malloc(sizeof(char) * strlen(symbol) + 1);
    strcpy(lval->symbol, symbol);

    return lval;
}

lval_t *lval_sexpr()
{
    lval_t *lval = malloc(sizeof(lval_t));

    if (!lval)
        return NULL;

    lval->type = SEXPR;
    lval->count = 0;
    lval->cell = NULL;

    return lval;
}

// Return an lval with an error code.
lval_t *lval_err(lval_error_t error)
{
    lval_t *lval = malloc(sizeof(lval_t));

    if (!lval)
        return NULL;

    lval->type = ERROR;
    lval->error = error;

    return lval;
}

// Takes a lval error and return a human readable string.
char *interpret_lval_error(lval_error_t error)
{
    switch (error)
    {
    case NONE:
        return "there where no error";
    case DIV_BY_ZERO:
        return "tried to divide by zero";
    case UNKNOWN_OP:
        return "unknown operator";
    default:
        return "unknown error";
    }
}
