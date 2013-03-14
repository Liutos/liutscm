/*
 * read.c
 *
 * Read and parse S-exp
 *
 * Copyright (C) 2013-03-13 liutos <mat.liutos@gmail.com>
 */
#include "types.h"
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

lisp_object_t make_fixnum(int value) {
  lisp_object_t fixnum = malloc(sizeof(struct lisp_object_t));
  fixnum->type = FIXNUM;
  fixnum->values.fixnum.value = value;
  return fixnum;
}

lisp_object_t make_eof_object(void) {
  lisp_object_t eof_object = malloc(sizeof(struct lisp_object_t));
  eof_object->type = EOF_OBJECT;
  return eof_object;
}

lisp_object_t read_fixnum(char c, int sign, FILE *stream) {
  int number = c - '0';
  int digit;
  while (isdigit(digit = fgetc(stream))) {
    number = number * 10 + digit - '0';
  }
  return make_fixnum(number * sign);
}

void read_comment(FILE *stream) {
  int c = fgetc(stream);
  while (c != '\n' && c != EOF)
    c = fgetc(stream);
}

lisp_object_t make_boolean(int value) {
  lisp_object_t boolean = malloc(sizeof(struct lisp_object_t));
  boolean->type = BOOLEAN;
  boolean->values.boolean.value = value;
  return boolean;
}

lisp_object_t make_character(char c) {
  lisp_object_t character = malloc(sizeof(struct lisp_object_t));
  character->type = CHARACTER;
  character->values.character.value = c;
  return character;
}

lisp_object_t make_string(char *str) {
  lisp_object_t string = malloc(sizeof(struct lisp_object_t));
  string->type = STRING;
  string->values.string.value = str;
  return string;
}

lisp_object_t make_empty_list(void) {
  lisp_object_t empty_list = malloc(sizeof(struct lisp_object_t));
  empty_list->type = EMPTY_LIST;
  return empty_list;
}

lisp_object_t make_close_object(void) {
  lisp_object_t close_object = malloc(sizeof(struct lisp_object_t));
  close_object->type = CLOSE_OBJECT;
  return close_object;
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
#define BUFFER_SIZE 100
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
        fprintf(stderr, "unexpected token '%c'\n", c);
        exit(1);
      }
    }
    case '#': {
      int c = fgetc(stream);
      switch (c) {
        case 't': return make_boolean(1);
        case 'f': return make_boolean(0);
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
      else {
        fprintf(stderr, "Pair not supported yet\n");
        exit(1);
      }
    }
    case ')': return make_close_object();
    default :
      fprintf(stderr, "unexpected token '%c'\n", c);
      exit(1);
  }
}
