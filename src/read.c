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

lisp_object_t read_object(FILE *);

hash_table_t make_hash_table(unsigned int (*hash_function)(char *), int (*comparator)(char *, char *), unsigned int size) {
  hash_table_t table = malloc(sizeof(struct hash_table_t));
  table->hash_function = hash_function;
  table->comparator = comparator;
  table->size = size;
  table->datum = malloc(size * sizeof(struct lisp_object_t));
  memset(table->datum, '\0', size);
  return table;
}

int is_separator(int c) {
  return EOF == c || isspace(c) || '(' == c || ')' == c || '"' == c;
}

lisp_object_t read_fixnum(char c, int sign, FILE *stream) {
  int number = c - '0';
  int digit;
  while (isdigit(digit = fgetc(stream))) {
    number = number * 10 + digit - '0';
  }
  ungetc(digit, stream);
  return make_fixnum(number * sign);
}

void read_comment(FILE *stream) {
  int c = fgetc(stream);
  while (c != '\n' && c != EOF)
    c = fgetc(stream);
}

lisp_object_t read_character(FILE *stream) {
  int c = fgetc(stream);
  switch (c) {
    case '\\': {
      int c = fgetc(stream);
      switch (c) {
        case 'n': return make_character('\n');
        case 'r': return make_character('\r');
        case 't': return make_character('\t');
        case 'f': return make_character('\f');
        case 'b': return make_character('\b');
        case 'v': return make_character('\v');
        case 'a': return make_character('\a');
        default :
          fprintf(stderr, "unexpected token '%c'\n", c);
          exit(1);
      }
    }
    case EOF:
      fprintf(stderr, "unexpected end of file\n");
      exit(1);
    default : return make_character(c);
  }
}

lisp_object_t read_string(FILE *stream) {
  static char buffer[BUFFER_SIZE];
  int i = 0;
  int c = fgetc(stream);
  while (i < BUFFER_SIZE - 2 && c != EOF && c != '"') {
    buffer[i++] = c;
    c = fgetc(stream);
  }
  char *str = malloc((i + 2) * sizeof(char));
  strncpy(str, buffer, i + 1);
  str[i + 1] = '\0';
  return make_string(str);
}

lisp_object_t read_pair(FILE *stream) {
  lisp_object_t object = read_object(stream);
  if (CLOSE_OBJECT == object->type)
    return make_empty_list();
  if (DOT_OBJECT == object->type) {
    lisp_object_t o1 = read_object(stream);
    lisp_object_t o2 = read_object(stream);
    if (CLOSE_OBJECT == o2->type)
      return o1;
    else {
      fprintf(stderr, "More than one objects after '.'");
      exit(1);
    }
  } else
    return make_pair(object, read_pair(stream));
}

lisp_object_t read_symbol(char init, FILE *stream) {
  static char buffer[BUFFER_SIZE];
  int i = 1;
  int c = fgetc(stream);
  buffer[0] = init;
  while (i < BUFFER_SIZE -2 && !is_separator(c)) {
    buffer[i++] = c;
    c = fgetc(stream);
  }
  ungetc(c, stream);
  char *name = malloc((i + 1) * sizeof(char));
  strncpy(name, buffer, i);
  name[i] = '\0';
  return find_or_create_symbol(name);
}

lisp_object_t read_object(FILE *stream) {
  int c = fgetc(stream);
  switch (c) {
    case EOF: return make_eof_object();
    case ' ':
    case '\t':
    case '\n':
    case '\r': return read_object(stream);
    case ';': read_comment(stream); return read_object(stream);
    case '0': case '1': case '2': case '3': case '4':
    case '5': case '6': case '7': case '8': case '9':
      return read_fixnum(c, 1, stream);
    case '-': {
      int c = fgetc(stream);
      if (isdigit(c)) {
        return read_fixnum(c, -1, stream);
      } else {
        ungetc(c, stream);
        return read_symbol('-', stream);
      }
    }
    case '#': {
      int c = fgetc(stream);
      switch (c) {
        case 't': return make_true();
        case 'f': return make_false();
        case '\\': return read_character(stream);
        default :
          fprintf(stderr, "unexpected token '%c'\n", c);
          exit(1);
      }
    }
    case '"': return read_string(stream);
    case '(': {
      lisp_object_t next = read_object(stream);
      if (CLOSE_OBJECT == next->type)
        return make_empty_list();
      else
        return make_pair(next, read_pair(stream));
    }
    case ')': return make_close_object();
    case '\'': return make_pair(find_or_create_symbol("quote"),
                                make_pair(read_object(stream), make_empty_list()));
    case '.': return make_dot_object();
    default : return read_symbol(c, stream);
  }
}
