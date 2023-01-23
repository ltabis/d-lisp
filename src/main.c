#include <unistd.h>
#include "pnp.h"

#define INPUT_SIZE 2048
#define OK 0
#define ERR 1

int main() {
  pnp_t pnp = init_polish_notation_parser();
  char input[INPUT_SIZE] = {0};
  char *rd = NULL;

  fputs("d-lisp> ", stdout);

  while ((rd = fgets(input, INPUT_SIZE, stdin))) {

      if (strcmp(input, "exit\n") == 0) {
          puts("stoping d-lisp interpreter ...");
          break;
      }

      parse_user_input(&pnp, input);
      fputs("d-lisp> ", stdout);
  }

  if (rd != OK) {
      return ERR;
  }

  cleanup_polish_notation_parser(&pnp);

  return OK;
}
