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
    /* "(lambda () 1)", */
    /* "+", */
    /* "'hello", */
    /* "(if #f 1 2)", */
    /* "(if #t 1 2)", */
    /* "(if 1 'a 'b)", */
    /* "(if (+ 1 1) 1 1)", */

    /* "(f (g x))", */
    "(f (g (h x) (h y)))",
    /* "(define (last1 l) (if (null? (cdr l)) (car l) (last1 (cdr l))))", */
    /* "(begin \"doc\" (write x) y)", */
    /* "(lambda () (if (null? (car l)) (f (+ (* a x) b)) (g (/ x 2))))", */
    /* "((lambda (x y) (+ x y)) 1 2)", */
  };
  /* symbol_table = make_hash_table(hash_symbol_name, symbol_name_comparator, 11); */
  /* startup_environment = make_startup_environment(); */
  /* repl_environment = make_repl_environment(); */
  for (int i = 0; i < sizeof(cases) / sizeof(char *); i++) {
    FILE *fp = fmemopen(cases[i], strlen(cases[i]), "r");
    lisp_object_t in_port = make_file_in_port(fp);
    printf(">> %s\n=> ", cases[i]);
    sexp input = read_object(in_port);
    sexp code = compile_as_fn(input, repl_environment);
    write_object(code, out_port);
    putchar('\n');
    fclose(fp);
  }
  return 0;
}
