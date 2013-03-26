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

extern void init_environment(lisp_object_t);
extern void write_object(lisp_object_t, lisp_object_t);

/*
 * repl_environment: Environment used by REPL
 * null_environment: Environment with no bindings
 * startup_environment: Environment with default bindings
 */
lisp_object_t repl_environment;
lisp_object_t null_environment;
lisp_object_t startup_environment;

hash_table_t symbol_table;

/* Heap for allocating memory of Lisp objects */
struct lisp_object_t *objects_heap;
int free_index;
struct lisp_object_t *free_objects;
struct lisp_object_t *used_objects;

#define HEAP_SIZE 1000

struct lisp_object_t *init_heap(void) {
  struct lisp_object_t *heap = malloc(HEAP_SIZE * sizeof(struct lisp_object_t));
  for (int i = 0; i < HEAP_SIZE; i++) {
    heap[i].next = &(heap[i + 1]);
  }
  heap[HEAP_SIZE - 1].next = NULL;
  for (int i = HEAP_SIZE - 1; i >= 0; i--)
    heap[i].prev = &(heap[i - 1]);
  heap[0].prev = NULL;
  free_objects = heap;
  used_objects = NULL;
  free_index = 0;
  return heap;
}

lisp_object_t alloc_object(void) {
  /* lisp_object_t object = malloc(sizeof(struct lisp_object_t)); */
  /* object->ref_count = 0; */
  /* return object; */
  /* if (free_index >= HEAP_SIZE) { */
  if (NULL == free_objects) {
    fprintf(stderr, "Memory exhausted\n");
    exit(1);
  }
  /* lisp_object_t object = &objects_heap[free_index]; */
  /* free_index++; */
  lisp_object_t object = free_objects;
  free_objects = free_objects->next;
  /* Concatenate the new allocated object into list `used_objects' */
  object->next = used_objects;
  used_objects = object;
  return object;
}

lisp_object_t make_fixnum(int value) {
  return (lisp_object_t)((value << FIXNUM_BITS) | FIXNUM_TAG);
}

lisp_object_t make_flonum(float value) {
  lisp_object_t object = alloc_object();
  object->type = FLONUM;
  float_value(object) = value;
  return object;
}

lisp_object_t make_eof_object(void) {
  return eof_object;
}

lisp_object_t make_boolean(int value) {
  return (lisp_object_t)((value << BOOL_BITS) | BOOL_TAG);
}

lisp_object_t make_true(void) {
  return true_object;
}

lisp_object_t make_false(void) {
  return false_object;
}

lisp_object_t make_character(char c) {
  return (lisp_object_t)((c << CHAR_BITS) | CHAR_TAG);
}

lisp_object_t make_string(char *str) {
  /* lisp_object_t string = malloc(sizeof(struct lisp_object_t)); */
  lisp_object_t string = alloc_object();
  string->type = STRING;
  string->values.string.value = str;
  return string;
}

lisp_object_t make_empty_list(void) {
  return empty_list_object;
}

lisp_object_t make_close_object(void) {
  return close_object;
}

lisp_object_t make_dot_object(void) {
  return dot_object;
}

void inc_ref_count(lisp_object_t object) {
  if (object && is_pointer(object)) {
    object->ref_count++;
    /* printf("Increasing ref_count of "); */
    /* write_object(object, make_file_out_port(stdout)); */
    /* putchar('\n'); */
  }
}

#define unlink(x)                               \
  do {                                          \
    if ((x)->prev != NULL)                      \
      (x)->prev->next = (x)->next;              \
    (x)->next = free_objects;                   \
    free_objects = (x);                         \
  } while (0)

void dec_ref_count(lisp_object_t object) {
  if (object && is_pointer(object)) {
    object->ref_count--;
    /* printf("Decreasing ref_count of "); */
    /* write_object(object, make_file_out_port(stdout)); */
    /* putchar('\n'); */
    if (0 == object->ref_count) {
      printf("Releasing ");
      write_object(object, make_file_out_port(stdout));
      putchar('\n');
      /* Unlink this object within the `used_objects' list */
      unlink(object);
      /* if (object->prev != NULL) */
      /*   object->prev->next = object->next; */
      /* object->next = free_objects; */
      /* free_objects = object; */
    }
  }
}

/* PAIR */

lisp_object_t make_pair(lisp_object_t car, lisp_object_t cdr) {
  /* lisp_object_t pair = malloc(sizeof(struct lisp_object_t)); */
  lisp_object_t pair = alloc_object();
  pair->type = PAIR;
  /* Increase the reference count of the objects in car and cdr parts */
  inc_ref_count(car);
  inc_ref_count(cdr);
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
  /* lisp_object_t symbol = malloc(sizeof(struct lisp_object_t)); */
  lisp_object_t symbol = alloc_object();
  symbol->type = SYMBOL;
  symbol->values.symbol.name = name;
  return symbol;
}

lisp_object_t make_undefined(void) {
  return undefined_object;
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
  /* lisp_object_t vector = malloc(sizeof(struct lisp_object_t)); */
  lisp_object_t vector = alloc_object();
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
        /* return make_pair(pair_car(vars), pair_car(vals)); */
        return pair_car(vals);
      vars = pair_cdr(vars);
      vals = pair_cdr(vals);
    }
    env = enclosing_environment(env);
  }
  return NULL;
}

int search_binding_index(sexp var, sexp env, int *x, int *y) {
  int i = 0, j;
  while (!is_empty_environment(env)) {
    sexp vars = environment_vars(env);
    j = 0;
    while (is_pair(vars)) {
      if (pair_car(vars) == var) {
        *x = i;
        *y = j;
        /* return make_pair(make_fixnum(i), make_fixnum(j)); */
        return 1;
      }
      vars = pair_cdr(vars);
      j++;
    }
    env = enclosing_environment(env);
    i++;
  }
  return 0;
}

void add_binding(lisp_object_t var, lisp_object_t val, lisp_object_t environment) {
  lisp_object_t cell = search_binding(var, environment);
  if (!cell) {
    lisp_object_t vars = environment_vars(environment);
    lisp_object_t vals = environment_vals(environment);
    vars = make_pair(var, vars);
    vals = make_pair(val, vals);
    environment_vars(environment) = vars;
    environment_vals(environment) = vals;
    /* Increase the count because the environment references them */
    inc_ref_count(vars);
    inc_ref_count(vals);
  }
}

void set_binding(lisp_object_t var, lisp_object_t val, lisp_object_t environment) {
  lisp_object_t tmp = environment;
  while (!is_empty_environment(environment)) {
    lisp_object_t vars = environment_vars(environment);
    lisp_object_t vals = environment_vals(environment);
    while (is_pair(vars)) {
      if (pair_car(vars) == var) {
        dec_ref_count(pair_car(vals));  /* Decrease the reference count of previous variable value object */
        pair_car(vals) = val;
        inc_ref_count(val);             /* Increase the reference count of the new variable value object */
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
    /* return pair_cdr(cell); */
    return cell;
  else
    return make_undefined();
}

lisp_object_t make_file_in_port(FILE *stream) {
  /* lisp_object_t port = malloc(sizeof(struct lisp_object_t)); */
  lisp_object_t port = alloc_object();
  port->type = FILE_IN_PORT;
  in_port_stream(port) = stream;
  in_port_linum(port) = 1;
  return port;
}

lisp_object_t make_file_out_port(FILE *stream) {
  /* lisp_object_t port = malloc(sizeof(struct lisp_object_t)); */
  lisp_object_t port = alloc_object();
  port->type = FILE_OUT_PORT;
  out_port_stream(port) = stream;
  return port;
}

void free_file_out_port(lisp_object_t port) {
  /* free(port); */
  port->ref_count = 0;
  /* if (port->prev != NULL) */
  /*   port->prev->next = port->next; */
  /* port->next = free_objects; */
  /* free_objects = port; */
  unlink(port);
}
