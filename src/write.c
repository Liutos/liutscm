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

void write_char(char c, lisp_object_t port) {
  fputc(c, out_port_stream(port));
}

void write_string(char *str, lisp_object_t port) {
  fprintf(out_port_stream(port), "%s", str);
}

void write_object(lisp_object_t object, lisp_object_t port) {
  FILE *stream = out_port_stream(port);
  if (is_fixnum(object)) {
    fprintf(stream, "%d", fixnum_value(object));
    return;
  }
  if (is_char(object)) {
    char c = char_value(object);
    switch (c) {
      case '\n': write_string("#\\\\n", port); break;
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
    return;
  }
  if (is_bool(object)) {
    fprintf(stream, "#%c", bool_value(object) ? 't': 'f');
    return;
  }
  if (is_null(object)) {
    write_string("()", port);
    return;
  }
  if (is_eof(object)) {
    write_string("#<eof>", port);
    return;
  }
  if (is_undefined(object)) {
    write_string("#<undefined>", port);
    return;
  }
  switch (object->type) {
    case STRING:
      fprintf(stream, "\"%s\"", object->values.string.value);
      break;
    case PAIR: {
      write_string("(", port);
      write_object(pair_car(object), port);
      lisp_object_t x;
      for (x = pair_cdr(object); is_pair(x); x = pair_cdr(x)) {
        write_string(" ", port);
        write_object(pair_car(x), port);
      }
      if (!is_null(x)) {
        write_string(" . ", port);
        write_object(x, port);
      }
      write_string(")", port);
    }
      break;
    case SYMBOL: fprintf(stream, "%s", object->values.symbol.name); break;
    case PRIMITIVE_PROC: fprintf(stream, "#<procedure %p>", object->values.primitive_proc.C_proc); break;
    case COMPOUND_PROC:
      write_string("#<procedure ", port);
      write_object(compound_proc_parameters(object), port);
      write_string(" ", port);
      write_object(compound_proc_body(object), port);
      write_string(">", port);
      break;
    case FILE_IN_PORT:
      fprintf(stream, "#<port :in %p>", object);
      break;
    case FILE_OUT_PORT:
      fprintf(stream, "#<port :out %p>", object);
      break;
    case COMPILED_PROC: {
      fprintf(stream, "#<compiled-procedure %p>", object);
    }
      break;
    case VECTOR:
      write_string("#(", port);
      for (int i = 0; i < vector_length(object); i++) {
        write_object(vector_data_at(object, i), port);
        if (i != vector_length(object) - 1)
          write_string(" ", port);
      }
      write_string(")", port);
      break;
    default :
      fprintf(stderr, "cannot write unknown type\n");
      exit(1);
  }
}
