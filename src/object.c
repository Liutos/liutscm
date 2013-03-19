/*
 * object.c
 *
 * Constructors for data types
 *
 * Copyright (C) 2013-03-17 liutos <mat.liutos@gmail.com>
 */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>

#include "types.h"
#include "object.h"

void init_environment(lisp_object_t);

/*
 * repl_environment: Environment used by REPL
 * null_environment: Environment with no bindings
 * startup_environment: Environment with default bindings
 */
lisp_object_t repl_environment;
lisp_object_t null_environment;
lisp_object_t startup_environment;

hash_table_t symbol_table;

lisp_object_t make_fixnum(int value) {
  /* lisp_object_t fixnum = malloc(sizeof(struct lisp_object_t)); */
  /* fixnum->type = FIXNUM; */
  /* fixnum->values.fixnum.value = value; */
  /* return fixnum; */
  return (lisp_object_t)((value << FIXNUM_BITS) | FIXNUM_TAG);
}

lisp_object_t make_eof_object(void) {
  lisp_object_t eof_object = malloc(sizeof(struct lisp_object_t));
  eof_object->type = EOF_OBJECT;
  return eof_object;
}

lisp_object_t make_boolean(int value) {
  /* lisp_object_t boolean = malloc(sizeof(struct lisp_object_t)); */
  /* boolean->type = BOOLEAN; */
  /* boolean->values.boolean.value = value; */
  /* return boolean; */
  return (lisp_object_t)((value << BOOL_BITS) | BOOL_TAG);
}

lisp_object_t make_true(void) {
  static lisp_object_t true_object = NULL;
  if (true_object)
    return true_object;
  else {
    true_object = make_boolean(1);
    return true_object;
  }
}

lisp_object_t make_false(void) {
  static lisp_object_t false_object = NULL;
  if (false_object)
    return false_object;
  else {
    false_object = make_boolean(0);
    return false_object;
  }
}

lisp_object_t make_character(char c) {
  /* lisp_object_t character = malloc(sizeof(struct lisp_object_t)); */
  /* character->type = CHARACTER; */
  /* character->values.character.value = c; */
  /* return character; */
  return (lisp_object_t)((c << CHAR_BITS) | CHAR_TAG);
}

lisp_object_t make_string(char *str) {
  lisp_object_t string = malloc(sizeof(struct lisp_object_t));
  string->type = STRING;
  string->values.string.value = str;
  return string;
}

lisp_object_t make_empty_list(void) {
  static lisp_object_t empty_list = NULL;
  if (NULL == empty_list) {
    empty_list = malloc(sizeof(struct lisp_object_t));
    empty_list->type = EMPTY_LIST;
  }
  return empty_list;
}

lisp_object_t make_close_object(void) {
  lisp_object_t close_object = malloc(sizeof(struct lisp_object_t));
  close_object->type = CLOSE_OBJECT;
  return close_object;
}

lisp_object_t make_dot_object(void) {
  lisp_object_t dot_object = malloc(sizeof(struct lisp_object_t));
  dot_object->type = DOT_OBJECT;
  return dot_object;
}

/* PAIR */

lisp_object_t make_pair(lisp_object_t car, lisp_object_t cdr) {
  lisp_object_t pair = malloc(sizeof(struct lisp_object_t));
  pair->type = PAIR;
  pair->values.pair.car = car;
  pair->values.pair.cdr = cdr;
  return pair;
}

int pair_length(lisp_object_t pair) {
  if (is_null(pair))
    return 0;
  else
    return 1 + pair_length(pair_cdr(pair));
}

lisp_object_t make_list_aux(va_list ap) {
  lisp_object_t car = va_arg(ap, lisp_object_t);
  if (NULL == car) {
    va_end(ap);
    return make_empty_list();
  } else
    return make_pair(car, make_list_aux(ap));
}

lisp_object_t make_list(lisp_object_t e, ...) {
  va_list ap;
  va_start(ap, e);
  return make_pair(e, make_list_aux(ap));
}

lisp_object_t pair_nthcdr(lisp_object_t pair, int n) {
  if (0 == n || is_null(pair))
    return pair;
  else
    return pair_nthcdr(pair_cdr(pair), n - 1);
}

lisp_object_t make_symbol(char *name) {
  lisp_object_t symbol = malloc(sizeof(struct lisp_object_t));
  symbol->type = SYMBOL;
  symbol->values.symbol.name = name;
  return symbol;
}

lisp_object_t make_undefined(void) {
  lisp_object_t undefined = malloc(sizeof(struct lisp_object_t));
  undefined->type = UNDEFINED;
  return undefined;
}

/* VECTOR */

unsigned int va_list_length(va_list ap) {
  if (NULL == va_arg(ap, lisp_object_t)) {
    va_end(ap);
    return 0;
  } else
    return 1 + va_list_length(ap);
}

lisp_object_t make_vector(unsigned int length/* , ... */) {
  lisp_object_t vector = malloc(sizeof(struct lisp_object_t));
  vector->type = VECTOR;
  vector_length(vector) = length;
  vector_datum(vector) = malloc(length * sizeof(struct lisp_object_t));
  return vector;
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

lisp_object_t make_null_environment(void) {
  null_environment = make_empty_list();
  return null_environment;
}

lisp_object_t make_startup_environment(void) {
  lisp_object_t vars = make_empty_list();
  lisp_object_t vals = make_empty_list();
  startup_environment = make_pair(make_pair(vars, vals), make_null_environment());
  init_environment(startup_environment);
  return startup_environment;
}

lisp_object_t extend_environment(lisp_object_t vars, lisp_object_t vals, lisp_object_t environment) {
  return make_pair(make_pair(vars, vals), environment);
}

lisp_object_t make_repl_environment(void) {
  return extend_environment(make_empty_list(), make_empty_list(), startup_environment);
}

int is_empty_environment(lisp_object_t env) {
  return is_null(env);
}

lisp_object_t search_binding(lisp_object_t var, lisp_object_t env) {
  while (!is_empty_environment(env)) {
    lisp_object_t vars = environment_vars(env);
    lisp_object_t vals = environment_vals(env);
    while (is_pair(vars)) {
      if (pair_car(vars) == var)
        return make_pair(pair_car(vars), pair_car(vals)); /* pair as binding */
      vars = pair_cdr(vars);
      vals = pair_cdr(vals);
    }
    env = enclosing_environment(env);
  }
  return NULL;
}

lisp_object_t search_binding_index(lisp_object_t var, lisp_object_t env) {
  int i = 0, j;
  while (!is_empty_environment(env)) {
    lisp_object_t vars = environment_vars(env);
    j = 0;
    while (is_pair(vars)) {
      if (pair_car(vars) == var)
        return make_pair(make_fixnum(i), make_fixnum(j));
      vars = pair_cdr(vars);
      j++;
    }
    env = enclosing_environment(env);
    i++;
  }
  return NULL;
}

void add_binding(lisp_object_t var, lisp_object_t val, lisp_object_t environment) {
  lisp_object_t cell = search_binding(var, environment);
  if (!cell) {
    lisp_object_t vars = environment_vars(environment);
    lisp_object_t vals = environment_vals(environment);
    environment_vars(environment) = make_pair(var, vars);
    environment_vals(environment) = make_pair(val, vals);
  }
}

void set_binding(lisp_object_t var, lisp_object_t val, lisp_object_t environment) {
  lisp_object_t tmp = environment;
  while (!is_empty_environment(environment)) {
    lisp_object_t vars = environment_vars(environment);
    lisp_object_t vals = environment_vals(environment);
    while (is_pair(vars)) {
      if (pair_car(vars) == var) {
        pair_car(vals) = val;
        break;
      }
      vars = pair_cdr(vars);
      vals = pair_cdr(vals);
    }
    environment = enclosing_environment(environment);
  }
  add_binding(var, val, tmp);
}

lisp_object_t get_variable_value(lisp_object_t var, lisp_object_t environment) {
  lisp_object_t cell = search_binding(var, environment);
  if (cell)
    return pair_cdr(cell);
  else
    return make_undefined();
}

lisp_object_t make_file_in_port(FILE *stream) {
  lisp_object_t port = malloc(sizeof(struct lisp_object_t));
  port->type = FILE_IN_PORT;
  /* port->values.file_in_port.stream = stream; */
  in_port_stream(port) = stream;
  in_port_linum(port) = 1;
  return port;
}

lisp_object_t make_file_out_port(FILE *stream) {
  lisp_object_t port = malloc(sizeof(struct lisp_object_t));
  port->type = FILE_OUT_PORT;
  /* port->values.file_out_port.stream = stream; */
  out_port_stream(port) = stream;
  return port;
}
