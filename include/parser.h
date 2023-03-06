#ifndef PARSER_H_
#define PARSER_H_

#include "mpc.h"

// S-expression parser.
typedef struct sep_s
{
  mpc_parser_t *number;
  mpc_parser_t *string;
  mpc_parser_t *symbol;
  mpc_parser_t *comment;
  mpc_parser_t *expr;
  mpc_parser_t *sexpr;
  mpc_parser_t *qexpr;
  mpc_parser_t *program;
} sep_t;

// Create a new d-list parser.
sep_t init_parser();
// Cleanup memory of a d-list parser.
void cleanup_parser(sep_t *parser);

#endif // PARSER_H_
