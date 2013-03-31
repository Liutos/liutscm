/*
 * test-asm.c
 *
 * Test samples for the assembler
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
#include "init.h"

extern sexp extract_labels(sexp, int *);

int main(int argc, char *argv[])
{
  init_impl();
  char *cases[] = {
    /* "1", */
    /* "+", */
    /* "'hello", */
    /* "(+ 1 2)", */
    /* "(if #t 1 2)", */
    /* "(begin (set! a 1) a)", */
    /* "(begin \"doc\" (write \"Hello, world\") 2)", */
    "(lambda (x . y) (+ x 1))",
    /* "(+ 1 1)", */
    /* "((lambda (x . y) (cons x y)) 1 2 3 4)", */
    /* "(+ (* 1 2) (+ 3 (read)))", */
    /* "(eval (read) (repl-environment))", */
  };
  for (int i = 0; i < sizeof(cases) / sizeof(char *); i++) {
    FILE *fp = fmemopen(cases[i], strlen(cases[i]), "r");
    lisp_object_t in_port = make_file_in_port(fp);
    printf(">> %s\n", cases[i]);
    lisp_object_t value =
        compile_as_fn(read_object(in_port), repl_environment);
    /* value = */
    /*     run_compiled_code(value, repl_environment, EOL); */
    value = compiled_proc_code(value);
    port_format(scm_out_port, "-- %*\n", value);
    value = assemble_code(value);
    port_format(scm_out_port, "=> %*\n", value);
    fclose(fp);
  }
  return 0;
}
