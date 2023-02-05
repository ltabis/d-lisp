#ifndef SEP_H_
#define SEP_H_

#include "mpc.h"
#include "lval.h"

// S-expression parser.
typedef struct sep_s
{
  mpc_parser_t *number;
  mpc_parser_t *operator;
  mpc_parser_t *expression;
  mpc_parser_t *program;
} sep_t;

// Create a new polish notation parser.
sep_t init_polish_notation_parser();
// Parse an input using a specific parser and print the Ast or an error.
void parse_user_input(sep_t *parser, char *input);
// Free a polish notation parser.
void cleanup_polish_notation_parser(sep_t *parser);

#endif // SEP_H_
