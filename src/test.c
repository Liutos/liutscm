/*
 * test.c
 *
 * Simple tests
 *
 * Copyright (C) 2013-03-14 liutos <mat.liutos@gmail.com>
 */
#include <stdio.h>
#include <string.h>
#include "write.h"
#include "eval.h"
#include "read.h"

extern lisp_object_t repl_environment;

int main(int argc, char *argv[])
{
  char *cases[] = {
    /* "123", */
    /* "-123", */
    /* "#t", */
    /* "#f", */
    /* "#\\a", */
    /* "#\\A", */
    /* "#\\\\n", */
    /* "#\\ ", */
    /* "\"Hello, world!\"", */
    /* "( )", */
    /* "'(1 . 2)", */
    /* "'(1 (2))", */
    /* "hello", */
    /* "'hello", */
    /* "(quote hello)", */
    /* "(if #t 1 2)", */
    /* "(if #f 1 2)", */
    /* "-", */
    /* "(+ 1 2)", */
    /* "(- 4 3)", */
    /* "(* 5 6)", */
    /* "(/ 8 7)", */
    /* "(= 1 2)", */
    /* "(= 1 1)", */
    /* "(> 3 4)", */
    /* "(% 10 3)", */
    /* "(eq 'hello 'hello)", */
    /* "(eq 1 1)", */
    /* "(char->code #\\a)", */
    /* "(code->char 98)", */
    /* "(char-at 5 \"Hello, world!\")", */
    /* "(car '(1 2))", */
    /* "(cdr '(1 2))", */
    /* "(cons 1 '(2))", */
    /* "(symbol-name 'hello)", */
    /* "(eq (string->symbol \"hello\") 'hello)", */
    /* "(type-of 'hello)", */
    /* "(type-of #\\a)", */
    /* "(define foo 1)", */
    /* "foo", */
    /* "(set! foo '(1 . 2))", */
    /* "foo", */
    /* "(set-car! foo 2)", */
    /* "foo", */
    /* "(set-cdr! foo 3)", */
    /* "foo", */
    /* "(lambda (x) (+ x 1))", */
    /* "(+ (+ 1 2) (+ 3 4))", */
    /* "((lambda (x) (+ x 1)) 1)", */
    /* "(define plus-one (lambda (n) (+ n 1)))", */
    /* "plus-one", */
    /* "(plus-one 1)", */
    /* "(begin (define a 1) (set! a (+ a 1)) a)", */
    /* "((lambda (x) (+ x 1)) 1)", */
    /* "(cond)", */
    /* "(cond (#f 1))", */
    /* "(cond ((eq 1 1) 2) (else 3))", */
    /* "(let ((x 1)) (+ x 1))", */
    /* "(let ((x 1) (y 2)) (set! x (+ y 1)) (* y y))", */
    /* "((lambda (x y) (set! x (+ x 1)) (* x y)) 1 2)", */
    /* "(and #t #t)", */
    /* "(and #t #f)", */
    /* "(and #f #t)", */
    /* "(and #f #f)", */
    /* "(and)", */
    /* "(and 1 2)", */
    /* "(and #f 3)", */
    /* "(or #t #t)", */
    /* "(or #t #f)", */
    /* "(or #f #t)", */
    /* "(or #f #f)", */
    /* "(or 1 #f)", */
    /* "(or 1 (/ 1 0))", */
    /* "(apply + 1 '(2))", */
    /* "(repl-environment)", */
    /* "(define a 1)", */
    /* "(repl-environment)", */
    /* "(eval '(+ 1 2) (repl-environment))", */
    "(define port (open-in \"/home/liutos/src/scheme/liutscm/README.md\"))",
    "(read-char port)",
    "(close-in port)",
  };
  symbol_table = make_hash_table(hash_symbol_name, symbol_name_comparator, 11);
  startup_environment = make_startup_environment();
  repl_environment = make_repl_environment();
  for (int i = 0; i < sizeof(cases) / sizeof(char *); i++) {
    FILE *stream = fmemopen(cases[i], strlen(cases[i]), "r");
    printf(">> %s\n=> ", cases[i]);
    write_object(eval_object(read_object(stream), repl_environment));
    putchar('\n');
    fclose(stream);
  }
  return 0;
}
