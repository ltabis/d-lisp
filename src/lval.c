#include "lval.h"

//  --------------
// | Constructors |
//  --------------

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

// Return an lval with a given symbol.
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

// Return an lval with a q-expression.
lval_t *lval_qexpr()
{
    lval_t *lval = malloc(sizeof(lval_t));

    if (!lval)
        return NULL;

    lval->type = QEXPR;
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

//  ----------------------------------
// | Reading the abstract syntax tree |
//  ----------------------------------

const char *compound_characters[COMPOUND_CHAR_COUNT] = {"(", ")", "{", "}"};

// Check if the given input is a compound statement character.
bool is_compound_character(const char *token)
{
    for (unsigned int j = 0; j < COMPOUND_CHAR_COUNT; ++j)
    {
        if (strcmp(token, compound_characters[j]) == 0)
            return true;
    }

    return false;
}

// Transform a ast value to a number.
lval_t *lval_read_num(const mpc_ast_t *ast)
{
    errno = 0;
    long number = strtol(ast->contents, NULL, 10);

    return errno == ERANGE ? lval_err(NUM_CONVERSION) : lval_num(number);
}

// Read a S or Q expression.
lval_t *lval_read_expr(const mpc_ast_t *ast, lval_type_t expr_type)
{
    if (expr_type != SEXPR && expr_type != QEXPR)
        return NULL;

    lval_t *expr = expr_type == SEXPR ? lval_sexpr() : lval_qexpr();

    for (unsigned int i = 0; i < ast->children_num; ++i)
    {
        if (is_compound_character(ast->children[i]->contents))
            continue;

        if (strcmp(ast->children[i]->tag, "regex") == 0)
            continue;

        expr = lval_add(expr, lval_read(ast->children[i]));
    }

    return expr;
}

lval_t *lval_read(const mpc_ast_t *ast)
{
    if (strstr(ast->tag, "number"))
        return lval_read_num(ast);
    else if (strstr(ast->tag, "symbol"))
        return lval_sym(ast->contents);
    else if (strstr(ast->tag, "sexpr") || strcmp(ast->tag, ">") == 0)
        return lval_read_expr(ast, SEXPR);
    else if (strstr(ast->tag, "qexpr"))
        return lval_read_expr(ast, QEXPR);
    else
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

//  ----------------------
// | evaluate expressions |
//  ----------------------

lval_t *lval_eval(lval_t *lval)
{
    if (lval->type == SEXPR)
        return lval_eval_expr(lval);

    return lval;
}

lval_t *lval_eval_expr(lval_t *lval)
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

//  -------------------------------
// | print the generated lval tree |
//  -------------------------------

static void lval_print_expr(const lval_t *lval, char begin, char end);

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
        lval_print_expr(lval, '(', ')');
        break;
    case QEXPR:
        lval_print_expr(lval, '{', '}');
        break;

    default:
        break;
    }
}

static void lval_print_expr(const lval_t *lval, char begin, char end)
{
    putchar(begin);

    for (unsigned int i = 0; i < lval->count; ++i)
    {
        lval_print(lval->cell[i]);

        if (i != lval->count - 1)
        {
            putchar(' ');
        }
    }

    putchar(end);
}

void lval_println(lval_t *lval)
{
    lval_print(lval);
    putchar('\n');
}

//  ----------------------------
// | Cleanup and error handling |
//  ----------------------------

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
    case QEXPR:
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
    // TODO: should be removed.
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
