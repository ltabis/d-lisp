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
    default:
        return "unknown error";
    }
}
