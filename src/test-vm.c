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
#include "init.h"

extern sexp extract_labels(sexp, int *);

int main(int argc, char *argv[])
{
  init_impl();
  char *cases[] = {
    /* "1", */
    /* "12.3", */
    /* "123.45", */
    /* "1234.567", */
    /* "12345.6789", */
    /* ";;\n987654.3210", */
    /* "+i", */
    /* "'hello", */
    /* "#\\汉", */
    /* "(if #t 1 2)", */
    /* "(set! a 1)", */
    /* "(begin (set! a 1) a)", */
    /* "(begin \"doc\" (write \"Hello, world\") 2)", */
    /* "(lambda (x) (+i x 1))", */
    /* "(+i 1 1)", */
    /* "(eq? \"abc\" \"abc\")", */
    /* "(eq? #\\a #\\a)", */
    /* "((lambda (x . y) (cons x y)) 1 2 3 4)", */
    /* "(eq? 'hello 'hello)", */
    /* "(cdr '(1 2))", */
    /* "(string-ref \"汉\" 0)", */
    /* "(string-length \"汉字\")", */
    "(string-set! \"汉字\" 1 #\\语)",
    /* "(eval '(cdr '(1 2 3)) (repl-environment))", */
    /* "(cons 1 2)", */
    /* "#(1 2 3)", */
    /* "(+i 1 2)", */
    /* "(-i 1 2)", */
    /* "(*i 1 2)", */
    /* "(/i 1 2)", */
  };
  for (int i = 0; i < sizeof(cases) / sizeof(char *); i++) {
    FILE *fp = fmemopen(cases[i], strlen(cases[i]), "r");
    lisp_object_t in_port = make_file_in_port(fp);
    printf(">> %s\n", cases[i]);
    lisp_object_t compiled_code =
        compile_as_fn(read_object(in_port), repl_environment);
    /* port_format(scm_out_port, "-- %*\n", compiled_code); */
    lisp_object_t value =
        run_compiled_code(compiled_code, repl_environment, vm_stack);
    port_format(scm_out_port, "=> %*\n", value);
    fclose(fp);
  }
  return 0;
}
