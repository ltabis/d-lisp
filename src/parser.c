#include <stdio.h>
#include "mpc.h"

// enable flycheck https://www.reddit.com/r/emacs/comments/audffp/tip_how_to_use_a_stable_and_fast_environment_to/

int main(int argc, char **argv) {
  return 0;
}

mpc_parser_t *doge_parser() {
  mpc_parser_t *adjectives = mpc_or(4,
                                    mpc_sym("wow"),
                                    mpc_sym("many"),
                                    mpc_sym("so"),
                                    mpc_sym("such")
                                    );
  mpc_parser_t *nouns = mpc_or(5,
                               mpc_sym("lisp"),
                               mpc_sym("languages"),
                               mpc_sym("book"),
                               mpc_sym("build"),
                               mpc_sym("c")
                               );
  mpc_parser_t *phrase = mpc_and(2, mpcf_strfold, adjectives, nouns, free);
  mpc_parser_t *doge = mpc_many(mpcf_strfold, phrase);

  return doge;
}
