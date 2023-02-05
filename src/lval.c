#include "lval.h"

// Return a lval with a number.
lval_t *lval_num(long value)
{
    lval_t *lval = malloc(sizeof(lval_t));

    if (!lval)
        return NULL;

    lval->type = NUMBER;
    lval->number = value;

    return lval;
}

// Return an lval with the given symbol.
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

// Return an lval with an s-expression.
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

// Transform a ast value to a number.
lval_t *lval_read_num(const mpc_ast_t *ast)
{
    errno = 0;
    long number = strtol(ast->contents, NULL, 10);

    return errno == ERANGE ? lval_err(NUM_CONVERSION) : lval_num(number);
}

lval_t *lval_read(const mpc_ast_t *ast)
{
    if (strstr(ast->tag, "number"))
        return lval_read_num(ast);
    if (strstr(ast->tag, "symbol"))
        return lval_sym(ast->contents);

    if (!strcmp(ast->tag, ">") || strstr(ast->tag, "sexpr"))
    {
        lval_t *sexpr = lval_sexpr();

        for (unsigned int i = 0; i < ast->children_num; ++i)
        {
            if (strcmp(ast->children[i]->contents, "(") == 0)
                continue;
            if (strcmp(ast->children[i]->contents, ")") == 0)
                continue;
            if (strcmp(ast->children[i]->tag, "regex") == 0)
                continue;

            sexpr = lval_add(sexpr, lval_read(ast->children[i]));
        }

        return sexpr;
    }

    return NULL;
}

lval_t *lval_add(lval_t *dest, lval_t *other)
{
    dest->count++;
    dest->cell = realloc(dest->cell, sizeof(lval_t *) * dest->count);
    dest->cell[dest->count - 1] = other;

    return dest;
}

lval_t *lval_pop(lval_t *lval, unsigned int index)
{
    if (index <= lval->count - 1)
    {
        lval_t *pop = lval->cell[index];

        memmove(&lval->cell[index], &lval->cell[index + 1], sizeof(lval_t *) * lval->count - index - 1);
        lval->count--;
        lval->cell = realloc(lval->cell, sizeof(lval_t *) * lval->count);

        return pop;
    }
    else
    {
        return lval_err(POP_OUT_OF_SCOPE);
    }
}

lval_t *lval_take(lval_t *lval, unsigned int index)
{
    lval_t *pop = lval_pop(lval, index);
    lval_del(lval);
    return pop;
}

lval_t *lval_eval(lval_t *lval)
{
    if (lval->type == SEXPR)
        return lval_eval_sexpr(lval);

    return lval;
}

lval_t *lval_eval_sexpr(lval_t *lval)
{

    for (unsigned int i = 0; i < lval->count; ++i)
    {
        lval->cell[i] = lval_eval(lval->cell[i]);

        if (lval->cell[i]->type == ERROR)
            return lval_take(lval, i);
    }

    if (lval->count == 0)
    {
        return lval;
    }

    if (lval->count == 1)
    {
        return lval_take(lval, 0);
    }

    lval_t *first = lval_pop(lval, 0);

    if (first->type != SYMBOL)
    {
        lval_del(first);
        lval_del(lval);
        return lval_err(FIRST_EXPR_NOT_SYMBOL);
    }
    else
    {
        lval_t *result = builtin_op(lval, first->symbol);
        lval_del(first);
        return result;
    }
}

lval_t *builtin_op(lval_t *lval, char *symbol)
{
    for (unsigned int i = 0; i < lval->count; ++i)
    {
        if (lval->cell[i]->type != NUMBER)
        {
            lval_del(lval);
            return lval_err(NOT_A_NUM);
        }
    }

    lval_t *result = lval_pop(lval, 0);

    if (lval->count == 0 && strcmp(symbol, "-") == 0)
    {
        result->number = -result->number;
    }

    while (lval->count != 0)
    {
        lval_t *next = lval_pop(lval, 0);

        if (strcmp(symbol, "-") == 0)
            result->number -= next->number;
        else if (strcmp(symbol, "+") == 0)
            result->number += next->number;
        else if (strcmp(symbol, "*") == 0)
            result->number *= next->number;
        else if (strcmp(symbol, "/") == 0)
            if (next->number)
                result->number /= next->number;
            else
            {
                lval_del(next);
                lval_del(result);
                lval_del(lval);
                return lval_err(DIV_BY_ZERO);
            }
        else if (strcmp(symbol, "%") == 0)
            result->number %= next->number;

        lval_del(next);
    }

    lval_del(lval);
    return result;
}

static void lval_print_sexpr(const lval_t *lval);

static void lval_print(const lval_t *lval)
{
    switch (lval->type)
    {
    case NUMBER:
        printf("%ld", lval->number);
        break;
    case SYMBOL:
        printf("%s", lval->symbol);
        break;
    case ERROR:
        printf("Error: %s", lval_interpret_error(lval->error));
        break;
    case SEXPR:
        lval_print_sexpr(lval);
        break;

    default:
        break;
    }
}

static void lval_print_sexpr(const lval_t *lval)
{
    putchar('(');

    for (unsigned int i = 0; i < lval->count; ++i)
    {
        lval_print(lval->cell[i]);

        if (i != lval->count - 1)
        {
            putchar(' ');
        }
    }

    putchar(')');
}

void lval_println(lval_t *lval)
{
    lval_print(lval);
    putchar('\n');
}

// Clean up a lval and all of it's nodes.
void lval_del(lval_t *lval)
{
    switch (lval->type)
    {
    case NUMBER:
        break;
    case SYMBOL:
        free(lval->symbol);
        break;
    case SEXPR:
        for (unsigned int i = 0; i < lval->count; ++i)
        {
            lval_del(lval->cell[i]);
        }
        free(lval->cell);
        break;

    default:
        break;
    }

    free(lval);
}

// Takes a lval error and return a human readable string.
char *lval_interpret_error(lval_error_t error)
{
    switch (error)
    {
    case NONE:
        return "there where no error";
    case DIV_BY_ZERO:
        return "tried to divide by zero";
    case UNKNOWN_OP:
        return "unknown operator";
    case NUM_CONVERSION:
        return "failed to convert number";
    case NOT_A_NUM:
        return "element after a symbol must be a number";
    case FIRST_EXPR_NOT_SYMBOL:
        return "the first element of an s-expression must be a symbol";
    default:
        return "unknown error";
    }
}
