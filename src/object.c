/*
 * object.c
 *
 * Constructors and operations for the data types
 *
 * Copyright (C) 2013-03-17 liutos <mat.liutos@gmail.com>
 */
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "object.h"
#include "types.h"
#include "write.h"

#define HEAP_SIZE 1000

/* #define unlink(x)                               \ */
/*   do {                                          \ */
/*     if ((x)->prev != NULL)                      \ */
/*       (x)->prev->next = (x)->next;              \ */
/*     (x)->next = free_objects;                   \ */
/*     free_objects = (x);                         \ */
/*   } while (0) */

hash_table_t symbol_table;
/*
 * null_environment: Environment with no bindings
 * startup_environment: Environment with default bindings
 * repl_environment: Environment used by REPL
 */
sexp null_environment = EOL;
sexp repl_environment;
sexp startup_environment;

sexp scm_in_port;
sexp scm_out_port;

/*
 * objects_heap: A large consecutive memory for allocating Lisp objects
 * free_objects: A linked list contains all unused memory cell
 * used_objects: A linked list contains all allocated objects
 */
struct lisp_object_t *objects_heap;
struct lisp_object_t *free_objects;
struct lisp_object_t *used_objects;

/* memory management */
sexp alloc_object(enum object_type type) {
  if (NULL == free_objects) {
    fprintf(stderr, "Memory exhausted\n");
    exit(1);
  }
  sexp object = free_objects;
  free_objects = free_objects->next;

  object->next = used_objects;
  used_objects = object;

  object->type = type;
  return object;
}

void reclaim(sexp object) {
  /* unlink(object); */
  /* Unlink from `used_objects' */
  if (object->prev != NULL)
    object->prev->next = object->next;
  if (object->next != NULL)
    object->next->prev = object->prev;
  /* Concatenate into `free_objects' as header */
  object->prev = NULL;
  object->next = free_objects;
  free_objects = object;

  port_format(scm_out_port, "Releasing %*\n", object);
  if (is_pair(object)) {
    dec_ref_count(pair_car(object));
    dec_ref_count(pair_cdr(object));
  }
  if (is_in_port(object))
    fclose(in_port_stream(object));
  if (is_out_port(object))
    fclose(out_port_stream(object));
}

void dec_ref_count(sexp object) {
  if (!object || !is_pointer(object)) return;
  object->ref_count--;
  if (object->ref_count == 0)
    reclaim(object);
}

void inc_ref_count(lisp_object_t object) {
  if (object && is_pointer(object))
    object->ref_count++;
}

struct lisp_object_t *init_heap(void) {
  struct lisp_object_t *heap = malloc(HEAP_SIZE * sizeof(struct lisp_object_t));
  memset(heap, '\0', HEAP_SIZE * sizeof(struct lisp_object_t));
  for (int i = 0; i < HEAP_SIZE; i++)
    heap[i].next = &(heap[i + 1]);
  heap[HEAP_SIZE - 1].next = NULL;
  for (int i = HEAP_SIZE - 1; i > 0; i--)
    heap[i].prev = &(heap[i - 1]);
  heap[0].prev = NULL;
  free_objects = heap;
  used_objects = NULL;
  return heap;
}

/* constructors */
/* tagged pointer constants */
sexp make_close_object(void) { return close_object; }
sexp make_dot_object(void) { return dot_object; }
sexp make_empty_list(void) { return EOL; }
sexp make_eof_object(void) { return eof_object; }
sexp make_false(void) { return false_object; }
sexp make_true(void) { return true_object; }
sexp make_undefined(void) { return undefined_object; }

/* tagged pointer data types with rule */
sexp make_fixnum(int value) { return to_fixnum(value); }
sexp make_character(char c) { return to_char(c); }

/* tagged union data type */
sexp make_string(char *str) {
  sexp string = alloc_object(STRING);
  string->values.string.value = str;
  return string;
}

sexp make_pair(sexp car, sexp cdr) {
  lisp_object_t pair = alloc_object(PAIR);
  ASIG(pair_car(pair), car);
  ASIG(pair_cdr(pair), cdr);
  return pair;
}

sexp make_symbol(char *name) {
  sexp symbol = alloc_object(SYMBOL);
  symbol->values.symbol.name = name;
  return symbol;
}

sexp make_file_in_port(FILE *stream) {
  sexp port = alloc_object(FILE_IN_PORT);
  in_port_stream(port) = stream;
  in_port_linum(port) = 1;
  return port;
}

sexp make_file_out_port(FILE *stream) {
  sexp port = alloc_object(FILE_OUT_PORT);
  out_port_stream(port) = stream;
  return port;
}

sexp make_flonum(float value) {
  sexp object = alloc_object(FLONUM);
  float_value(object) = value;
  return object;
}

sexp make_primitive_proc(C_proc_t C_proc) {
  /* sexp proc = malloc(sizeof(struct sexp)); */
  sexp proc = alloc_object(PRIMITIVE_PROC);
  /* proc->type = ; */
  /* proc->values.primitive_proc.C_proc = C_proc; */
  primitive_C_proc(proc) = C_proc;
  return proc;
}

sexp make_lambda_procedure(sexp pars, sexp body, sexp env) {
  sexp proc = alloc_object(COMPOUND_PROC);
  ASIG(compound_proc_parameters(proc), pars);
  ASIG(compound_proc_body(proc), body);
  ASIG(compound_proc_environment(proc), env);
  return proc;
}

sexp make_compiled_proc(sexp args, sexp code, sexp env) {
  sexp proc = alloc_object(COMPILED_PROC);
  compiled_proc_args(proc) = args;
  compiled_proc_env(proc) = env;
  compiled_proc_code(proc) = code;
  return proc;
}

sexp make_vector(unsigned int length) {
  sexp vector = alloc_object(VECTOR);
  vector_length(vector) = length;
  vector_datum(vector) = malloc(length * sizeof(struct lisp_object_t));
  return vector;
}

sexp make_return_info(sexp code, int pc, sexp env) {
  /* sexp info = malloc(sizeof(struct sexp)); */
  sexp info = alloc_object(RETURN_INFO);
  /* info->type = ; */
  return_code(info) = code;
  return_pc(info) = pc;
  return_env(info) = env;
  return info;
}

sexp make_macro_procedure(sexp pars, sexp body, sexp env) {
  sexp macro = alloc_object(MACRO);
  ASIG(macro_proc_pars(macro), pars);
  ASIG(macro_proc_body(macro), body);
  ASIG(macro_proc_env(macro), env);
  return macro;
}

sexp make_environment(sexp bindings, sexp outer_env) {
  sexp env = alloc_object(ENVIRONMENT);
  environment_bindings(env) = bindings;
  environment_outer(env) = outer_env;
  return env;
}

sexp make_string_in_port(char *string) {
  sexp sp = alloc_object(STRING_IN_PORT);
  in_sp_string(sp) = string;
  in_sp_position(sp) = 0;
  return sp;
}

/* utilities */
/* PAIR */
sexp make_list_aux(va_list ap) {
  sexp car = va_arg(ap, sexp);
  if (NULL == car) {
    va_end(ap);
    return make_empty_list();
  } else
    return make_pair(car, make_list_aux(ap));
}

sexp make_list(sexp e, ...) {
  va_list ap;
  va_start(ap, e);
  return make_pair(e, make_list_aux(ap));
}

/* Set the `pair2' as the last cdr of proper list `pair1' */
sexp nconc_pair(sexp pair1, sexp pair2) {
  if (is_null(pair1)) return pair2;
  if (is_null(pair2)) return pair1;
  sexp head = pair1;
  while (!is_null(pair_cdr(pair1)))
    pair1 = pair_cdr(pair1);
  pair_cdr(pair1) = pair2;
  return head;
}

/* Returns the number of elements in proper list */
int pair_length(sexp pair) {
  int len = 0;
  for (; !is_null(pair); pair = pair_cdr(pair))
    len++;
  return len;
}

sexp pair_nthcdr(sexp pair, int n) {
tail_loop:
  if (0 == n || is_null(pair)) return pair;
  pair = pair_cdr(pair);
  n--;
  goto tail_loop;
}

/* (a b) + (c d) => ((a . c) (b . d)) */
/* (a b) + (c) => ((a . c) (b . ())) */
sexp merge_alist(sexp l1, sexp l2) {
  /* if (!is_pair(l1) || !is_pair(l2)) return EOL; */
  /* return make_pair(make_pair(pair_car(l1), pair_car(l2)), */
  /*                  merge_alist(pair_cdr(l1), pair_cdr(l2))); */
  if (!is_pair(l1)) return EOL;
  sexp val = is_pair(l2) ? pair_car(l2): EOL;
  sexp rest = is_pair(l2) ? pair_cdr(l2): EOL;
  return make_pair(make_pair(pair_car(l1), val),
                   merge_alist(pair_cdr(l1), rest));
}

/* Others */
int is_self_eval(sexp obj) {
  /* return !is_pointer(obj) || obj->type == FIXNUM || obj->type == CHARACTER; */
  return !is_pair(obj) && !is_symbol(obj);
}

/* hash table manipulation */
hash_table_t make_hash_table(hash_fn_t hash_function, comp_fn_t comparator, unsigned int size) {
  hash_table_t table = malloc(sizeof(struct hash_table_t));
  table->hash_function = hash_function;
  table->comparator = comparator;
  table->size = size;
  table->datum = malloc(size * sizeof(struct lisp_object_t));
  memset(table->datum, '\0', size);
  return table;
}

unsigned int compute_index(char *key, hash_table_t table) {
  hash_fn_t hash_function = table->hash_function;
  unsigned int size = table->size;
  return hash_function(key) % size;
}

table_entry_t make_entry(char *key, sexp value, table_entry_t next) {
  table_entry_t entry = malloc(sizeof(struct table_entry_t));
  entry->key = key;
  entry->value = value;
  entry->next = next;
  return entry;
}

void store_into_hash_table(char *key, sexp value, hash_table_t table) {
  unsigned int index = compute_index(key, table);
  table_entry_t entry = table->datum[index];
  table_entry_t new_entry = make_entry(key, value, entry);
  table->datum[index] = new_entry;
}

sexp find_in_hash_table(char *target, hash_table_t table) {
  unsigned int index = compute_index(target, table);
  table_entry_t entry = table->datum[index];
  comp_fn_t comparator = table->comparator;
  while (entry != NULL) {
    char *key = entry->key;
    if (0 == comparator(key, target))
      return entry->value;
    entry = entry->next;
  }
  return NULL;
}

/* symbol table manipulation */
unsigned int hash_symbol_name(char *name) {
  unsigned int val;
  for (val = 0; *name != '\0'; name++)
    val = (val << 5) + *name;
  return val;
}

int symbol_name_comparator(char *n1, char *n2) {
  return strcmp(n1, n2);
}

hash_table_t make_symbol_table(void) {
  return make_hash_table(hash_symbol_name, symbol_name_comparator, 11);
}

void store_symbol(lisp_object_t symbol) {
  char *key = symbol_name(symbol);
  store_into_hash_table(key, symbol, symbol_table);
}

sexp find_symbol(char *name) {
  return find_in_hash_table(name, symbol_table);
}

sexp find_or_create_symbol(char *name) {
  sexp symbol = find_symbol(name);
  if (NULL == symbol) {
    symbol = make_symbol(name);
    store_symbol(symbol);
    return symbol;
  } else
    return symbol;
}

/* Environment manipulation */
sexp extend_environment(sexp vars, sexp vals, sexp env) {
  /* return make_pair(make_pair(vars, vals), env); */
  sexp bindings = merge_alist(vars, vals);
  return make_environment(bindings, env);
}

sexp make_startup_environment(void) {
  if (startup_environment == NULL)
    startup_environment = extend_environment(EOL, EOL, null_environment);
  return startup_environment;
}

sexp make_repl_environment(void) {
  return extend_environment(EOL, EOL, startup_environment);
}

int is_empty_environment(sexp env) {
  return null_environment == env;
}

sexp search_binding(sexp var, sexp env) {
  while (!is_empty_environment(env)) {
    sexp bindings = environment_bindings(env);
    while (is_pair(bindings)) {
      sexp b = pair_car(bindings);
      if (pair_car(b) == var)
        return pair_cdr(b);
      bindings = pair_cdr(bindings);
    }
    /* sexp vars = environment_vars(env); */
    /* sexp vals = environment_vals(env); */
    /* while (is_pair(vars)) { */
    /*   if (pair_car(vars) == var) */
    /*     return pair_car(vals); */
    /*   vars = pair_cdr(vars); */
    /*   vals = pair_cdr(vals); */
    /* } */
    env = enclosing_environment(env);
  }
  return NULL;
}

int search_binding_index(sexp var, sexp env, int *x, int *y) {
  int i = 0;
  while (!is_empty_environment(env)) {
    sexp bindings = environment_bindings(env);
    for (int j = 0; is_pair(bindings); bindings = pair_cdr(bindings), j++) {
      sexp b = pair_car(bindings);
      if (pair_car(b) != var) continue;
      *x = i;
      *y = j;
      return yes;
    }
    /* sexp vars = environment_vars(env); */
    /* for (int j = 0; is_pair(vars); vars = pair_cdr(vars), j++) { */
    /*   if (pair_car(vars) == var) { */
    /*     *x = i; */
    /*     *y = j; */
    /*     return 1; */
    /*   } */
    /* } */
    env = enclosing_environment(env);
    i++;
  }
  return no;
}

/* Create a new binding if this `var' is not used yet */
void add_binding(sexp var, sexp val, sexp env) {
  sexp cell = search_binding(var, env);
  if (!cell) {
    /* sexp vars = environment_vars(environment); */
    /* sexp vals = environment_vals(environment); */
    /* vars = make_pair(var, vars); */
    /* vals = make_pair(val, vals); */
    /* ASIG(environment_vars(environment), vars); */
    /* ASIG(environment_vals(environment), vals); */
    sexp bindings = environment_bindings(env);
    bindings = make_pair(make_pair(var, val), bindings);
    environment_bindings(env) = bindings;
  }
}

/* Change an existing binding or create a new binding */
void set_binding(sexp var, sexp val, sexp environment) {
  sexp tmp = environment;
  while (!is_empty_environment(environment)) {
    sexp bindings = environment_bindings(environment);
    while (is_pair(bindings)) {
      if (pair_caar(bindings) == var) {
        pair_cdr(pair_car(bindings)) = val;
        break;
      }
      bindings = pair_cdr(bindings);
    }
    /* sexp vars = environment_vars(environment); */
    /* sexp vals = environment_vals(environment); */
    /* while (is_pair(vars)) { */
    /*   if (pair_car(vars) == var) { */
    /*     pair_car(vals) = val; */
    /*     break; */
    /*   } */
    /*   vars = pair_cdr(vars); */
    /*   vals = pair_cdr(vals); */
    /* } */
    environment = enclosing_environment(environment);
  }
  add_binding(var, val, tmp);
}

sexp get_variable_value(sexp var, sexp env) {
  sexp cell = search_binding(var, env);
  if (cell)
    return cell;
  else
    return make_undefined();
}
