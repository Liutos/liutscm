/*
 * test-vm.c
 *
 * Test samples for the virtual machine
 *
 * Copyright (C) 2013-03-18 liutos <mat.liutos@gmail.com>
 */
#include <stdio.h>
#include <string.h>

#include "types.h"
#include "object.h"
#include "read.h"
#include "write.h"
#include "compiler.h"
#include "eval.h"
#include "vm.h"

int main(int argc, char *argv[])
{
  objects_heap = init_heap();
  lisp_object_t out_port = make_file_out_port(stdout);
  char *cases[] = {
    "1",
    "+",
    "'hello",
    "(if #t 1 2)",
    "(set! car car)",
    "(begin \"doc\" (write \"Hello, world\") 2)",
    "(lambda (x) (+ x 1))",
    "(+ 1 1)",
    "((lambda (x y) (+ x y)) 1 2)",
  };
  symbol_table = make_hash_table(hash_symbol_name, symbol_name_comparator, 11);
  startup_environment = make_startup_environment();
  repl_environment = make_repl_environment();
  for (int i = 0; i < sizeof(cases) / sizeof(char *); i++) {
    FILE *fp = fmemopen(cases[i], strlen(cases[i]), "r");
    lisp_object_t in_port = make_file_in_port(fp);
    printf(">> %s\n", cases[i]);
    lisp_object_t compiled_code =
        compile_raw_object(read_object(in_port), repl_environment);
    printf("-- ");
    write_object(compiled_code, make_file_out_port(stdout));
    compiled_code = assemble_code(compiled_code);
    printf("\n-> ");
    write_object(compiled_code, out_port);
    printf("\n");
    lisp_object_t stack = make_empty_list();
    lisp_object_t value = run_compiled_code(compiled_code, repl_environment, stack);
    printf("=> ");
    write_object(value, out_port);
    putchar('\n');
    fclose(fp);
  }
  return 0;
}
