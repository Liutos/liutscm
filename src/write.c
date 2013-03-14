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
    case BOOLEAN:
      printf("#%c", object->values.boolean.value ? 't': 'f');
      break;
    case CHARACTER: {
      char c = object->values.character.value;
      switch (c) {
        case '\n': printf("#\\\\n"); break;
        case '\r': printf("#\\\\r"); break;
        case '\t': printf("#\\\\t"); break;
        case '\f': printf("#\\\\f"); break;
        case '\b': printf("#\\\\b"); break;
        case '\v': printf("#\\\\v"); break;
        case '\a': printf("#\\\\a"); break;
        default :
          if (33 <= c && c <= 127)
            printf("#\\%c", c);
          else
            printf("#\\\\x%02d", c);
      }
    }
      break;
    case STRING:
      printf("\"%s\"", object->values.string.value);
      break;
    case EMPTY_LIST: printf("()"); break;
    default :
      fprintf(stderr, "cannot write unknown type\n");
      exit(1);
  }
}
