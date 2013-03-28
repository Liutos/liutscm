/*
 * init.c
 *
 * Preparation for starting the Lisp interpreter
 *
 * Copyright (C) 2013-03-28 liutos <mat.liutos@gmail.com>
 */
#include <stdio.h>

#include "object.h"

extern void init_environment(sexp);

void init_impl(void) {
  objects_heap = init_heap();
  symbol_table = make_symbol_table();
  startup_environment = make_startup_environment();
  init_environment(startup_environment);
  repl_environment = make_repl_environment();
  /* input and output port */
  scm_in_port = make_file_in_port(stdin);
  scm_out_port = make_file_out_port(stdout);
}
