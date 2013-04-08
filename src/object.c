/*
 * object.c
 *
 * Constructors and operations for the data types
 *
 * Copyright (C) 2013-03-17 liutos <mat.liutos@gmail.com>
 */
#include <assert.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "object.h"
#include "types.h"
#include "write.h"

#define HEAP_SIZE 1000

extern char port_read_char(sexp);

void mark(sexp);
int nzero(char);

int alloc_count;
int mark_count;
hash_table_t symbol_table;
/*
 * global_env: Environment could be accessed anywhere
 * null_environment: Environment with no bindings
 * startup_environment: Environment with default bindings
 * repl_environment: Environment used by REPL
 */
sexp global_env;
sexp null_environment = EOL;
sexp repl_environment;
sexp startup_environment;

sexp scm_err_port;
sexp scm_in_port;
sexp scm_out_port;
/*
 * objects_heap: A large consecutive memory for allocating Lisp objects
 * free_objects: A linked list contains all unused memory cell
 * used_objects: A linked list contains all allocated objects
 */
struct lisp_object_t *objects_heap;
struct lisp_object_t *free_objects;
sexp root;

/* Memory management */
void mark_compiled_proc(sexp proc) {
  mark(compiled_proc_args(proc));
  mark(compiled_proc_code(proc));
  mark(compiled_proc_env(proc));
}

void mark_compound_proc(sexp proc) {
  mark(compound_proc_parameters(proc));
  mark(compound_proc_body(proc));
  mark(compound_proc_environment(proc));
}

void mark_env(sexp env) {
  mark(environment_bindings(env));
  mark(environment_outer(env));
}

void mark_pair(sexp pair) {
  mark(pair_car(pair));
  mark(pair_cdr(pair));
}

void mark_return_info(sexp ri) {
  mark(return_code(ri));
  mark(return_env(ri));
}

/* Set an object's gc_mark as used. */
void mark(sexp obj) {
  if (!obj || !is_pointer(obj) || obj->gc_mark == yes) return;
  obj->gc_mark = yes;
  mark_count++;
  if (is_compiled_proc(obj))
    mark_compiled_proc(obj);
  else if (is_compound(obj))
    mark_compound_proc(obj);
  else if (is_environment(obj))
    mark_env(obj);
  else if (is_pair(obj))
    mark_pair(obj);
  else if (is_return_info(obj))
    mark_return_info(obj);
}

void scan_heap(void) {
  for (int i = 0; i < HEAP_SIZE; i++) {
    sexp obj = &objects_heap[i];
    /* Reclaim the object which is used but not marked. */
    if (obj->is_used == yes && obj->gc_mark == no) {
      port_format(scm_out_port, "Reclaiming %*\n", obj);
      obj->next = free_objects;
      free_objects = obj;
      obj->is_used = no;
      alloc_count--;
    } else
      obj->gc_mark = no;
  }
  printf("GC is Done!\n");
  scm_in_port->gc_mark = yes;
  scm_out_port->gc_mark = yes;
  scm_err_port->gc_mark = yes;
  mark_count = 0;
}

/* Mark and sweep */
void trigger_gc(void) {
  mark(root);
  printf("alloc_count: %d\n", alloc_count);
  printf("mark_count: %d\n", mark_count);
  /* exit(1); */
  scan_heap();
}

sexp alloc_object(enum object_type type) {
  if (free_objects == NULL)
    trigger_gc();
  if (NULL == free_objects) {
    fprintf(stderr, "Memory exhausted\n");
    exit(1);
  }
  sexp object = free_objects;
  free_objects = free_objects->next;

  alloc_count++;
  object->is_used = yes;
  object->type = type;
  return object;
}

struct lisp_object_t *init_heap(void) {
  struct lisp_object_t *heap =
      calloc(HEAP_SIZE, sizeof(struct lisp_object_t));
  /* memset(heap, '\0', HEAP_SIZE * sizeof(struct lisp_object_t)); */
  for (int i = 0; i < HEAP_SIZE; i++)
    heap[i].next = &(heap[i + 1]);
  heap[HEAP_SIZE - 1].next = NULL;
  free_objects = heap;
  return heap;
}

/* Constructors */
/* Tagged pointer constants */
sexp make_close_object(void) { return close_object; }
sexp make_dot_object(void) { return dot_object; }
sexp make_empty_list(void) { return EOL; }
sexp make_eof_object(void) { return eof_object; }
sexp make_false(void) { return false_object; }
sexp make_true(void) { return true_object; }
sexp make_undefined(void) { return undefined_object; }

/* Tagged pointer data types with rule */
sexp make_fixnum(int value) { return to_fixnum(value); }
sexp make_character(char c) { return to_char(c); }

/* Tagged union data types */
sexp make_string(char *str) {
  sexp string = alloc_object(STRING);
  string->values.string.value = str;
  return string;
}

sexp make_pair(sexp car, sexp cdr) {
  lisp_object_t pair = alloc_object(PAIR);
  pair_car(pair) = car;
  pair_cdr(pair) = cdr;
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
  sexp proc = alloc_object(PRIMITIVE_PROC);
  primitive_C_proc(proc) = C_proc;
  return proc;
}

sexp make_lambda_procedure(sexp pars, sexp body, sexp env) {
  sexp proc = alloc_object(COMPOUND_PROC);
  compound_proc_parameters(proc) = pars;
  compound_proc_body(proc) = body;
  compound_proc_environment(proc) = env;
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
  sexp info = alloc_object(RETURN_INFO);
  return_code(info) = code;
  return_pc(info) = pc;
  return_env(info) = env;
  return info;
}

sexp make_macro_procedure(sexp pars, sexp body, sexp env) {
  sexp macro = alloc_object(MACRO);
  macro_proc_pars(macro) = pars;
  macro_proc_body(macro) = body;
  macro_proc_env(macro) = env;
  return macro;
}

sexp make_environment(sexp bindings, sexp outer_env) {
  sexp env = alloc_object(ENVIRONMENT);
  environment_bindings(env) = bindings;
  environment_outer(env) = outer_env;
  return env;
}

sexp make_wchar(void) {
  sexp wc = alloc_object(WCHAR);
  wchar_value(wc)[WCHAR_LENGTH - 1] = '\0';
  return wc;
}

int get_mask(int n) {
  switch (n) {
    case 1: return 0x3f;
    case 2: return 0x1f;
    case 3: return 0x0f;
    case 4: return 0x07;
    case 5: return 0x03;
    case 6: return 0x01;
    default :
      port_format(scm_err_port, "Impossible - get_mask\n");
      exit(1);
  }
}

/* Computes the code point of an UTF-8 encoding character */
/* sexp bytes2code_point(char *bytes) { */
/*   int cp = 0; */
/*   int cnt = nzero(*bytes); */
/*   if (cnt == 0) return make_fixnum(*bytes); */
/*   cp = cp | get_mask(cnt); */
/*   cp = cp << (8 - (cnt + 1)); */
/*   bytes++; */
/*   for (int i = 0; i < cnt; i++) { */
/* cp = cp | */
/*   } */
/* } */

int utf8_strlen(char *str) {
  int ulen = 0;
  while (*str) {
    int n = nzero(*str);
    if (n == 0) {
      str++;
      /* ulen++; */
    } else {
      /* ulen += n; */
      str += n;
    }
    ulen++;
  }
  return ulen;
}

sexp make_wstring(char *bytes) {
  sexp init_wchar(char *);
  sexp ws = alloc_object(WSTRING);
  int len = utf8_strlen(bytes);
  wstring_length(ws) = len;
  wstring_value(ws) = calloc(len, sizeof(sexp));
  /* wstring_value(ws)[len - 1] = '\0'; */
  /* for (int i = 0; i < len; i++) */
  /*   wstring_value(ws)[i] = make_character(bytes[i]); */
  int i = 0;
  while (*bytes) {
    int n = nzero(*bytes);
    if (n == 0)
      wstring_value(ws)[i] = make_character(*bytes);
    else
      wstring_value(ws)[i] = init_wchar(bytes);
    i++;
    bytes += n == 0 ? 1: n;
  }
  return ws;
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
  if (!is_pair(l1)) return EOL;
  sexp val = is_pair(l2) ? pair_car(l2): EOL;
  sexp rest = is_pair(l2) ? pair_cdr(l2): EOL;
  return make_pair(make_pair(pair_car(l1), val),
                   merge_alist(pair_cdr(l1), rest));
}

/* PORT */
sexp read_byte(sexp port) {
  FILE *stream = in_port_stream(port);
  return make_fixnum(fgetc(stream));
}

/* Computes the number of prefix zero bits in a byte */
int nzero(char c) {
  char mask = 0x80;
  int count = 0;
  for (; (c & mask) != 0; c = c << 1)
    count++;
  return count;
}

sexp read_char(sexp port) {
  char c = port_read_char(port);
  if (nzero(c) == 0) return make_character(c);
  assert(nzero(c) < 6);
  sexp wc = make_wchar();
  wchar_value(wc)[0] = c;
  for (int i = 1; i < nzero(c); i++) {
    char ch = port_read_char(port);
    wchar_value(wc)[i] = ch;
  }
  return wc;
}

/* WCHAR */
sexp init_wchar(char *bytes) {
  sexp wc = make_wchar();
  int n = nzero(*bytes);
  for (int i = 0; i < n; i++)
    wchar_value(wc)[i] = bytes[i];
  return wc;
}

/* Others */
int is_self_eval(sexp obj) {
  return !is_pair(obj) && !is_symbol(obj);
}

/* Hash table manipulation */
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

/* Symbol table manipulation */
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
    symbol = make_symbol(strdup(name));
    store_symbol(symbol);
    return symbol;
  } else
    return symbol;
}

/* Environment manipulation */
sexp extend_environment(sexp vars, sexp vals, sexp env) {
  sexp bindings = merge_alist(vars, vals);
  return make_environment(bindings, env);
}

sexp make_global_env(void) {
  if (global_env == NULL)
    global_env = extend_environment(EOL, EOL, startup_environment);
  return global_env;
}

sexp make_startup_environment(void) {
  if (startup_environment == NULL)
    startup_environment = extend_environment(EOL, EOL, null_environment);
  return startup_environment;
}

sexp make_repl_environment(void) {
  return extend_environment(EOL, EOL, global_env);
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
    env = enclosing_environment(env);
    i++;
  }
  return no;
}

/* Create a new binding if this `var' is not used yet */
void add_binding(sexp var, sexp val, sexp env) {
  sexp cell = search_binding(var, env);
  if (!cell) {
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
