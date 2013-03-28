/*
 * proc.c
 *
 * Primitive procedures implementation
 *
 * Copyright (C) 2013-03-17 liutos <mat.liutos@gmail.com>
 */
#include <stdlib.h>
#include <string.h>

#include "types.h"
#include "object.h"

extern void write_object(lisp_object_t, lisp_object_t);
extern lisp_object_t read_object(lisp_object_t);

#define PHEAD(C_proc) lisp_object_t C_proc(lisp_object_t args)

/* ARITHMETIC */

/* Binary plus */
lisp_object_t plus_proc(lisp_object_t args) {
  lisp_object_t n1 = pair_car(args);
  lisp_object_t n2 = pair_cadr(args);
  return make_fixnum(fixnum_value(n1) + fixnum_value(n2));
}

/* Binary minus */
lisp_object_t minus_proc(lisp_object_t args) {
  lisp_object_t n1 = pair_car(args);
  lisp_object_t n2 = pair_cadr(args);
  return make_fixnum(fixnum_value(n1) - fixnum_value(n2));
}

/* Binary multiply */
lisp_object_t multiply_proc(lisp_object_t args) {
  lisp_object_t n1 = pair_car(args);
  lisp_object_t n2 = pair_cadr(args);
  return make_fixnum(fixnum_value(n1) * fixnum_value(n2));
}

/* Binary divide */
lisp_object_t divide_proc(lisp_object_t args) {
  lisp_object_t n1 = pair_car(args);
  lisp_object_t n2 = pair_cadr(args);
  return make_fixnum(fixnum_value(n1) / fixnum_value(n2));
}

/* Binary numeric equal */
lisp_object_t numeric_equal_proc(lisp_object_t args) {
  lisp_object_t n1 = pair_car(args);
  lisp_object_t n2 = pair_cadr(args);
  return fixnum_value(n1) == fixnum_value(n2) ? make_true(): make_false();
}

lisp_object_t mod_proc(lisp_object_t args) {
  lisp_object_t n1 = pair_car(args);
  lisp_object_t n2 = pair_cadr(args);
  return make_fixnum(fixnum_value(n1) % fixnum_value(n2));
}

lisp_object_t greater_than_proc(lisp_object_t args) {
  lisp_object_t n1 = pair_car(args);
  lisp_object_t n2 = pair_cadr(args);
  return fixnum_value(n1) > fixnum_value(n2) ? make_true(): make_false();
}

sexp bit_and_proc(sexp args) {
  sexp n1 = pair_car(args);
  sexp n2 = pair_cadr(args);
  return make_fixnum(fixnum_value(n1) & fixnum_value(n2));
}

sexp bit_or_proc(sexp args) {
  sexp n1 = pair_car(args);
  sexp n2 = pair_cadr(args);
  return make_fixnum(fixnum_value(n1) | fixnum_value(n2));
}

sexp bit_not_proc(sexp args) {
  sexp n = pair_car(args);
  return make_fixnum(~fixnum_value(n));
}

/* Are the two arguments identical? */
lisp_object_t is_identical_proc(lisp_object_t args) {
  lisp_object_t o1 = pair_car(args);
  lisp_object_t o2 = pair_cadr(args);
  return o1 == o2 ? make_true(): make_false();
}

/* FLONUM */

sexp flonum_plus_proc(sexp args) {
  sexp n1 = pair_car(args);
  sexp n2 = pair_cadr(args);
  return make_flonum(float_value(n1) + float_value(n2));
}

sexp flonum_minus_proc(sexp args) {
  sexp n1 = pair_car(args);
  sexp n2 = pair_cadr(args);
  return make_flonum(float_value(n1) - float_value(n2));
}

sexp flonum_multiply_proc(sexp args) {
  sexp n1 = pair_car(args);
  sexp n2 = pair_cadr(args);
  return make_flonum(float_value(n1) * float_value(n2));
}

sexp flonum_divide_proc(sexp args) {
  sexp n1 = pair_car(args);
  sexp n2 = pair_cadr(args);
  return make_flonum(float_value(n1) / float_value(n2));
}

sexp integer_to_float_proc(sexp args) {
  sexp n = pair_car(args);
  return make_flonum((float)(fixnum_value(n)));
}

/* CHAR */

/* Get the encode of a character */
lisp_object_t char2code_proc(lisp_object_t args) {
  lisp_object_t c = pair_car(args);
  return make_fixnum(char_value(c));
}

/* Return a character with given encode */
lisp_object_t code2char_proc(lisp_object_t args) {
  lisp_object_t n = pair_car(args);
  return make_character(fixnum_value(n));
}

/* STRING */

/* Get the specific character in a string */
lisp_object_t char_at_proc(lisp_object_t args) {
  lisp_object_t n = pair_cadr(args);
  lisp_object_t str = pair_car(args);
  return make_character(string_value(str)[fixnum_value(n)]);
}

lisp_object_t string_length_proc(lisp_object_t args) {
  lisp_object_t str = pair_car(args);
  unsigned int length = strlen(string_value(str));
  return make_fixnum(length);
}

PHEAD(string_equal_proc) {
  lisp_object_t str1 = pair_car(args);
  lisp_object_t str2 = pair_cadr(args);
  return strcmp(string_value(str1), string_value(str2)) ? make_false(): make_true();
}

/* PAIR */

lisp_object_t pair_car_proc(lisp_object_t args) {
  lisp_object_t list = pair_car(args);
  return pair_car(list);
}

lisp_object_t pair_cdr_proc(lisp_object_t args) {
  lisp_object_t list = pair_car(args);
  return pair_cdr(list);
}

lisp_object_t pair_set_car_proc(lisp_object_t args) {
  lisp_object_t pair = pair_car(args);
  lisp_object_t val = pair_cadr(args);
  /* dec_ref_count(pair_car(pair)); */
  pair_car(pair) = val;
  /* inc_ref_count(val); */
  return make_undefined();
}

lisp_object_t pair_set_cdr_proc(lisp_object_t args) {
  lisp_object_t pair = pair_car(args);
  lisp_object_t val = pair_cadr(args);
  /* dec_ref_count(pair_cdr(pair)); */
  pair_cdr(pair) = val;
  /* inc_ref_count(val); */
  return make_undefined();
}

/* Construct a pair by two arguments */
lisp_object_t pair_cons_proc(lisp_object_t args) {
  lisp_object_t o1 = pair_car(args);
  lisp_object_t o2 = pair_cadr(args);
  return make_pair(o1, o2);
}

/* SYMBOL */

lisp_object_t symbol_name_proc(lisp_object_t args) {
  lisp_object_t sym = pair_car(args);
  return make_string(symbol_name(sym));
}

/* Create a symbol looks the same as the string argument */
lisp_object_t string2symbol_proc(lisp_object_t args) {
  lisp_object_t str = pair_car(args);
  return find_or_create_symbol(string_value(str));
}

/* Return a symbol indicates the argument's type */
lisp_object_t type_of_proc(lisp_object_t args) {
  lisp_object_t o = pair_car(args);
  if (is_fixnum(o)) return S("fixnum");
  else if (is_bool(o)) return S("boolean");
  else if (is_char(o)) return S("character");
  else if (is_null(o)) return S("empty-list");
  else {
    switch (o->type) {
      case STRING: return S("string");
      case PAIR: return S("pair");
      case SYMBOL: return S("symbol");
      case PRIMITIVE_PROC: return S("function");
      case FILE_IN_PORT: return S("file-in-port");
      default :
        fprintf(stderr, "Unknown data type: %d\n", o->type);
        exit(1);
    }
  }
}

/* Return the environment used by the REPL */
lisp_object_t get_repl_environment(lisp_object_t args) {
  return repl_environment;
}

/* Return the environment with default bindings */
lisp_object_t get_startup_environment(lisp_object_t args) {
  return startup_environment;
}

/* Return a environment with nothing */
lisp_object_t get_null_environment(lisp_object_t args) {
  return null_environment;
}

/* VECTOR */

sexp vector_ref_proc(sexp args) {
  sexp vector = pair_car(args);
  sexp n = pair_cadr(args);
  return vector_data_at(vector, fixnum_value(n));
}

sexp vector_set_proc(sexp args) {
  sexp vector = pair_car(args);
  sexp n = pair_cadr(args);
  sexp value = pair_caddr(args);
  vector_data_at(vector, fixnum_value(n)) = value;
  return value;
}

/* File port support */

lisp_object_t open_in_proc(lisp_object_t args) {
  lisp_object_t path = pair_car(args);
  FILE *fp = fopen(string_value(path), "r");
  if (NULL == fp) {
    fprintf(stderr, "Can not open file '%s'\n", string_value(path));
    exit(1);
  }
  return make_file_in_port(fp);
}

lisp_object_t read_char_proc(lisp_object_t args) {
  lisp_object_t port = pair_car(args);
  return make_character(fgetc(in_port_stream(port)));
}

lisp_object_t close_in_proc(lisp_object_t args) {
  lisp_object_t port = pair_car(args);
  fclose(in_port_stream(port));
  return make_undefined();
}

lisp_object_t open_out_proc(lisp_object_t args) {
  lisp_object_t path = pair_car(args);
  FILE *fp = fopen(string_value(path), "w");
  if (NULL == fp) {
    fprintf(stderr, "Can not open file '%s'\n", string_value(path));
    exit(1);
  }
  return make_file_out_port(fp);
}

lisp_object_t write_char_proc(lisp_object_t args) {
  lisp_object_t ch = pair_car(args);
  lisp_object_t port = pair_cadr(args);
  fputc(char_value(ch), out_port_stream(port));
  return make_undefined();
}

lisp_object_t close_out_proc(lisp_object_t args) {
  lisp_object_t port = pair_car(args);
  fclose(out_port_stream(port));
  return make_undefined();
}

/* Write an object to standard output */
lisp_object_t write_proc(lisp_object_t args) {
  lisp_object_t object = pair_car(args);
  lisp_object_t out_port = make_file_out_port(stdout);
  write_object(object, out_port);
  return make_undefined();
}

/* Read and parse an S-exp */
lisp_object_t read_proc(lisp_object_t args) {
  lisp_object_t in_port = make_file_in_port(stdin);
  return read_object(in_port);
}

lisp_object_t apply_proc(lisp_object_t args) {
  fprintf(stderr, "Impossible - APPLY\n");
  exit(1);
}

lisp_object_t eval_proc(lisp_object_t args) {
  fprintf(stderr, "Impossible - EVAL\n");
  exit(1);
}

lisp_object_t make_primitive_proc(lisp_object_t (*C_proc)(lisp_object_t)) {
  lisp_object_t proc = malloc(sizeof(struct lisp_object_t));
  proc->type = PRIMITIVE_PROC;
  proc->values.primitive_proc.C_proc = C_proc;
  return proc;
}

void add_primitive_proc(char *Lisp_name, lisp_object_t (*C_proc)(lisp_object_t), lisp_object_t environment) {
  lisp_object_t proc = make_primitive_proc(C_proc);
  lisp_object_t var = find_or_create_symbol(Lisp_name);
  add_binding(var, proc, environment);
}

void init_environment(lisp_object_t environment) {
#define ADD(Lisp_name, C_proc) add_primitive_proc(Lisp_name, C_proc, environment);
  /* ARITHMETIC */
  add_primitive_proc("+", plus_proc, environment);
  add_primitive_proc("-", minus_proc, environment);
  add_primitive_proc("*", multiply_proc, environment);
  add_primitive_proc("quotient", divide_proc, environment);
  add_primitive_proc("=", numeric_equal_proc, environment);
  add_primitive_proc("remainder", mod_proc, environment);
  add_primitive_proc(">", greater_than_proc, environment);
  ADD("&", bit_and_proc);
  ADD("|", bit_or_proc);
  ADD("~", bit_not_proc);
  ADD("eq?", is_identical_proc);
  /* FLONUM */
  ADD("+.", flonum_plus_proc);
  ADD("-.", flonum_minus_proc);
  ADD("*.", flonum_multiply_proc);
  ADD("/.", flonum_divide_proc);
  ADD("integer->float", integer_to_float_proc);
  /* CHAR */
  add_primitive_proc("char->integer", char2code_proc, environment);
  add_primitive_proc("integer->char", code2char_proc, environment);
  /* STRING */
  add_primitive_proc("string-ref", char_at_proc, environment);
  ADD("string-length", string_length_proc);
  ADD("string=?", string_equal_proc);
  /* PAIR */
  ADD("car", pair_car_proc);
  ADD("cdr", pair_cdr_proc);
  ADD("cons", pair_cons_proc);
  ADD("set-car!", pair_set_car_proc);
  ADD("set-cdr!", pair_set_cdr_proc);
  /* SYMBOL */
  ADD("symbol-name", symbol_name_proc);
  ADD("string->symbol", string2symbol_proc);
  add_primitive_proc("type-of", type_of_proc, environment);
  add_primitive_proc("apply", apply_proc, environment);
  add_primitive_proc("eval", eval_proc, environment);
  add_primitive_proc("repl-environment", get_repl_environment, environment);
  /* IN PORT */
  add_primitive_proc("open-in", open_in_proc, environment);
  add_primitive_proc("read-char", read_char_proc, environment);
  add_primitive_proc("close-in", close_in_proc, environment);
  add_primitive_proc("read", read_proc, environment);
  /* OUT PORT */
  add_primitive_proc("open-out", open_out_proc, environment);
  add_primitive_proc("write-char", write_char_proc, environment);
  add_primitive_proc("close-out", close_out_proc, environment);
  add_primitive_proc("write", write_proc, environment);
  /* VECTOR */
  ADD("vector-ref", vector_ref_proc);
  ADD("vector-set!", vector_set_proc);
}
