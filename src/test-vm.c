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
#include "compile.h"
#include "eval.h"
#include "vm.h"

int main(int argc, char *argv[])
{
  lisp_object_t out_port = make_file_out_port(stdout);
  char *cases[] = {
    /* "1", */
    /* "+", */
    /* "'hello", */
    /* "(if #t 1 2)", */
    /* "(set! car car)", */
    /* "(if (= x y) (f (g x)) (h x y (h 1 2)))", */
    /* "(begin \"doc\" (write \"Hello, world\") 2)", */
    /* "(begin (+ (* a x) (f x)) x)", */
    /* "(lambda (x) (+ x 1))", */
    /* "(+ 1 1)", */
    "((lambda (y z) (+ x y)) 1 2)",
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
    /* write_object(ugly_machine(compiled_code, repl_environment), out_port); */
    compiled_code = assemble_code(compiled_code);
    printf("-> ");
    write_object(compiled_code, out_port);
    printf("\n=> ");
    lisp_object_t stack = make_empty_list();
    write_object(run_compiled_code(compiled_code, repl_environment, stack), out_port);
    putchar('\n');
    fclose(fp);
  }
  return 0;
}
