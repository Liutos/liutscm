/*
 * write.c
 *
 * Printer for Lisp objects
 *
 * Copyright (C) 2013-03-13 liutos <mat.liutos@gmail.com>
 */
#include <assert.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

#include "object.h"
#include "types.h"

/* extern int is_label(sexp); */

void port_format(sexp, const char *, ...);

void write_char(char c, lisp_object_t port) {
  fputc(c, out_port_stream(port));
}

void write_string(char *str, lisp_object_t port) {
  fprintf(out_port_stream(port), "%s", str);
}

void write_fixnum(int n, sexp port) {
  fprintf(out_port_stream(port), "%d", n);
}

void write_flonum(float f, sexp port) {
  fprintf(out_port_stream(port), "%f", f);
}

void write_addr(void *ptr, sexp port) {
  fprintf(out_port_stream(port), "%p", ptr);
}

void write_object(sexp object, sexp port) {
  FILE *stream = out_port_stream(port);
  if (is_fixnum(object))
    write_fixnum(fixnum_value(object), port);
  else if (is_char(object)) {
    char c = char_value(object);
    switch (c) {
      case '\n': write_string("#\\\\n", port); break;
      case '\r': write_string("#\\\\r", port); break;
      case '\t': write_string("#\\\\t", port); break;
      case '\f': write_string("#\\\\f", port); break;
      case '\b': write_string("#\\\\b", port); break;
      case '\v': write_string("#\\\\v", port); break;
      case '\a': write_string("#\\\\a", port); break;
      default :
        if (33 <= c && c <= 127)
          fprintf(stream, "#\\%c", c);
        else
          fprintf(stream, "#\\\\x%02d", c);
    }
  } else if (is_bool(object))
    port_format(port, "#%c", make_character(bool_value(object) ? 't': 'f'));
  else if (is_null(object))
    write_string("()", port);
  else if (is_eof(object))
    write_string("#<eof>", port);
  else if (is_undefined(object))
    write_string("#<undefined>", port);
  if (!is_pointer(object)) return;
  /* objects on heap process starts */
  assert(is_pointer(object));
  switch (object->type) {
    case STRING:
      port_format(port, "\"%s\"", object);
      break;
    case PAIR: {
      write_char('(', port);
      write_object(pair_car(object), port);
      lisp_object_t x;
      for (x = pair_cdr(object); is_pair(x); x = pair_cdr(x)) {
        write_char(' ', port);
        write_object(pair_car(x), port);
      }
      if (!is_null(x)) {
        write_string(" . ", port);
        write_object(x, port);
      }
      write_char(')', port);
    }
      break;
    case SYMBOL: write_string(symbol_name(object), port); break;
    case PRIMITIVE_PROC:
      port_format(port, "#<procedure %p>",
                  /* primitive_name(object),  */primitive_C_proc(object));
      break;
    case COMPOUND_PROC:
      write_string("#<procedure ", port);
      write_object(compound_proc_parameters(object), port);
      write_char(' ', port);
      write_object(compound_proc_body(object), port);
      write_char('>', port);
      break;
    case MACRO:
      write_string("#<macro ", port);
      write_object(macro_proc_pars(object), port);
      write_char(' ', port);
      write_object(macro_proc_body(object), port);
      write_char('>', port);
      break;
    case FILE_IN_PORT:
      port_format(port, "#<port :in %p>", object);
      break;
    case FILE_OUT_PORT:
      port_format(port, "#<port :out %p>", object);
      break;
    case COMPILED_PROC:
      write_string("#<compiled-procedure ", port);
      /* write_object(compiled_proc_code(object), port); */
      sexp code = compiled_proc_code(object);
      write_char('\n', port);
      while (!is_null(code)) {
        sexp ins = pair_car(code);
        if (is_label(ins))
          port_format(port, "%s:", symbol_name(ins));
        else
          port_format(port, "\t%*\n", ins);
        code = pair_cdr(code);
      }
      write_char('>', port);
      break;
    case VECTOR:
      write_string("#(", port);
      for (int i = 0; i < vector_length(object); i++) {
        write_object(vector_data_at(object, i), port);
        if (i != vector_length(object) - 1)
          write_char(' ', port);
      }
      write_char(')', port);
      break;
    case RETURN_INFO:
      fprintf(stream, "#<return-info :code %p :pc %d :env %p>",
              return_code(object),
              return_pc(object),
              return_env(object));
      break;
    case FLONUM: write_flonum(float_value(object), port); break;
    case ENVIRONMENT:
      /* port_format(scm_out_port, "#<environment :bindings %* :outer_env %p>", */
      /*             environment_bindings(object), */
      /*             environment_outer(object)); */
      write_string("#<environment :bindings", port);
      for (sexp env = object; !is_empty_environment(env);
           env = environment_outer(env))
        port_format(scm_out_port, " %*", environment_bindings(env));
      port_format(scm_out_port, " %p>", object);
      break;
    default :
      fprintf(stderr, "cannot write unknown type %d\n", object->type);
      exit(1);
  }
}

void port_format(sexp port, const char *fmt, ...) {
  va_list ap;
  char c;
  va_start(ap, fmt);
  while ((c = *fmt++) != '\0') {
    if (c != '%')
      write_char(c, port);
    else {
      c = *fmt++;
      sexp obj = va_arg(ap, sexp);
      switch (c) {
        case 'c': write_char(char_value(obj), port); break;
        case 'd': write_fixnum(fixnum_value(obj), port); break;
        case 'f': write_flonum(float_value(obj), port); break;
        case 'p': write_addr(obj, port); break;
        case 's': write_string(string_value(obj), port); break;
        case '%': write_char('%', port); break;
        case '*': write_object(obj, port); break;
        default :
          fprintf(stderr, "Unexpected character %c\n", c);
          exit(1);
      }
    }
  }
}
