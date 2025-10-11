
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <getopt.h>
#include <string.h>

#include "../cfgparser.h"

static void
errfunc(const char* s)
{
  puts(s);
}

int
main(int argc, char** argv)
{
  int opt;
  char file[128];
  while ((opt = getopt(argc, argv, "f:")) != -1) {
    switch (opt) {
    case 'f':
      strcpy(file, optarg);
      break;
    }
  }
  
  FILE* fp = fopen(file, "rb");
  if (!fp) {
    perror("fopen");
    return -1;
  }
  
  fseek(fp, 0, SEEK_END);
  size_t end = ftell(fp);
  fseek(fp, 0, SEEK_SET);
  
  char* b = calloc(end + 1, 1);
  
  fread(b, 1, end, fp);
  
  cfg_file_t* f = cfg_parse(b, errfunc);
  
  if (f) {
    cfg_dump(f);
  }
  else {
    exit(1);
  }
  
  cfg_free(f);
  
  fclose(fp);
  free(b);
}