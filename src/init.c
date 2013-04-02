/*
 * init.c
 *
 * Preparation for starting the Lisp interpreter
 *
 * Copyright (C) 2013-03-28 liutos <mat.liutos@gmail.com>
 */
#include <stdio.h>

#include "object.h"
#include "read.h"
#include "eval.h"

extern void init_environment(sexp);

void load_init_file(char *path) {
  /* char *path = ".liut.scm"; */
  FILE *fp = fopen(path, "r");
  if (NULL == fp) {
    fprintf(stderr, "No initialization file '%s'\n", path);
    /* exit(1); */
    return;
  }
  lisp_object_t in_port = make_file_in_port(fp);
  lisp_object_t exp = read_object(in_port);
  while (!is_eof(exp)) {
    eval_object(exp, repl_environment);
    exp = read_object(in_port);
  }
}

void init_impl(void) {
  objects_heap = init_heap();
  symbol_table = make_symbol_table();
  /* Environment initialization */
  startup_environment = make_startup_environment();
  init_environment(startup_environment);
  global_env = make_global_env();
  repl_environment = make_repl_environment();
  /* input and output port */
  scm_in_port = make_file_in_port(stdin);
  scm_out_port = make_file_out_port(stdout);
  scm_err_port = make_file_out_port(stderr);
  load_init_file(".liut.scm");
}
