#include "parser.h"

sep_t init_parser()
{
  mpc_parser_t *number = mpc_new("number");
  mpc_parser_t *string = mpc_new("string");
  mpc_parser_t *symbol = mpc_new("symbol");
  mpc_parser_t *expr = mpc_new("expr");
  mpc_parser_t *sexpr = mpc_new("sexpr");
  mpc_parser_t *qexpr = mpc_new("qexpr");
  mpc_parser_t *comment = mpc_new("comment");
  mpc_parser_t *program = mpc_new("dlisp");

  sep_t parser = {
      .number = number,
      .string = string,
      .symbol = symbol,
      .expr = expr,
      .sexpr = sexpr,
      .qexpr = qexpr,
      .comment = comment,
      .program = program};

  mpca_lang(MPCA_LANG_DEFAULT,
            "                                         \
number  : /-?[0-9]+/ ;                                \
string  : /\"(\\\\.|[^\"])*\"/ ;                      \
symbol  : /[a-zA-Z0-9_+\\-*\\/\\\\=<>!&]+/ ;          \
sexpr   : '(' <expr>* ')' ;                           \
qexpr   : '{' <expr>* '}' ;                           \
expr    : <number> | <string> | <symbol> | <sexpr> |  \
         <qexpr> ;                                    \
comment : /;[^\\r\\n]*/ ;                             \
dlisp   : /^/ <expr>* /$/ ;                           \
",
            number,
            string,
            symbol,
            sexpr,
            qexpr,
            expr,
            comment,
            program);

  return parser;
}

void cleanup_parser(sep_t *parser)
{
  mpc_cleanup(8,
              parser->number,
              parser->string,
              parser->symbol,
              parser->sexpr,
              parser->qexpr,
              parser->expr,
              parser->comment,
              parser->program);
}
