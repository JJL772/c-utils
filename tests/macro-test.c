
#include "../macro-tools.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

int
main(int argc, char** argv)
{
  for (int i = 1; i < argc; ++i) {
    char* s = expand_macro_string_env(argv[i]);
    puts(s);
    free(s);
  }
  return 0;
}