#include "../glob.h"

#include <stdio.h>

#define TEST(_a, _b, _exp) \
  do { \
  if (globish((_a), (_b)) != (_exp)) \
    printf("\033[033mglobish(%s, %s) != %d!!!\033[0m\n", (_a), (_b), (_exp)); \
  else \
    printf("\033[032mglobish(%s, %s) passed\033[0m\n", (_a), (_b)); \
  } while (0)

int
main()
{
  TEST("*", "test", 1);
  TEST("*", "", 1);
  TEST("test", "test", 1);
  TEST("*.txt", "test.txt", 1);
  TEST("*.txt", ".txt", 1);
  TEST("*.txt", "txt", 0);
  TEST("?.txt", "t.txt", 1);
  TEST("?.txt", ".txt", 0);
  TEST("test.?xt", "test.txt", 1);
  TEST("*.txt", ".gitkeep", 0);
  TEST(".gitkeep", ".gitkeep", 1);
  TEST("AaAa", "aaaa", 1);
  TEST("*.HL?", "mysave.HL3", 1);
  TEST("*.hl?", "mysave.HL3", 1);
  TEST("*.hl?", "mysave.HL", 0);
  TEST("*.HL?", "mysave.SAV", 0);
  TEST("*.*", "mysave.SAV", 1);
  TEST("*.*", "mysave", 0);
  TEST("*.*", "test.", 0);
}