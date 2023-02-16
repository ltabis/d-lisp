#include "lval.h"

//  --------------
// | Constructors |
//  --------------

// Return a new environment.
lenv_t *lenv_new()
{
    lenv_t *env = malloc(sizeof(lenv_t));

    if (!env)
        return NULL;

    env->count = 0;
    env->syms = NULL;
    env->vals = NULL;

    return env;
}

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

// Return an lval with a function pointer.
lval_t *lval_fun(lbuiltin function)
{
    lval_t *lval = malloc(sizeof(lval_t));

    if (!lval)
        return NULL;

    lval->type = FUN;
    lval->function = function;

    return lval;
}


// Return an lval with an error code.
lval_t *lval_err(const char *msg)
{
    lval_t *lval = malloc(sizeof(lval_t));

    if (!lval)
        return NULL;

    lval->type = ERROR;
    lval->error = strdup(msg);

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

    return errno == ERANGE ? lval_err("Expression is not a number") : lval_num(number);
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

/// @brief Pops an lval from an expression.
/// This function reallocates the lval from which the element has been popped.
/// @param lval lval to pop an element from.
/// @param index which lval to pop.
/// @return the popped lval.
lval_t *lval_pop(lval_t *lval, unsigned int index)
{
    if (index <= lval->count - 1)
    {
        lval_t *pop = lval->cell[index];

        // Move pointers to the start of the given index to remove the desired element.
        memmove(&lval->cell[index], &lval->cell[index + 1], sizeof(lval_t *) * (lval->count - index - 1));
        lval->count--;
        lval->cell = realloc(lval->cell, sizeof(lval_t *) * lval->count);

        return pop;
    }
    else
    {
        return lval_err("Trying to pop a lval using an out of scope index");
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
        return lval_err("The first element of a S-Expression must be a symbol");
    }
    else
    {
        if (strcmp(first->symbol, "head") == 0)
            return builtin_head(lval);
        else if (strcmp(first->symbol, "tail") == 0)
            return builtin_tail(lval);
        else if (strcmp(first->symbol, "list") == 0)
            return builtin_list(lval);
        else if (strcmp(first->symbol, "eval") == 0)
            return builtin_eval(lval);
        else if (strcmp(first->symbol, "join") == 0)
            return builtin_join(lval);
        else if (strstr("-+/*%", first->symbol))
            return builtin_op(lval, first->symbol);

        lval_del(first);
        // FIXME: Will never branch here because mpc only parses the symbols
        //        defined above. I guess i'll let this here in case I implement
        //        my own parser.
        return lval_err("unknown symbol");
    }
}

/// @brief evaluate an math operator on a list of numbers.
/// @param lval
/// @return the result of the evaluation.
lval_t *builtin_op(lval_t *lval, char *symbol)
{
    for (unsigned int i = 0; i < lval->count; ++i)
    {
        if (lval->cell[i]->type != NUMBER)
        {
            lval_del(lval);
            return lval_err("Numerical operators can only be applied to number");
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
                return lval_err("Cannot divide by zero");
            }
        else if (strcmp(symbol, "%") == 0)
            result->number %= next->number;

        lval_del(next);
    }

    lval_del(lval);
    return result;
}

/// @brief get the head of a qexpr and deletes the tail.
/// @param lval
/// @return the popped head from a qexpr.
lval_t *builtin_head(lval_t *lval)
{
    LASSERT(lval, lval->count == 1 && lval->cell[0]->type == QEXPR, "`head` symbol can only be applied to one Q-Expression");
    LASSERT(lval, lval->cell[0]->count >= 1, "`head` symbol cannot be applied to an empty Q-Expression");

    lval_t *q = lval_take(lval, 0);

    while (q->count > 1)
    {
        lval_del(lval_pop(q, 1));
    }

    return q;
}

/// @brief get the tail of a qexpr and deletes the head.
/// @param lval
/// @return the tail of a qexpr.
lval_t *builtin_tail(lval_t *lval)
{
    LASSERT(lval, lval->count == 1 && lval->cell[0]->type == QEXPR, "`tail` symbol can only be applied to one Q-Expression");
    LASSERT(lval, lval->cell[0]->count >= 1, "`tail` symbol cannot be applied to an empty Q-Expression");

    lval_t *q = lval_take(lval, 0);

    lval_del(lval_pop(q, 0));

    return q;
}

/// @brief Transform a sexpr into a qexpr.
/// @param lval
/// @return a qexpr.
lval_t *builtin_list(lval_t *lval)
{
    LASSERT(lval, lval->type == SEXPR, "`list` symbol can only be applied to a S-Expression");

    lval->type = QEXPR;
    return lval;
}

/// @brief Evaluate a qexpr by transforming it into a sexpr.
/// @param lval
/// @return the result of the evaluation.
lval_t *builtin_eval(lval_t *lval)
{
    LASSERT(lval, lval->count == 1 && lval->cell[0]->type == QEXPR, "`eval` symbol can only be applied to a Q-Expression");

    lval_t *q = lval_take(lval, 0);
    q->type = SEXPR;
    return lval_eval(q);
}

/// @brief join n-qexpr together.
/// @param lval
/// @return the merged qexpr.
lval_t *builtin_join(lval_t *lval)
{
    unsigned int req_space = 0;

    for (unsigned int i = 0; i < lval->count; ++i)
    {
        LASSERT(lval, lval->cell[i]->type == QEXPR, "`join` symbol can only be applied to Q-Expressions")
        req_space += lval->cell[i]->count;
    }

    lval_t *join = lval_qexpr();

    join->count = req_space;
    join->cell = malloc(sizeof(lval_t *) * req_space);

    for (unsigned int i = 0; lval->count;)
    {
        lval_t *next = lval_pop(lval, 0);

        while (next->count) {
            join->cell[i] = lval_pop(next, 0);
            i++;
        }

        lval_del(next);
    }

    lval_del(lval);
    return join;
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
        printf("Error: %s", lval->error);
        break;
    case SEXPR:
        lval_print_expr(lval, '(', ')');
        break;
    case QEXPR:
        lval_print_expr(lval, '{', '}');
        break;
    case FUN:
        puts("<function>");
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

//  -------------------
// | lval manipulation |
//  -------------------

lval_t *lval_clone(lval_t *lval)
{
    lval_t *new = malloc(sizeof(lval_t));

    if (!new) return NULL;

    new->type = lval->type;

    switch (new->type) {
        case NUMBER: new->number = lval->number; break;
        case FUN: new->function = lval->function; break;
        case SYMBOL: new->symbol = strdup(lval->symbol); break;
        case ERROR: new->error = strdup(lval->error); break;
        case SEXPR:
        case QEXPR:
            new->count = lval->count;
            new->cell = malloc(sizeof(lval_t *) * new->count);

            for (unsigned int i = 0; i < new->count; ++i)
            {
                new->cell[i] = lval_clone(lval->cell[i]);
            }
            break;
        default:
            // FIXME: Should crash the program because all enum values should be handled.
            return NULL;
            break;
    }

    return new;
}

// Clean up the environment.
void lenv_del(lenv_t *env)
{
    for (size_t i = 0; i < env->count; ++i) {
        free(env->syms[i]);
        lval_del(env->vals[i]);
    }

    free(env->syms);
    free(env->vals);
    free(env);
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
    case QEXPR:
        for (unsigned int i = 0; i < lval->count; ++i)
        {
            lval_del(lval->cell[i]);
        }
        free(lval->cell);
        break;
    case FUN:
    default:
        // FIXME: Should crash the program because all enum values should be handled.
        return;
        break;
    }

    free(lval);
}