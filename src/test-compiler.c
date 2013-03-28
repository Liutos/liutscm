/*
 * test-compiler.c
 *
 * Some sample tests for compiler
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
#include "init.h"

int main(int argc, char *argv[])
{
  init_impl();
  /* objects_heap = init_heap(); */
  lisp_object_t out_port = make_file_out_port(stdout);
  char *cases[] = {
    /* "1", */
    "+",
    /* "'hello", */
    "(set! car car)",
    "(begin \"doc\" (write x) y)",
    "(lambda (x) (+ x 1))",
    "((lambda (x y) (+ x y)) 1 2)",
  };
  /* symbol_table = make_hash_table(hash_symbol_name, symbol_name_comparator, 11); */
  /* startup_environment = make_startup_environment(); */
  /* repl_environment = make_repl_environment(); */
  for (int i = 0; i < sizeof(cases) / sizeof(char *); i++) {
    FILE *fp = fmemopen(cases[i], strlen(cases[i]), "r");
    lisp_object_t in_port = make_file_in_port(fp);
    printf(">> %s\n=> ", cases[i]);
    write_object(compile_object(read_object(in_port), repl_environment), out_port);
    putchar('\n');
    fclose(fp);
  }
  return 0;
}
