/*
 * init.c
 *
 * Preparation for starting the Lisp interpreter
 *
 * Copyright (C) 2013-03-28 liutos <mat.liutos@gmail.com>
 */
#include "object.h"

extern void init_environment(sexp);

void init_impl(void) {
  objects_heap = init_heap();
  symbol_table = make_symbol_table();
  startup_environment = make_startup_environment();
  init_environment(startup_environment);
  repl_environment = make_repl_environment();
}
