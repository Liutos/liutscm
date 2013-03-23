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

enum object_type {
  FIXNUM,
  EOF_OBJECT,
  BOOLEAN,
  CHARACTER,
  STRING,
  EMPTY_LIST,
  CLOSE_OBJECT,
  PAIR,
  SYMBOL,
  DOT_OBJECT,
  UNDEFINED,
  PRIMITIVE_PROC,
  COMPOUND_PROC,
  FILE_IN_PORT,
  FILE_OUT_PORT,
  COMPILED_PROC,
  VECTOR,
  RETURN_INFO,
  FLONUM,
};

typedef struct lisp_object_t *sexp;

typedef struct lisp_object_t {
  enum object_type type;
  int ref_count;
  struct lisp_object_t *next, *prev;
  union {
    struct {
      int value;
    } fixnum;
    struct {
      int value;
    } boolean;
    struct {
      char value;
    } character;
    struct {
      char *value;
    } string;
    struct {
      struct lisp_object_t *car;
      struct lisp_object_t *cdr;
    } pair;
    struct {
      char *name;
    } symbol;
    struct {
      struct lisp_object_t *(*C_proc)(struct lisp_object_t *);
    } primitive_proc;
    struct {
      struct lisp_object_t *parameters;
      struct lisp_object_t *raw_body;
      struct lisp_object_t *environment;
    } compound_proc;
    struct {
      FILE *stream;
      int line_num;
    } file_in_port;
    struct {
      FILE *stream;
    } file_out_port;
    struct {
      struct lisp_object_t *args;
      struct lisp_object_t *code;
      struct lisp_object_t *env;
    } compiled_proc;
    struct {
      struct lisp_object_t **datum;
      unsigned int length;
    } vector;
    struct {
      struct lisp_object_t *code;
      int pc;
      struct lisp_object_t *env;
    } return_info;
    struct {
      float value;
    } flonum;
  } values;
} *lisp_object_t;

typedef struct table_entry_t {
  char *key;
  lisp_object_t value;
  struct table_entry_t *next;
} *table_entry_t;

/*
 * hash_function: Compute the index from the key
 * comparator   : Compare the first and second arguments. Return zero when they're not equal and non-zero otherwise.
 */
typedef struct hash_table_t {
  unsigned int (*hash_function)(char *);
  int (*comparator)(char *, char *);
  table_entry_t *datum;
  unsigned int size;
} *hash_table_t;

/* ---ACCESSORS, PREDICATES--- */
/* ACCESSORS: $(type_name)_$(slot_name) */
/* PREDICATES: is_$(type_name) */
/* pointer on heap */
#define POINTER_MASK 0x03
#define POINTER_TAG 0x00
#define is_pointer(x) (POINTER_TAG == ((int)(x) & POINTER_MASK))
#define is_of_tag(x, mask, tag) (tag == (((int)(x)) & mask))
#define EXTENDED_TAG 0x0e
#define EXTENDED_MASK 0x0f
#define EXTENDED_BITS 4
#define MAKE_SINGLETON_OBJECT(n) ((lisp_object_t)((n << EXTENDED_BITS) | EXTENDED_TAG))
/* PAIR */
#define pair_car(x) ((x)->values.pair.car)
#define pair_cdr(x) ((x)->values.pair.cdr)
#define is_pair(x) (is_pointer(x) && (PAIR == (x)->type))
/* EMPTY_LIST */
#define EMPTY_LIST_MASK 0x0f
#define EMPTY_LIST_TAG 0x0e
#define EMPTY_LIST_BITS 4
#define empty_list_object MAKE_SINGLETON_OBJECT(2)
#define is_null(x) (empty_list_object == x)
#define EOL empty_list_object
/* SYMBOL */
#define symbol_name(x) ((x)->values.symbol.name)
#define is_symbol(x) (is_pointer(x) && (SYMBOL == (x)->type))
/* FIXNUM */
#define FIXNUM_BITS 2
#define fixnum_value(x) (((int)(x)) >> FIXNUM_BITS)
#define FIXNUM_MASK 0x03
#define FIXNUM_TAG 0x01
#define is_fixnum(x) is_of_tag(x, FIXNUM_MASK, FIXNUM_TAG)
/* PRIMITIVE_PROC */
#define primitive_C_proc(x) ((x)->values.primitive_proc.C_proc)
#define is_primitive(x) (is_pointer(x) && PRIMITIVE_PROC == (x)->type)
/* CHARACTER */
#define CHAR_TAG 0x06
#define CHAR_MASK 0x0f
#define CHAR_BITS 4
#define is_char(x) is_of_tag(x, CHAR_MASK, CHAR_TAG)
#define char_value(x) (((int)(x)) >> CHAR_BITS)
/* STRING */
#define string_value(x) ((x)->values.string.value)
#define is_string(x) (is_pointer(x) && STRING == (x)->type)
/* COMPOUND_PROC */
#define compound_proc_parameters(x) ((x)->values.compound_proc.parameters)
#define compound_proc_body(x) ((x)->values.compound_proc.raw_body)
#define compound_proc_environment(x) ((x)->values.compound_proc.environment)
#define is_compound(x) (is_pointer(x) && COMPOUND_PROC == (x)->type)
#define is_function(x) (is_primitive(x) || is_compound(x))
/* BOOLEAN */
#define BOOL_MASK 0x0f
#define BOOL_TAG 0x0e
#define BOOL_BITS 4
#define bool_value(x) (((int)(x)) >> BOOL_BITS)
#define is_bool(x) (is_true(x) || is_false(x))
#define true_object MAKE_SINGLETON_OBJECT(1)
#define false_object MAKE_SINGLETON_OBJECT(0)
#define is_true(x) (true_object == x)
#define is_false(x) (false_object == x)
/* UNDEFINED */
#define undefined_object MAKE_SINGLETON_OBJECT(4)
#define is_undefined(x) (undefined_object == x)
/* CLOSE_OBJECT */
#define close_object MAKE_SINGLETON_OBJECT(5)
#define is_close_object(x) (close_object == x)
/* DOT_OBJECT */
#define dot_object MAKE_SINGLETON_OBJECT(6)
#define is_dot_object(x) (dot_object == x)
/* FILE_IN_PORT */
#define in_port_stream(x) ((x)->values.file_in_port.stream)
#define in_port_linum(x) ((x)->values.file_in_port.line_num)
/* FILE_OUT_PORT */
#define out_port_stream(x) ((x)->values.file_out_port.stream)
/* EOF */
#define EOF_BITS 4
#define EOF_TAG 0x0e
#define EOF_MASK 0x0f
#define eof_object MAKE_SINGLETON_OBJECT(3)
#define is_eof(x) (eof_object == x)
/* COMPILED_PROC */
#define is_compiled_proc(x) (is_pointer(x) && COMPILED_PROC == (x)->type)
#define compiled_proc_args(x) ((x)->values.compiled_proc.args)
#define compiled_proc_code(x) ((x)->values.compiled_proc.code)
#define compiled_proc_env(x) ((x)->values.compiled_proc.env)
/* VECTOR */
#define is_vector(x) (is_pointer(x) && VECTOR == (x)->type)
#define vector_datum(x) ((x)->values.vector.datum)
#define vector_length(x) ((x)->values.vector.length)
#define vector_data_at(x, i) (vector_datum(x)[i])
/* RETURN_INFO */
#define is_return_info(x) (is_pointer(x) && RETURN_INFO == (x)->type)
#define return_code(x) ((x)->values.return_info.code)
#define return_pc(x) ((x)->values.return_info.pc)
#define return_env(x) ((x)->values.return_info.env)
/* FLONUM */
#define is_float(x) (is_pointer(x) && FLONUM == (x)->type)
#define float_value(x) ((x)->values.flonum.value)

/*
 * pair_cadr: second element
 * pair_caddr: third element
 * pair_cadddr: fourth element
 */
#define pair_cadr(x) pair_car(pair_cdr(x))
#define pair_cddr(x) pair_cdr(pair_cdr(x))
#define pair_caddr(x) pair_car(pair_cddr(x))
#define pair_cadddr(x) pair_car(pair_cdddr(x))
#define pair_caar(x) pair_car(pair_car(x))
#define pair_cdar(x) pair_cdr(pair_car(x))
#define pair_cdddr(x) pair_cdr(pair_cddr(x))

#endif
