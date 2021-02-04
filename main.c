#include "global.h"
#include "symbol-table.h"
#include "code_translation.h"

extern int yydebug;
extern int yyparse();
extern void println(int x);

int status = 0;

int main(void)
{
//  freopen("../../ExampleInputs/codegen.c", "r", stdin);
//  freopen("../../mips_code/codegen.s", "w", stdout);

  SymTabInit(Global);
  SymTabInit(Local);

  print_globals();
  if (yyparse() < 0) {
    printf("main: syntax error\n");
    status = 1;
  }

  if(!SymTabLookup("main", Global)) {
    fprintf(stderr, "No function called main found in the source code.\n");
    status = 1;
  }

  print_strings();
  
  return status;
}
