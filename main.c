#include <time.h>

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
  bool optimized = false;
  bool timer = false;
  for (int i = 0; i < argc; i++) {
    if (strcmp("-Olocal", argv[i]) == 0) {
      enable_local_optimization();
      optimized = true;
    } else if (strcmp("-Oglobal", argv[i]) == 0) {
      enable_global_optimization();
      optimized = true;
    } else if (strcmp("-Odev", argv[i]) == 0) {
      dev = true;
    } else if (strcmp("-Otimer", argv[i]) == 0) {
      timer = true;
    }
  }

  FILE *file_timer;
  FILE *file_3addr;
  clock_t start, end;
  double cpu_time_used;
  int num_execs = 1;

  if(timer) {
    // Execute multiple times and save running time
    num_execs = 10;
    if (optimized) {
      file_timer = fopen("../test/timer_opt.txt", "w");
    } else {
      file_timer = fopen("../test/timer.txt", "w");
    }
  }

  for(int i = 0; i < num_execs; i++) {
    SymTabInit(Global);
    SymTabInit(Local);

    if (dev) {
      // In development mode, we read from a file
      freopen("../test/source.c", "r", stdin);
      if (optimized) {
        freopen("../test/source_opt.s", "w", stdout);
        file_3addr = fopen("../test/3addr.c", "w");
        print_blocks_and_instructions(file_3addr);
      } else {
        freopen("../test/source.s", "w", stdout);
      }
    }

    if(timer) {
      start = clock();
    }

    print_pre_defined_instructions();
    if (yyparse() < 0) {
      printf("main: syntax error\n");
      status = 1;
    }

    if (!SymTabLookup("main", Global)) {
      fprintf(stderr, "No function called main found in the source code.\n");
      status = 1;
    }
    print_strings();

    if (dev) {
      fclose(file_3addr);
    }

    if (timer) {
      end = clock();
      cpu_time_used = ((double)(end - start)) / CLOCKS_PER_SEC;
      fprintf(file_timer, "Running time : %f \n", cpu_time_used);
    }
  }

  if (timer) {
    fclose(file_timer);
  }

  return status;
}
