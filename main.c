#include "global.h"
#include "symbol-table.h"

extern int yydebug;
extern int yyparse();
extern void println(int x);

int status = 0;

int main(void)
{
  freopen("../ExampleInputs/codegen.c", "r", stdin);

  SymTabInit(Global);
  SymTabInit(Local);

  if (yyparse() < 0) {
    printf("main: syntax error\n");
    status = 1;
  }
  
  return status;
}
