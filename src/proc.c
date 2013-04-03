/*
 * proc.c
 *
 * Primitive procedures implementation
 *
 * Copyright (C) 2013-03-17 liutos <mat.liutos@gmail.com>
 */
#include <stdlib.h>
#include <string.h>

#include "eval.h"
#include "object.h"
#include "read.h"
#include "types.h"
#include "write.h"

/* #define ADD(Lisp_name, C_proc)                          \ */
/*   add_primitive_proc(Lisp_name, C_proc, environment) */
#define DEFPROC(Lisp_name, C_proc, is_se, code_name, arity)                   \
  {.type=PRIMITIVE_PROC, .values={.primitive_proc={(C_proc_t)C_proc, is_se, Lisp_name, code_name, to_fixnum(arity)}}}
#define PHEAD(C_proc) lisp_object_t C_proc(lisp_object_t args)

/* FIXNUM */
/* Binary plus */
sexp plus_proc(sexp args) {
  sexp n1 = pair_car(args);
  sexp n2 = pair_cadr(args);
  return make_fixnum(fixnum_value(n1) + fixnum_value(n2));
}

/* Binary minus */
sexp minus_proc(sexp args) {
  sexp n1 = pair_car(args);
  sexp n2 = pair_cadr(args);
  return make_fixnum(fixnum_value(n1) - fixnum_value(n2));
}

/* Binary multiply */
sexp multiply_proc(sexp args) {
  sexp n1 = pair_car(args);
  sexp n2 = pair_cadr(args);
  return make_fixnum(fixnum_value(n1) * fixnum_value(n2));
}

/* Binary divide */
sexp divide_proc(sexp args) {
  sexp n1 = pair_car(args);
  sexp n2 = pair_cadr(args);
  return make_fixnum(fixnum_value(n1) / fixnum_value(n2));
}

/* Binary numeric equal */
lisp_object_t numeric_equal_proc(lisp_object_t args) {
  lisp_object_t n1 = pair_car(args);
  lisp_object_t n2 = pair_cadr(args);
  return fixnum_value(n1) == fixnum_value(n2) ? make_true(): make_false();
}

lisp_object_t modulo_proc(lisp_object_t args) {
  lisp_object_t n1 = pair_car(args);
  lisp_object_t n2 = pair_cadr(args);
  return make_fixnum(fixnum_value(n1) % fixnum_value(n2));
}

lisp_object_t greater_than_proc(lisp_object_t args) {
  lisp_object_t n1 = pair_car(args);
  lisp_object_t n2 = pair_cadr(args);
  return fixnum_value(n1) > fixnum_value(n2) ? make_true(): make_false();
}

/* Bitwise and */
sexp bit_and_proc(sexp args) {
  sexp n1 = pair_car(args);
  sexp n2 = pair_cadr(args);
  return make_fixnum(fixnum_value(n1) & fixnum_value(n2));
}

/* Bitwise or */
sexp bit_or_proc(sexp args) {
  sexp n1 = pair_car(args);
  sexp n2 = pair_cadr(args);
  return make_fixnum(fixnum_value(n1) | fixnum_value(n2));
}

/* Bitwise not */
sexp bit_not_proc(sexp args) {
  sexp n = pair_car(args);
  return make_fixnum(~fixnum_value(n));
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

/* FILE_IN_PORT */
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

/* Read and parse an S-exp */
sexp read_proc(void) {
  return read_object(scm_in_port);
}
/* lisp_object_t read_proc(lisp_object_t args) { */
/*   lisp_object_t in_port = make_file_in_port(stdin); */
/*   return read_object(in_port); */
/* } */

/* FILE_OUT_PORT */
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

/* FUNCTION */
lisp_object_t apply_proc(lisp_object_t args) {
  fprintf(stderr, "Impossible - APPLY\n");
  exit(1);
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

/* STRING_IN_PORT */
sexp string2in_port_proc(sexp args) {
  sexp str = first(args);
  return make_string_in_port(string_value(str));
}

sexp read_sp_char_proc(sexp args) {
  sexp sp = first(args);
  char c = in_sp_char(sp);
  in_sp_position(sp)++;
  return make_character(c);
}

/* Others */
lisp_object_t eval_proc(lisp_object_t args) {
  /* fprintf(stderr, "Impossible - EVAL\n"); */
  /* exit(1); */
  sexp exp = pair_car(args);
  sexp env = pair_cadr(args);
  return eval_object(exp, env);
}

/* Are the two arguments identical? */
lisp_object_t is_identical_proc(lisp_object_t args) {
  lisp_object_t o1 = pair_car(args);
  lisp_object_t o2 = pair_cadr(args);
  return o1 == o2 ? make_true(): make_false();
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

/* Environment */
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

/* void add_primitive_proc(char *Lisp_name, lisp_object_t (*C_proc)(lisp_object_t), lisp_object_t environment) { */
/*   lisp_object_t proc = make_primitive_proc(C_proc); */
/*   lisp_object_t var = find_or_create_symbol(Lisp_name); */
/*   add_binding(var, proc, environment); */
/* } */

void add_primitive_proc(sexp proc, sexp env) {
  sexp var = S(primitive_name(proc));
  add_binding(var, proc, env);
}

struct lisp_object_t primitive_procs[] = {
  DEFPROC("+", plus_proc, no, "IADD", 2),
  DEFPROC("-", minus_proc, no, "ISUB", 2),
  DEFPROC("*", multiply_proc, no, "IMUL", 2),
  DEFPROC("quotient", divide_proc, no, "IDIV", 2),
  DEFPROC("remainder", modulo_proc, no, NULL, 2),
  DEFPROC("=", numeric_equal_proc, no, NULL, 2),
  DEFPROC(">", greater_than_proc, no, NULL, 2),
  DEFPROC("&", bit_and_proc, no, NULL, 2),
  DEFPROC("|", bit_or_proc, no, NULL, 2),
  DEFPROC("~", bit_not_proc, no, NULL, 1),
  DEFPROC("char->integer", char2code_proc, no, NULL, 1),
  DEFPROC("integer->char", code2char_proc, no, NULL, 1),
  DEFPROC("string-ref", char_at_proc, no, NULL, 2),
  DEFPROC("string-length", string_length_proc, no, NULL, 1),
  DEFPROC("string=?", string_equal_proc, no, NULL, 2),
  DEFPROC("car", pair_car_proc, no, "CAR", 1),
  DEFPROC("cdr", pair_cdr_proc, no, "CDR", 1),
  DEFPROC("cons", pair_cons_proc, no, NULL, 2),
  DEFPROC("set-car!", pair_set_car_proc, yes, NULL, 2),
  DEFPROC("set-cdr!", pair_set_cdr_proc, yes, NULL, 2),
  DEFPROC("symbol-name", symbol_name_proc, no, NULL, 1),
  DEFPROC("string->symbol", string2symbol_proc, no, NULL, 1),
  DEFPROC("apply", apply_proc, yes, NULL, -1),
  DEFPROC("open-in", open_in_proc, yes, NULL, 1),
  DEFPROC("read-char", read_char_proc, yes, NULL, 1),
  DEFPROC("close-in", close_in_proc, yes, NULL, 1),
  DEFPROC("read", read_proc, yes, NULL, 0),
  DEFPROC("open-out", open_out_proc, yes, NULL, 1),
  DEFPROC("write-char", write_char_proc, yes, NULL, 2),
  DEFPROC("close-out", close_out_proc, yes, NULL, 1),
  DEFPROC("write", write_proc, yes, NULL, 1),
  DEFPROC("vector-ref", vector_ref_proc, no, NULL, 2),
  DEFPROC("vector-set!", vector_set_proc, yes, NULL, 3),
  DEFPROC("+.", flonum_plus_proc, no, NULL, 2),
  DEFPROC("-.", flonum_minus_proc, no, NULL, 2),
  DEFPROC("*.", flonum_multiply_proc, no, NULL, 2),
  DEFPROC("/.", flonum_divide_proc, no, NULL, 2),
  DEFPROC("integer->float", integer_to_float_proc, no, NULL, 1),
  DEFPROC("repl-environment", get_repl_environment, no, NULL, 0),
  /* STRING_IN_PORT */
  DEFPROC("string->in-port", string2in_port_proc, no, NULL, 1),
  DEFPROC("read-string-in-port-char", read_sp_char_proc, yes, NULL, 1),
  /* Others */
  DEFPROC("type-of", type_of_proc, no, NULL, 1),
  DEFPROC("eq?", is_identical_proc, no, NULL, 2),
  DEFPROC("eval", eval_proc, yes, NULL, 2),
};

void init_environment(lisp_object_t environment) {
  int len = sizeof(primitive_procs) / sizeof(struct lisp_object_t);
  for (int i = 0; i < len; i++) {
    sexp pp = &primitive_procs[i];
    /* char *name = primitive_name(pp); */
    /* C_proc_t proc = primitive_C_proc(pp); */
    add_primitive_proc(pp, environment);
    /* ADD(name, proc); */
  }
  /* FIXNUM */
  /* ADD("+", plus_proc); */
  /* ADD("-", minus_proc); */
  /* ADD("*", multiply_proc); */
  /* ADD("quotient", divide_proc); */
  /* ADD("=", numeric_equal_proc); */
  /* ADD("remainder", modulo_proc); */
  /* ADD(">", greater_than_proc); */
  /* ADD("&", bit_and_proc); */
  /* ADD("|", bit_or_proc); */
  /* ADD("~", bit_not_proc); */

  /* CHAR */
  /* ADD("char->integer", char2code_proc); */
  /* ADD("integer->char", code2char_proc); */
  /* STRING */
  /* ADD("string-ref", char_at_proc); */
  /* ADD("string-length", string_length_proc); */
  /* ADD("string=?", string_equal_proc); */
  /* PAIR */
  /* ADD("car", pair_car_proc); */
  /* ADD("cdr", pair_cdr_proc); */
  /* ADD("cons", pair_cons_proc); */
  /* ADD("set-car!", pair_set_car_proc); */
  /* ADD("set-cdr!", pair_set_cdr_proc); */
  /* SYMBOL */
  /* ADD("symbol-name", symbol_name_proc); */
  /* ADD("string->symbol", string2symbol_proc); */
  /* FUNCTION */
  /* ADD("apply", apply_proc); */
  /* FILE_IN_PORT */
  /* ADD("open-in", open_in_proc); */
  /* ADD("read-char", read_char_proc); */
  /* ADD("close-in", close_in_proc); */
  /* ADD("read", read_proc); */
  /* FILE_OUT_PORT */
  /* ADD("open-out", open_out_proc); */
  /* ADD("write-char", write_char_proc); */
  /* ADD("close-out", close_out_proc); */
  /* ADD("write", write_proc); */
  /* VECTOR */
  /* ADD("vector-ref", vector_ref_proc); */
  /* ADD("vector-set!", vector_set_proc); */
  /* FLONUM */
  /* ADD("+.", flonum_plus_proc); */
  /* ADD("-.", flonum_minus_proc); */
  /* ADD("*.", flonum_multiply_proc); */
  /* ADD("/.", flonum_divide_proc); */
  /* ADD("integer->float", integer_to_float_proc); */
  /* Environment */
  /* ADD("repl-environment", get_repl_environment); */
  /* Others */
  /* ADD("type-of", type_of_proc); */
  /* ADD("eq?", is_identical_proc); */
  /* ADD("eval", eval_proc); */
}
