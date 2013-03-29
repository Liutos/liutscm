/*
 * types.h
 *
 * Definition of data types in Lisp
 *
 * Copyright (C) 2013-03-13 liutos <mat.liutos@gmail.com>
 */
#ifndef TYPES_H
#define TYPES_H

#include <stdio.h>

typedef struct lisp_object_t *sexp;
typedef sexp (*C_proc_t)(sexp);
typedef unsigned int (*hash_fn_t)(char *);
typedef int (*comp_fn_t)(char *, char *);

enum object_type {
  /* tagged pointer types */
  /* FIXNUM, */
  /* EOF_OBJECT, */
  /* BOOLEAN, */
  /* CHARACTER, */
  /* EMPTY_LIST, */
  /* CLOSE_OBJECT, */
  /* DOT_OBJECT, */
  /* UNDEFINED, */
  /* compound structure types */
  STRING,
  PAIR,
  SYMBOL,
  PRIMITIVE_PROC,
  COMPOUND_PROC,
  FILE_IN_PORT,
  FILE_OUT_PORT,
  COMPILED_PROC,
  VECTOR,
  RETURN_INFO,
  FLONUM,
  MACRO,
};

/* Lisp object */
typedef struct lisp_object_t {
  enum object_type type;
  int ref_count;
  sexp next, prev;
  union {
    struct {
      char *value;
    } string;
    struct {
      sexp car;
      sexp cdr;
    } pair;
    struct {
      char *name;
    } symbol;
    struct {
      C_proc_t C_proc;
      int is_side_effect;
      char *Lisp_name;
    } primitive_proc;
    struct {
      sexp parameters;
      sexp raw_body;
      sexp environment;
    } compound_proc;
    struct {
      FILE *stream;
      int line_num;
    } file_in_port;
    struct {
      FILE *stream;
    } file_out_port;
    struct {
      sexp args;
      sexp code;
      sexp env;
    } compiled_proc;
    struct {
      sexp *datum;
      unsigned int length;
    } vector;
    struct {
      sexp code;
      int pc;
      sexp env;
    } return_info;
    struct {
      float value;
    } flonum;
  } values;
} *lisp_object_t;

/* hash table */
typedef struct table_entry_t {
  char *key;
  lisp_object_t value;
  struct table_entry_t *next;
} *table_entry_t;

typedef struct hash_table_t {
  hash_fn_t hash_function;
  comp_fn_t comparator;
  table_entry_t *datum;
  unsigned int size;
} *hash_table_t;

#define yes 1
#define no 0

/* predicates and accessors */
#define EXTENDED_BITS 4
#define EXTENDED_MASK 0x0f
#define EXTENDED_TAG 0x0e
#define MAKE_SINGLETON_OBJECT(n)\
  ((lisp_object_t)((n << EXTENDED_BITS) | EXTENDED_TAG))
/* tagged pointer constant definitions */
#define false_object                            \
  MAKE_SINGLETON_OBJECT(0)
#define true_object                             \
  MAKE_SINGLETON_OBJECT(1)
#define empty_list_object                       \
  MAKE_SINGLETON_OBJECT(2)
#define eof_object                              \
  MAKE_SINGLETON_OBJECT(3)
#define undefined_object                        \
  MAKE_SINGLETON_OBJECT(4)
#define close_object                            \
  MAKE_SINGLETON_OBJECT(5)
#define dot_object                              \
  MAKE_SINGLETON_OBJECT(6)

#define is_of_tag(x, mask, tag) (tag == (((int)(x)) & mask))
/* BOOLEAN */
#define BOOL_MASK 0x0f
#define BOOL_TAG 0x0e
#define BOOL_BITS 4
#define is_true(x) (true_object == x)
#define is_false(x) (false_object == x)
#define is_bool(x) (is_true(x) || is_false(x))
#define bool_value(x) (((int)(x)) >> BOOL_BITS)
/* CLOSE_OBJECT */
#define is_close_object(x) (close_object == x)
/* DOT_OBJECT */
#define is_dot_object(x) (dot_object == x)
/* EMPTY_LIST */
#define EMPTY_LIST_MASK 0x0f
#define EMPTY_LIST_TAG 0x0e
#define EMPTY_LIST_BITS 4
#define is_null(x) (empty_list_object == x)
#define EOL empty_list_object
/* EOF */
#define EOF_BITS 4
#define EOF_TAG 0x0e
#define EOF_MASK 0x0f
#define is_eof(x) (eof_object == x)
/* UNDEFINED */
#define is_undefined(x) (undefined_object == x)

/* FIXNUM */
#define FIXNUM_BITS 2
#define FIXNUM_MASK 0x03
#define FIXNUM_TAG 0x01
#define is_fixnum(x) is_of_tag(x, FIXNUM_MASK, FIXNUM_TAG)
#define to_fixnum(x) ((lisp_object_t)((value << FIXNUM_BITS) | FIXNUM_TAG))
#define fixnum_value(x) (((int)(x)) >> FIXNUM_BITS)
/* CHARACTER */
#define CHAR_BITS 4
#define CHAR_MASK 0x0f
#define CHAR_TAG 0x06
#define is_char(x) is_of_tag(x, CHAR_MASK, CHAR_TAG)
#define to_char(x) ((lisp_object_t)((c << CHAR_BITS) | CHAR_TAG))
#define char_value(x) (((int)(x)) >> CHAR_BITS)

/* pointer on heap */
#define POINTER_MASK 0x03
#define POINTER_TAG 0x00
#define is_pointer(x) is_of_tag(x, POINTER_MASK, POINTER_TAG)
#define is_pointer_tag(x, tag)\
  (is_pointer(x) && tag == (x)->type)
/* STRING */
#define is_string(x) is_pointer_tag(x, STRING)
#define string_value(x) ((x)->values.string.value)
/* PAIR */
#define is_pair(x) is_pointer_tag(x, PAIR)
#define pair_car(x) ((x)->values.pair.car)
#define pair_cdr(x) ((x)->values.pair.cdr)
/* SYMBOL */
#define is_symbol(x) is_pointer_tag(x, SYMBOL)
#define symbol_name(x) ((x)->values.symbol.name)
/* FILE_IN_PORT */
#define is_in_port(x) is_pointer_tag(x, FILE_IN_PORT)
#define in_port_stream(x) ((x)->values.file_in_port.stream)
#define in_port_linum(x) ((x)->values.file_in_port.line_num)
/* FILE_OUT_PORT */
#define is_out_port(x) is_pointer_tag(x, FILE_OUT_PORT)
#define is_port(x) (is_in_port(x) || is_out_port(x))
#define out_port_stream(x) ((x)->values.file_out_port.stream)
/* VECTOR */
#define is_vector(x) is_pointer_tag(x, VECTOR)
#define vector_datum(x) ((x)->values.vector.datum)
#define vector_length(x) ((x)->values.vector.length)
#define vector_data_at(x, i) (vector_datum(x)[i])
/* FLONUM */
#define is_float(x) is_pointer_tag(x, FLONUM)
#define float_value(x) ((x)->values.flonum.value)
/* PRIMITIVE_PROC */
#define is_primitive(x) is_pointer_tag(x, PRIMITIVE_PROC)
#define primitive_C_proc(x) ((x)->values.primitive_proc.C_proc)
#define primitive_se(x) ((x)->values.primitive_proc.is_side_effect)
#define primitive_name(x) ((x)->values.primitive_proc.Lisp_name)
/* COMPOUND_PROC */
#define is_compound(x) is_pointer_tag(x, COMPOUND_PROC)
#define is_function(x) (is_primitive(x) || is_compound(x))
#define compound_proc_parameters(x) ((x)->values.compound_proc.parameters)
#define compound_proc_body(x) ((x)->values.compound_proc.raw_body)
#define compound_proc_environment(x) ((x)->values.compound_proc.environment)
/* MACRO */
#define is_macro(x) is_pointer_tag(x, MACRO)
#define macro_proc_pars(x) compound_proc_parameters(x)
#define macro_proc_body(x) compound_proc_body(x)
#define macro_proc_env(x) compound_proc_environment(x)
/* COMPILED_PROC */
#define is_compiled_proc(x) is_pointer_tag(x, COMPILED_PROC)
#define compiled_proc_args(x) ((x)->values.compiled_proc.args)
#define compiled_proc_code(x) ((x)->values.compiled_proc.code)
#define compiled_proc_env(x) ((x)->values.compiled_proc.env)
/* RETURN_INFO */
#define is_return_info(x) is_pointer_tag(x, RETURN_INFO)
#define return_code(x) ((x)->values.return_info.code)
#define return_pc(x) ((x)->values.return_info.pc)
#define return_env(x) ((x)->values.return_info.env)

/* utilities */
/* PAIR */
#define pair_cddr(x) pair_cdr(pair_cdr(x))
#define pair_cdddr(x) pair_cdr(pair_cddr(x))
#define pair_cadr(x) pair_car(pair_cdr(x))
#define pair_caddr(x) pair_car(pair_cddr(x))
#define pair_cadddr(x) pair_car(pair_cdddr(x))

#define first(x) pair_car(x)
#define second(x) pair_cadr(x)
#define third(x) pair_caddr(x)
#define fourth(x) pair_cadddr(x)

#define pair_caar(x) pair_car(pair_car(x))
#define pair_cdar(x) pair_cdr(pair_car(x))

#define LIST(...) make_list(__VA_ARGS__, NULL)
/* SYMBOL */
#define S(name) find_or_create_symbol(name)
#define is_label(x) is_symbol(x)

/* maintain reference count */
/* Assign and increase the ref_count */
#define ASIG(var, value) var = value
/* #define ASIG(var, value)                        \ */
/*   do {                                          \ */
/*     var = value;                                \ */
/*     inc_ref_count(value);                       \ */
/*   } while(0) */

#endif
