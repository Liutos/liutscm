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
#include "init.h"

void load_init_file(void);

int main(int argc, char *argv[])
{
  char *cases[] = {
    "1.234",
    "(+. 1.1 1.2)",
    "(integer->float 123)",
    "(& 5 7)",
    "#t",
    "#f",
    "\"Hello, world!\"",
    "'(1 . 2)",
    "'hello",
    /* "(if #f 1 2)", */
    /* "-", */
    /* "(+ 1 2)", */
    /* "(eq? 'hello 'hello)", */
    /* "(eq? 1 1)", */
    /* "(eq? (string->symbol \"hello\") 'hello)", */
    /* "(type-of 'hello)", */
    /* "type-of", */
    /* "#\\a", */
    /* "(type-of #\\a)", */
    /* "(lambda (x) (+ x 1))", */
    /* "((lambda (x) (+ x 1)) 1)", */
    /* "(define plus-one (lambda (n) (+ n 1)))", */
    /* "plus-one", */
    /* "(plus-one 1)", */
    /* "(cond)", */
    /* "(cond ((eq? 1 1) 2) (else 3))", */
  };
  init_impl();
  /* load_init_file(); */
  DECL(out_port, make_file_out_port(stdout));
  for (int i = 0; i < sizeof(cases) / sizeof(char *); i++) {
    FILE *stream = fmemopen(cases[i], strlen(cases[i]), "r");
    sexp in_port = make_file_in_port(stream);
    printf(">> %s\n=> ", cases[i]);
    sexp input = read_object(in_port);
    sexp value = eval_object(input, repl_environment);
    write_object(value, out_port);
    putchar('\n');
    dec_ref_count(in_port);
    dec_ref_count(input);
    /* dec_ref_count(value); */
  }
  dec_ref_count(out_port);
  return 0;
}

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
