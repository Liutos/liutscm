/*
 * test.c
 *
 * Simple tests
 *
 * Copyright (C) 2013-03-14 liutos <mat.liutos@gmail.com>
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "write.h"
#include "eval.h"
#include "read.h"
#include "object.h"

void load_init_file(void) {
  char *path = ".liut.scm";
  FILE *fp = fopen(path, "r");
  if (NULL == fp) {
    fprintf(stderr, "No initialization file '%s'\n", path);
    exit(1);
  }
  lisp_object_t in_port = make_file_in_port(fp);
  lisp_object_t exp = read_object(in_port);
  while (!is_eof(exp)) {
    eval_object(exp, repl_environment);
    exp = read_object(in_port);
  }
}

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
    /* "(quotient 8 7)", */
    /* "(= 1 2)", */
    /* "(= 1 1)", */
    /* "(> 3 4)", */
    /* "(remainder 10 3)", */
    /* "(eq? 'hello 'hello)", */
    /* "(eq? 1 1)", */
    /* "(char->code #\\a)", */
    /* "(code->char 98)", */
    /* "(char-at 5 \"Hello, world!\")", */
    /* "(car '(1 2))", */
    /* "(cdr '(1 2))", */
    /* "(cons 1 '(2))", */
    /* "(symbol-name 'hello)", */
    /* "(eq? (string->symbol \"hello\") 'hello)", */
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
    /* "(cond ((eq? 1 1) 2) (else 3))", */
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
    /* "(define in-port (open-in \"/home/liutos/src/scheme/liutscm/README.md\"))", */
    /* "(read-char in-port)", */
    /* "(close-in in-port)", */
    /* "(define out-port (open-out \"/home/liutos/building/tmp/abc.txt\"))", */
    /* "(write-char #\\a out-port)", */
    /* "(close-out out-port)", */
    /* "(write #\\a)", */
    /* "(write (string->symbol \"Hello, world\"))", */
    /* "(read)", */
    /* "(eq? '() '())", */
    /* "(< (+ 1 2) 2)", */
    /* "(string-length \"Hello, world!\")", */
    /* "(string=? \"abc\" \"abc\")", */
    "(char>? #\\a #\\b)",
    /* "(odd? 1)", */
    /* "(odd? 2)", */
    /* "(even? 2)", */
    /* "(even? 1)", */
  };
  symbol_table = make_hash_table(hash_symbol_name, symbol_name_comparator, 11);
  startup_environment = make_startup_environment();
  repl_environment = make_repl_environment();
  load_init_file();
  lisp_object_t out_port = make_file_out_port(stdout);
  for (int i = 0; i < sizeof(cases) / sizeof(char *); i++) {
    FILE *stream = fmemopen(cases[i], strlen(cases[i]), "r");
    lisp_object_t in_port = make_file_in_port(stream);
    printf(">> %s\n=> ", cases[i]);
    write_object(eval_object(read_object(in_port), repl_environment), out_port);
    putchar('\n');
    fclose(stream);
  }
  return 0;
}
