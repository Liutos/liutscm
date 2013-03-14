/*
 * main.c
 *
 *
 *
 * Copyright (C) 2013-03-13 liutos <mat.liutos@gmail.com>
 */
#include "write.h"
#include "eval.h"
#include "read.h"
#include <stdio.h>

int main(int argc, char *argv[])
{
  while (1) {
    fputs("> ", stdout);
    fflush(stdout);
    write_object(eval_object(read_object(stdin)));
    putc('\n', stdout);
  }
  return 0;
}
