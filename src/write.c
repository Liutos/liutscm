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

void write_object(lisp_object_t object, lisp_object_t port) {
  FILE *stream = out_port_stream(port);
  switch (object->type) {
    case FIXNUM:
      fprintf(stream, "%d", object->values.fixnum.value);
      break;
    case BOOLEAN:
      fprintf(stream, "#%c", object->values.boolean.value ? 't': 'f');
      break;
    case CHARACTER: {
      char c = object->values.character.value;
      switch (c) {
        case '\n': fprintf(stream, "#\\\\n"); break;
        case '\r': fprintf(stream, "#\\\\r"); break;
        case '\t': fprintf(stream, "#\\\\t"); break;
        case '\f': fprintf(stream, "#\\\\f"); break;
        case '\b': fprintf(stream, "#\\\\b"); break;
        case '\v': fprintf(stream, "#\\\\v"); break;
        case '\a': fprintf(stream, "#\\\\a"); break;
        default :
          if (33 <= c && c <= 127)
            fprintf(stream, "#\\%c", c);
          else
            fprintf(stream, "#\\\\x%02d", c);
      }
    }
      break;
    case STRING:
      fprintf(stream, "\"%s\"", object->values.string.value);
      break;
    case EMPTY_LIST: fprintf(stream, "()"); break;
    case PAIR: {
      fprintf(stream, "(");
      write_object(pair_car(object), port);
      lisp_object_t x;
      for (x = pair_cdr(object); is_pair(x); x = pair_cdr(x)) {
        putchar(' ');
        write_object(pair_car(x), port);
      }
      if (!is_null(x)) {
        fprintf(stream, " . ");
        write_object(x, port);
      }
      fprintf(stream, ")");
    }
      break;
    case SYMBOL: fprintf(stream, "%s", object->values.symbol.name); break;
    case UNDEFINED: fprintf(stream, "#<undefined>"); break;
    case PRIMITIVE_PROC: fprintf(stream, "#<procedure %p>", object->values.primitive_proc.C_proc); break;
    case COMPOUND_PROC:
      fprintf(stream, "#<procedure ");
      write_object(compound_proc_parameters(object), port);
      fprintf(stream, " ");
      write_object(compound_proc_body(object), port);
      fprintf(stream, ">");
      break;
    case FILE_IN_PORT:
      fprintf(stream, "#<port :in %p>", object);
      break;
    case FILE_OUT_PORT:
      fprintf(stream, "#<port :out %p>", object);
      break;
    case EOF_OBJECT:
      fprintf(stream, "#<eof>");
      exit(1);
    case COMPILED_PROC:
      /* fprintf(stream, "#<compiled-procedure %p>", object); */
      write_object(compiled_proc_code(object), port);
      break;
    case VECTOR:
      fprintf(stream, "#(");
      for (int i = 0; i < vector_length(object); i++) {
        write_object(vector_data_at(object, i), port);
        if (i != vector_length(object) - 1)
          fprintf(stream, " ");
      }
      fprintf(stream, ")");
      break;
    default :
      fprintf(stderr, "cannot write unknown type\n");
      exit(1);
  }
}
