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

#define BUFFER_SIZE 100

hash_table_t symbol_table;

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
  ungetc(digit, stream);
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

lisp_object_t make_pair(lisp_object_t car, lisp_object_t cdr) {
  lisp_object_t pair = malloc(sizeof(struct lisp_object_t));
  pair->type = PAIR;
  pair->values.pair.car = car;
  pair->values.pair.cdr = cdr;
  return pair;
}

lisp_object_t make_symbol(char *name) {
  lisp_object_t symbol = malloc(sizeof(struct lisp_object_t));
  symbol->type = SYMBOL;
  symbol->values.symbol.name = name;
  return symbol;
}

hash_table_t make_hash_table(unsigned int (*hash_function)(char *), int (*comparator)(char *, char *), unsigned int size) {
  hash_table_t table = malloc(sizeof(struct hash_table_t));
  table->hash_function = hash_function;
  table->comparator = comparator;
  table->size = size;
  table->datum = malloc(size * sizeof(struct lisp_object_t));
  memset(table->datum, '\0', size);
  return table;
}

unsigned int hash_symbol_name(char *name) {
  unsigned int val;
  for (val = 0; *name != '\0'; name++)
    val = (val << 5) + *name;
  return val;
}

int symbol_name_comparator(char *n1, char *n2) {
  return strcmp(n1, n2);
}

unsigned int compute_index(char *key, hash_table_t table) {
  unsigned int (*hash_function)(char *);
  hash_function = table->hash_function;
  unsigned int size = table->size;
  return hash_function(key) % size;
}

lisp_object_t find_in_hash_table(char *target, hash_table_t table) {
  unsigned int index = compute_index(target, table);
  table_entry_t entry = table->datum[index];
  int (*comparator)(char *, char *);
  comparator = table->comparator;
  while (entry != NULL) {
    char *key = entry->key;
    if (0 == comparator(key, target))
      return entry->value;
    entry = entry->next;
  }
  return NULL;
}

table_entry_t make_entry(char *key, lisp_object_t value, table_entry_t next) {
  table_entry_t entry = malloc(sizeof(struct table_entry_t));
  entry->key = key;
  entry->value = value;
  entry->next = next;
  return entry;
}

void store_into_hash_table(char *key, lisp_object_t value, hash_table_t table) {
  unsigned int index = compute_index(key, table);
  table_entry_t entry = table->datum[index];
  table_entry_t new_entry = make_entry(key, value, entry);
  table->datum[index] = new_entry;
}

lisp_object_t find_symbol(char *name) {
  return find_in_hash_table(name, symbol_table);
}

void store_symbol(lisp_object_t symbol) {
  char *key = symbol_name(symbol);
  store_into_hash_table(key, symbol, symbol_table);
}

lisp_object_t find_or_create_symbol(char *name) {
  lisp_object_t symbol = find_symbol(name);
  if (NULL == symbol) {
    symbol = make_symbol(name);
    store_symbol(symbol);
    return symbol;
  } else
    return symbol;
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

lisp_object_t read_object(FILE *);

lisp_object_t read_pair(FILE *stream) {
  lisp_object_t object = read_object(stream);
  if (CLOSE_OBJECT == object->type)
    return make_empty_list();
  else
    return make_pair(object, read_pair(stream));
}

int is_separator(int c) {
  return EOF == c || ' ' == c || '(' == c || ')' == c || '"' == c;
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
  char *name = malloc((i + 2) * sizeof(char));
  strncpy(name, buffer, i + 1);
  name[i + 1] = '\0';
  /* return make_symbol(name); */
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
      else
        return make_pair(next, read_pair(stream));
    }
    case ')': return make_close_object();
    case '\'': return make_pair(find_or_create_symbol("quote"),
                                make_pair(read_object(stream), make_empty_list()));
    default : return read_symbol(c, stream);
  }
}
