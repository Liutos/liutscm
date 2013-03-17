/*
 * write.c
 *
 * Printer for Lisp objects
 *
 * Copyright (C) 2013-03-13 liutos <mat.liutos@gmail.com>
 */
#include <stdio.h>
#include <stdlib.h>

#include "types.h"

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
    case PAIR: {
      putchar('(');
      write_object(pair_car(object));
      lisp_object_t x;
      for (x = pair_cdr(object); is_pair(x); x = pair_cdr(x)) {
        putchar(' ');
        write_object(pair_car(x));
      }
      if (!is_null(x)) {
        printf(" . ");
        write_object(x);
      }
      putchar(')');
    }
      break;
    case SYMBOL: printf("%s", object->values.symbol.name); break;
    case UNDEFINED: printf("#<undefined>"); break;
    case PRIMITIVE_PROC: printf("#<procedure %p>", object->values.primitive_proc.C_proc); break;
    case COMPOUND_PROC:
      printf("#<procedure ");
      write_object(compound_proc_parameters(object));
      putchar(' ');
      write_object(compound_proc_body(object));
      putchar('>');
      break;
    case FILE_IN_PORT:
      printf("#<port :in %p>", object);
      break;
    case FILE_OUT_PORT:
      printf("#<port :out %p>", object);
      break;
    default :
      fprintf(stderr, "cannot write unknown type\n");
      exit(1);
  }
}
