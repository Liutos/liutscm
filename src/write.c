/*
 * write.c
 *
 *
 *
 * Copyright (C) 2013-03-13 liutos <mat.liutos@gmail.com>
 */
#include "types.h"
#include <stdio.h>
#include <stdlib.h>

void write_object(lisp_object_t object) {
  switch (object->type) {
    case FIXNUM:
      printf("%d", object->values.fixnum.value);
      break;
    default :
      fprintf(stderr, "cannot write unknown type\n");
      exit(1);
  }
}
