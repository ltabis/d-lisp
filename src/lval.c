#include "lval.h"

//  --------------
// | Constructors |
//  --------------

// Return a new environment.
lenv_t *lenv_new()
{
    lenv_t *env = malloc(sizeof(lenv_t));

    // FIXME: probably overkill.
    if (!env)
        return NULL;

    env->parent = NULL;
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

// Return an lval with a function pointer to a builtin function.
lval_t *lval_fun(lbuiltin function)
{
    lval_t *lval = malloc(sizeof(lval_t));

    if (!lval)
        return NULL;

    lval->type = FUN;
    lval->builtin = function;

    return lval;
}

lval_t *lval_lambda(lval_t *formals, lval_t *body)
{
    lval_t *lval = malloc(sizeof(lval_t));

    if (!lval)
        return NULL;

    lval->type = FUN;
    lval->builtin = NULL;
    lval->env = lenv_new();
    lval->formals = formals;
    lval->body = body;

    return lval;
}

// Return an lval with an error code.
lval_t *lval_err(const char *fmt, ...)
{
    lval_t *lval = malloc(sizeof(lval_t));

    if (!lval)
        return NULL;

    lval->type = ERROR;

    va_list va;
    va_start(va, fmt);

    // Write the error message using variable arguments.
    lval->error = malloc(sizeof(char) * 512);
    vsnprintf(lval->error, 511, fmt, va);

    va_end(va);

    // Resize the string's memory to it's actual character size.
    lval->error = realloc(lval->error, strlen(lval->error) + 1);

    return lval;
}

// Clone a copy of a lval matching a symbol name.
// Return an error if the symbol could not be found.
lval_t *lenv_get(lenv_t *env, const char *sym)
{
    for (size_t i = 0; i < env->count; i++) {
        if (strcmp(sym, env->syms[i]) == 0) {
            return lval_clone(env->vals[i]);
        }
    }

    if (env->parent)
    {
        return lenv_get(env->parent, sym);
    }

    return lval_err("symbol '%s' not found", sym);
}

// Push a new lval to the local env, replace an existing value
// if the key is already part of the environment.
void lenv_push(lenv_t *env, lval_t *key, lval_t *value)
{
    for (size_t i = 0; i < env->count; ++i) {
        if (strcmp(key->symbol, env->syms[i]) == 0) {
            lval_del(env->vals[i]);
            env->vals[i] = lval_clone(value);
            return;
        }
    }

    env->count++;
    env->syms = realloc(env->syms, sizeof(lval_t *) * env->count);
    env->vals = realloc(env->vals, sizeof(char *) * env->count);

    env->syms[env->count - 1] = strdup(key->symbol);
    env->vals[env->count - 1] = lval_clone(value);
}

// Push a new lval to the global env, replace an existing value
// if the key is already part of the environment.
void lenv_def(lenv_t *env, lval_t *key, lval_t *value)
{
    for (; env->parent; env = env->parent);
    lenv_push(env, key, value);
}

// Add a builtin function pointer to an environment.
void lenv_add_builtin(lenv_t *env, const char *name, lbuiltin function)
{
    lval_t *sym = lval_sym(name);
    lval_t *fun = lval_fun(function);

    lenv_push(env, sym, fun);

    lval_del(sym);
    lval_del(fun);
}

// Add all builtins function pointer to an environment.
void lenv_add_builtins(lenv_t *env)
{
    lenv_add_builtin(env, "+", &builtin_op_add);
    lenv_add_builtin(env, "-", &builtin_op_sub);
    lenv_add_builtin(env, "/", &builtin_op_div);
    lenv_add_builtin(env, "*", &builtin_op_mul);
    lenv_add_builtin(env, "%", &builtin_op_mod);
    lenv_add_builtin(env, "head", &builtin_head);
    lenv_add_builtin(env, "tail", &builtin_tail);
    lenv_add_builtin(env, "list", &builtin_list);
    lenv_add_builtin(env, "eval", &builtin_eval);
    lenv_add_builtin(env, "join", &builtin_join);
    lenv_add_builtin(env, "def", &builtin_def);
    lenv_add_builtin(env, "=", &builtin_push);
    lenv_add_builtin(env, "\\", &builtin_lambda);
    lenv_add_builtin(env, "fn", &builtin_fn);
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

    return errno == ERANGE ? lval_err("Expression '%s' is not a number", ast->contents) : lval_num(number);
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
        return lval_err("pop index out of scope (index: %u, size: %u)", index, lval->count - 1);
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

lval_t *lval_eval(lenv_t *env, lval_t *lval)
{
    if (lval->type == SYMBOL) {
        lval_t *val = lenv_get(env, lval->symbol);
        lval_del(lval);
        return val;
    }

    if (lval->type == SEXPR)
        return lval_eval_sexpr(env, lval);

    return lval;
}

// Evaluate a s-expr. The first element of an s-expr must be a function.
lval_t *lval_eval_sexpr(lenv_t *env, lval_t *lval)
{
    for (unsigned int i = 0; i < lval->count; ++i)
    {
        lval->cell[i] = lval_eval(env, lval->cell[i]);

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

    if (first->type != FUN)
    {
        lval_del(first);
        lval_del(lval);
        return lval_err("The first element of a S-Expression must be a function");
    }

    return lval_call(env, first, lval);
}

// TODO: Could implement partially initialize functions like in the book.
//       See https://buildyourownlisp.com/chapter12_functions
lval_t *lval_call(lenv_t *env, lval_t *func, lval_t *args)
{
    if (func->builtin)
        return func->builtin(env, args);

    LASSERT(func, func->formals->count == args->count,
        "lambda expected %ld parameter, got %ld", func->formals->count, args->count
    );

    for (size_t i = 0; i < args->count; ++i)
    {
        lenv_push(func->env, func->formals->cell[i], args->cell[i]);
    }

    lval_del(args);

    func->env->parent = env;

    return builtin_eval(func->env, lval_add(lval_sexpr(), lval_clone(func->body)));
}


/// @brief evaluate an math operator on a list of numbers.
/// @param lval
/// @return the result of the evaluation.
lval_t *builtin_op(lenv_t *env, lval_t *lval, char *symbol)
{
    for (unsigned int i = 0; i < lval->count; ++i)
    {
        if (lval->cell[i]->type != NUMBER)
        {
            lval_del(lval);
            return lval_err("Numerical operators can only be applied to numbers");
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

lval_t *builtin_op_add(lenv_t *env, lval_t *lval)
{
    return builtin_op(env, lval, "+");
}

lval_t *builtin_op_sub(lenv_t *env, lval_t *lval)
{
    return builtin_op(env, lval, "-");
}

lval_t *builtin_op_div(lenv_t *env, lval_t *lval)
{
    return builtin_op(env, lval, "/");
}

lval_t *builtin_op_mul(lenv_t *env, lval_t *lval)
{
    return builtin_op(env, lval, "*");
}

lval_t *builtin_op_mod(lenv_t *env, lval_t *lval)
{
    return builtin_op(env, lval, "%");
}

/// @brief get the head of a qexpr and deletes the tail.
/// @param lval
/// @return the popped head from a qexpr.
lval_t *builtin_head(lenv_t *env, lval_t *lval)
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
lval_t *builtin_tail(lenv_t *env, lval_t *lval)
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
lval_t *builtin_list(lenv_t *env, lval_t *lval)
{
    LASSERT(lval, lval->type == SEXPR, "`list` symbol can only be applied to a S-Expression");

    lval->type = QEXPR;
    return lval;
}

/// @brief Evaluate a qexpr by transforming it into a sexpr.
/// @param lval
/// @return the result of the evaluation.
lval_t *builtin_eval(lenv_t *env, lval_t *lval)
{
    LASSERT(lval, lval->count == 1 && lval->cell[0]->type == QEXPR, "`eval` symbol can only be applied to a Q-Expression");

    lval_t *q = lval_take(lval, 0);
    q->type = SEXPR;
    return lval_eval(env, q);
}

/// @brief join n-qexpr together.
/// @param lval
/// @return the merged qexpr.
lval_t *builtin_join(lenv_t *env, lval_t *lval)
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

lval_t *builtin_var(lenv_t *env, lval_t *lval, const char *function)
{
    LASSERT(lval, lval->cell[0]->type == QEXPR, "`def` function can only be applied to a Q-Expression of symbols followed by any expression");
    LASSERT(lval, lval->cell[0]->count == lval->count - 1, "the number of variables must be the same as values when using the `def` function");

    lval_t *symbols = lval->cell[0];

    for (size_t i = 0; i < symbols->count; ++i) {
        LASSERT(symbols, symbols->cell[i]->type == SYMBOL, "all members of the q-expression after the `def` function must be symbols");
    }

    for (size_t i = 0; i < symbols->count; ++i) {

        if (strcmp(function, "def") == 0)
        {
            lenv_def(env, symbols->cell[i], lval->cell[i + 1]);
        } else if (strcmp(function, "def") == 0) {
            lenv_push(env, symbols->cell[i], lval->cell[i + 1]);
        }
    }

    lval_del(lval);
    return lval_sexpr();
}

lval_t *builtin_def(lenv_t *env, lval_t *lval)
{
    return builtin_var(env, lval, "def");
}

lval_t *builtin_push(lenv_t *env, lval_t *lval)
{
    return builtin_var(env, lval, "=");
}

// Create a lambda function from two q-expr, one for the arguments,
// and the other for the body.
//
// NOTE: you can create a function naming syntax using:
//       `def {fun} (\ {args body} {def (head args) (\ (tail args) body)})`
//       It defines a function that creates a lambda from `args` (the first
//       one is the name of the function) and a `body`.
//       e.g. `fun {add x y} {+ x y}`
lval_t *builtin_lambda(lenv_t *env, lval_t *lval)
{
    LASSERT_NUM_PARAMS("\\", lval, 2);
    LASSERT_TYPE("\\", lval, 0, QEXPR);
    LASSERT_TYPE("\\", lval, 1, QEXPR);

    for (size_t i = 0; i < lval->cell[0]->count ;++i)
    {
        // FIXME: this is wrong because on error lval wont be freed,
        //        only lval->cell[0].
        LASSERT_TYPE("\\", lval->cell[0], i, SYMBOL);
    }

    lval_t *formals = lval_pop(lval, 0);
    lval_t *body = lval_pop(lval, 0);
    lval_del(lval);

    return lval_lambda(formals, body);
}

lval_t *builtin_fn(lenv_t *env, lval_t *lval)
{
    LASSERT_NUM_PARAMS("fn", lval, 2);
    LASSERT_TYPE("fn", lval, 0, QEXPR);
    LASSERT_TYPE("fn", lval, 1, QEXPR);

    for (size_t i = 0; i < lval->cell[1]->count ;++i)
    {
        // FIXME: this is wrong because on error lval wont be freed,
        //        only lval->cell[1].
        LASSERT_TYPE("fn", lval->cell[0], i, SYMBOL);
    }

    lval_t *formals = lval_pop(lval, 0);
    lval_t *name = lval_pop(formals, 0);
    lval_t *body = lval_pop(lval, 0);
    lval_t *function = lval_lambda(formals, body);

    lenv_def(env, name, function);
    lval_del(lval);

    return function;
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
        printf("Error: %s.", lval->error);
        break;
    case SEXPR:
        lval_print_expr(lval, '(', ')');
        break;
    case QEXPR:
        lval_print_expr(lval, '{', '}');
        break;
    case FUN:
        if (lval->builtin) {
            puts("<builtin>");
        } else {
            printf("(\\");
            lval_print(lval->formals);
            putchar(' ');
            lval_print(lval->body);
            putchar(')');
        }
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

char *lval_type_name(unsigned int t)
{
  switch (t)
  {
    case NUMBER: return "Number";
    case FUN: return "Function";
    case ERROR: return "Error";
    case SYMBOL: return "Symbol";
    case SEXPR: return "S-Expression";
    case QEXPR: return "Q-Expression";
    default: return "Unknown";
  }
}

//  -------------------
// | lval manipulation |
//  -------------------

lenv_t *lenv_clone(lenv_t *env)
{
    lenv_t *new = malloc(sizeof(lenv_t));

    // We never clone the parent environment because it is shared.
    new->parent = env->parent;

    new->count = env->count;
    new->syms = malloc(sizeof(char *) * new->count);
    new->vals = malloc(sizeof(lval_t *) * new->count);

    for (size_t i = 0; i < new->count; ++i)
    {
        new->syms[i] = strdup(env->syms[i]);
        new->vals[i] = lval_clone(env->vals[i]);
    }

    return new;
}

lval_t *lval_clone(lval_t *lval)
{
    lval_t *new = malloc(sizeof(lval_t));

    if (!new) return NULL;

    new->type = lval->type;

    switch (new->type) {
        case NUMBER: new->number = lval->number; break;
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
        case FUN:
            if (lval->builtin)
            {
                new->builtin = lval->builtin;
            } else {
                new->builtin = NULL;
                new->env = lenv_clone(lval->env);
                new->formals = lval_clone(lval->formals);
                new->body = lval_clone(lval->body);
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
        if (!lval->builtin)
        {
            lenv_del(lval->env);
            lval_del(lval->formals);
            lval_del(lval->body);
        }
        break;
    default:
        // FIXME: Should crash the program because all enum values should be handled.
        return;
        break;
    }

    free(lval);
}