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
  char *cases[] = {
    /* "1", */
    /* "+", */
    /* "'hello", */
    /* "(if #f 1 2)", */
    /* "(if 1 1 2)", */
    /* "(if (+ 1 1) 1 1)", */
    /* "(lambda (x . y) (cons x y))", */
    /* "(define (last1 l) (if (null? (cdr l)) (car l) (last1 (cdr l))))", */
    /* "(car '(1 2))", */
    /* "(begin (define (even? n) (if (= n 0) #t (odd? (- n 1)))) (define (odd? n) (if (= n 0) #f (even? (- n 1)))) (odd? 1))", */
    "(begin (define (last1 l) (if (null? (cdr l)) (car l) (last1 (cdr l)))) (last1 '(1 2 3)))",
    /* "((lambda (x y) (+ x y)) 1 2)", */
    /* "(f (g x))", */
    /* "(begin (if p (f x) (* x x)) z)", */
    /* "(+ 1 2)", */
    /* "(begin (+ (* 1 2) (read)))", */
    /* "((lambda (x . y) (cons x y)) 1 2 3 4)", */
    /* "(eval (read) (repl-environment))", */
  };
  for (int i = 0; i < sizeof(cases) / sizeof(char *); i++) {
    FILE *fp = fmemopen(cases[i], strlen(cases[i]), "r");
    lisp_object_t in_port = make_file_in_port(fp);
    printf(">> %s\n=> ", cases[i]);
    sexp input = read_object(in_port);
    sexp code = compile_as_fn(input, repl_environment);
    write_object(code, scm_out_port);
    putchar('\n');
    fclose(fp);
  }
  return 0;
}
