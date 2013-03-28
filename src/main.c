/*
 * main.c
 *
 * REPL
 *
 * Copyright (C) 2013-03-13 liutos <mat.liutos@gmail.com>
 */
#include <stdio.h>
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
  objects_heap = init_heap();
  symbol_table = make_hash_table(hash_symbol_name, symbol_name_comparator, 11);
  startup_environment = make_startup_environment();
  repl_environment = make_repl_environment();
  /* lisp_object_t in_port = make_file_in_port(stdin); */
  /* lisp_object_t out_port = make_file_out_port(stdout); */
  DECL(in_port, make_file_in_port(stdin));
  DECL(out_port, make_file_out_port(stdout));
  load_init_file();
  while (1) {
    fputs("> ", stdout);
    fflush(stdout);
    write_object(eval_object(read_object(in_port), repl_environment), out_port);
    putc('\n', stdout);
  }
  return 0;
}
