#include "code_optimization.h"
#include "code_translation.h"
#include "global.h"
#include "symbol-table.h"

extern int yydebug;
extern int yyparse();
extern void println(int x);

int status = 0;

int main(int argc, char *argv[]) {
  bool dev = false;
  bool print_3addr = false;
  bool optimized = false;
  for (int i = 0; i < argc; i++) {
    if (strcmp("-Olocal", argv[i]) == 0) {
      enable_local_optimization();
      optimized = true;
    } else if (strcmp("-Oglobal", argv[i]) == 0) {
      enable_global_optimization();
      optimized = true;
    } else if (strcmp("-Odev", argv[i]) == 0) {
      dev = true;
    }
  }

  if (dev) {
    // In development mode, we read from a file
    freopen("../test/source.c", "r", stdin);
  }

  SymTabInit(Global);
  SymTabInit(Local);

  FILE* file_3addr;

  if (!print_3addr) {
    if (optimized) {
      freopen("../test/source_opt.s", "w", stdout);
      file_3addr = fopen("../test/3addr.c", "w");
      print_blocks_and_instructions(file_3addr);
    } else {
      freopen("../test/source.s", "w", stdout);
    }
    print_pre_defined_instructions();
  }

  if (yyparse() < 0) {
    printf("main: syntax error\n");
    status = 1;
  }

  if (!SymTabLookup("main", Global)) {
    fprintf(stderr, "No function called main found in the source code.\n");
    status = 1;
  }

  if (!print_3addr) {
    print_strings();
    fclose(file_3addr);
  }

  return status;
}
