/*
 * read.c
 *
 * Read and parse S-exp
 *
 * Copyright (C) 2013-03-13 liutos <mat.liutos@gmail.com>
 */
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#include "types.h"
#include "object.h"

#define BUFFER_SIZE 100

extern int nzero(char);
extern sexp read_char_proc(sexp);

sexp read_object(sexp);

int is_separator(int c) {
  return EOF == c || isspace(c) || '(' == c || ')' == c || '"' == c;
}

char port_read_char(sexp port) {
  FILE *stream = in_port_stream(port);
  return fgetc(stream);
}

void port_ungetc(char c, sexp port) {
  FILE *stream = in_port_stream(port);
  ungetc(c, stream);
}

/* object readers */
sexp read_float(int integer, sexp port) {
  int number = 0;
  int digit, i = 1;
  while (isdigit(digit = port_read_char(port))) {
    number = number * 10 + digit - '0';
    i = i * 10;
  }
  port_ungetc(digit, port);
  return make_flonum(integer + number * 1.0 / i);
}

sexp read_number(char c, int sign, sexp port) {
  int number = c - '0';
  int digit;
  while (isdigit(digit = port_read_char(port))) {
    number = number * 10 + digit - '0';
  }
  if (digit == '.')
    return read_float(number, port);
  port_ungetc(digit, port);
  return make_fixnum(number * sign);
}

void read_comment(sexp port) {
  int c = port_read_char(port);
  while (c != '\n' && c != EOF)
    c = port_read_char(port);
}

sexp read_character(sexp port) {
  int c = port_read_char(port);
  switch (c) {
    case '\\': {
      int c = port_read_char(port);
      switch (c) {
        case 'n': return make_character('\n');
        case 'r': return make_character('\r');
        case 't': return make_character('\t');
        case 'f': return make_character('\f');
        case 'b': return make_character('\b');
        case 'v': return make_character('\v');
        case 'a': return make_character('\a');
        default :
          fprintf(stderr, "unexpected token '%c' at line %d\n",
                  c, in_port_linum(port));
          exit(1);
      }
    }
    case EOF:
      fprintf(stderr, "unexpected end of file\n");
      exit(1);
    /* default : return make_character(c); */
    default :
      if (nzero(c) == 0)
        return make_character(c);
      else {
        port_ungetc(c, port);
        /* return read_char_proc(port); */
        return read_char(port);
      }
  }
}

sexp read_string(sexp port) {
  static char buffer[BUFFER_SIZE];
  int i = 0;
  int c = port_read_char(port);
  while (i < BUFFER_SIZE - 2 && c != EOF && c != '"') {
    buffer[i++] = c;
    c = port_read_char(port);
  }
  char *str = malloc((i + 2) * sizeof(char));
  strncpy(str, buffer, i + 1);
  str[i + 1] = '\0';
  /* return make_string(str); */
  return make_wstring(str);
}

sexp read_pair(sexp port) {
  sexp object = read_object(port);
  if (is_eof(object)) {
    fprintf(stderr, "Incomplete literal for list\n");
    exit(1);
  }
  if (is_close_object(object))
    return make_empty_list();
  if (is_dot_object(object)) {
    sexp o1 = read_object(port);
    sexp o2 = read_object(port);
    if (is_close_object(o2))
      return o1;
    else {
      fprintf(stderr, "More than one objects after '.' at line %d\n",
              in_port_linum(port));
      exit(1);
    }
  } else
    return make_pair(object, read_pair(port));
}

sexp read_symbol(char init, sexp port) {
  static char buffer[BUFFER_SIZE];
  int i = 1;
  int c = port_read_char(port);
  buffer[0] = init;
  while (i < BUFFER_SIZE -2 && !is_separator(c)) {
    buffer[i++] = c;
    c = port_read_char(port);
  }
  port_ungetc(c, port);
  char *name = malloc((i + 1) * sizeof(char));
  strncpy(name, buffer, i);
  name[i] = '\0';
  return find_or_create_symbol(name);
}

sexp read_vector(sexp port) {
  sexp list = read_pair(port);
  int length = pair_length(list);
  sexp vector = make_vector(length);
  for (int i = 0; !is_null(list); i++, list = pair_cdr(list)) {
    vector_data_at(vector, i) = pair_car(list);
  }
  return vector;
}

sexp read_object(sexp port) {
  int c = port_read_char(port);
  switch (c) {
    case EOF: return make_eof_object();
    case '\n': in_port_linum(port)++;
    case ' ':
    case '\t':
    case '\r': return read_object(port);
    case ';': read_comment(port); return read_object(port);
    case '0': case '1': case '2': case '3': case '4':
    case '5': case '6': case '7': case '8': case '9':
      return read_number(c, 1, port);
    case '-': {
      int c = port_read_char(port);
      if (isdigit(c)) {
        return read_number(c, -1, port);
      } else {
        port_ungetc(c, port);
        return read_symbol('-', port);
      }
    }
    case '#': {
      int c = port_read_char(port);
      switch (c) {
        case 't': return make_true();
        case 'f': return make_false();
        case '\\': return read_character(port);
        case '(': return read_vector(port);
        default :
          fprintf(stderr, "unexpected token '%c' at line %d\n",
                  c, in_port_linum(port));
          exit(1);
      }
    }
    case '"': return read_string(port);
    case '(': {
      sexp next = read_object(port);
      if (is_close_object(next))
        return EOL;
      else
        return make_pair(next, read_pair(port));
    }
    case ')': return make_close_object();
    case '\'': return make_pair(find_or_create_symbol("quote"),
                                make_pair(read_object(port), EOL));
    case '.': return make_dot_object();
    default : return read_symbol(c, port);
  }
}
