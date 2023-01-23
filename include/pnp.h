#ifndef PNP_H_
#define PNP_H_

#include "mpc.h"

// Polish Notation Parser.
typedef struct pnp_s {
  mpc_parser_t *number;
  mpc_parser_t *operator;
  mpc_parser_t *expression;
  mpc_parser_t *program;
} pnp_t;

// Create a new polish notation parser.
pnp_t init_polish_notation_parser();
// Parse an input using a specific parser and print the Ast or an error.
void parse_user_input(pnp_t *pnp, char *input);
// Free a polish notation parser.
void cleanup_polish_notation_parser(pnp_t *pnp);

#endif // PNP_H_
