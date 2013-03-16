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
    "(if #t 1 2)",
    "(if #f 1 2)",
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
    "(begin (define a 1) (set! a (+ a 1)) a)",
    "((lambda (x) (+ x 1)) 1)",
  };
  symbol_table = make_hash_table(hash_symbol_name, symbol_name_comparator, 11);
  lisp_object_t startup_environment = make_startup_environment();
  init_environment(startup_environment);
  for (int i = 0; i < sizeof(cases) / sizeof(char *); i++) {
    FILE *stream = fmemopen(cases[i], strlen(cases[i]), "r");
    printf(">> %s\n=> ", cases[i]);
    write_object(eval_object(read_object(stream), startup_environment));
    putchar('\n');
    fclose(stream);
  }
  return 0;
}
