#ifndef SEP_H_
#define SEP_H_

#include "mpc.h"
#include "lval.h"

// S-expression parser.
typedef struct sep_s
{
  mpc_parser_t *number;
  mpc_parser_t *symbol;
  mpc_parser_t *string;
  mpc_parser_t *expr;
  mpc_parser_t *sexpr;
  mpc_parser_t *qexpr;
  mpc_parser_t *program;
} sep_t;

// Create a new d-list parser.
sep_t init_parser();
// Parse an input using a specific parser and print the Ast or an error.
void parse_user_input(lenv_t *env, sep_t *parser, char *input);
// Cleanup memory of a d-list parser.
void cleanup_parser(sep_t *parser);

#endif // SEP_H_
