/*
 * main.c
 *
 *
 *
 * Copyright (C) 2013-03-13 liutos <mat.liutos@gmail.com>
 */
#include <stdio.h>

#include "write.h"
#include "eval.h"
#include "read.h"
#include "object.h"

int main(int argc, char *argv[])
{
  symbol_table = make_hash_table(hash_symbol_name, symbol_name_comparator, 11);
  startup_environment = make_startup_environment();
  repl_environment = make_repl_environment();
  lisp_object_t in_port = make_file_in_port(stdin);
  lisp_object_t out_port = make_file_out_port(stdout);
  while (1) {
    fputs("> ", stdout);
    fflush(stdout);
    write_object(eval_object(read_object(in_port), repl_environment), out_port);
    putc('\n', stdout);
  }
  return 0;
}
