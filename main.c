#include "code_optimization.h"
#include "code_translation.h"
#include "global.h"
#include "symbol-table.h"

extern int yydebug;
extern int yyparse();
extern void println(int x);

int status = 0;

int main(int argc, char *argv[]) {
  freopen("../test/control_flow.c", "r", stdin);

  bool optimize = false;
  for (int i = 0; i < argc; i++) {
    if (strcmp("-Olocal", argv[i]) == 0) {
      enable_local_optimization();
      optimize = true;
    } else if (strcmp("-Oglobal", argv[i]) == 0) {
      optimize = true;
      enable_global_optimization();

    }
  }

//  if(optimize) {
//    freopen("../test/control_flow_opt.s", "w", stdout);
//  } else {
//    freopen("../test/control_flow.s", "w", stdout);
//  }

  SymTabInit(Global);
  SymTabInit(Local);

//  print_pre_defined_instructions();
  if (yyparse() < 0) {
    printf("main: syntax error\n");
    status = 1;
  }

  if (!SymTabLookup("main", Global)) {
    fprintf(stderr, "No function called main found in the source code.\n");
    status = 1;
  }

//  print_strings();

  return status;
}
