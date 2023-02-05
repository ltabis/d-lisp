#include "lval.h"

// Return a new valid lval.
lval_t new_lval_ok(long value)
{
    return (lval_t){
        .value = value,
        .error = NONE,
    };
}

// Return an lval with an error code.
lval_t new_lval_err(lval_error_t error)
{
    return (lval_t){
        .value = 0,
        .error = error,
    };
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
