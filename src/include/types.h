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
};

typedef struct lisp_object_t {
  enum object_type type;
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
/* PAIR */
#define pair_car(x) ((x)->values.pair.car)
#define pair_cdr(x) ((x)->values.pair.cdr)
#define is_pair(x) (PAIR == (x)->type)
/* EMPTY_LIST */
#define is_null(x) (EMPTY_LIST == (x)->type)
/* SYMBOL */
#define symbol_name(x) ((x)->values.symbol.name)
#define is_symbol(x) (SYMBOL == (x)->type)
/* FIXNUM */
#define fixnum_value(x) ((x)->values.fixnum.value)
/* PRIMITIVE_PROC */
#define primitive_C_proc(x) ((x)->values.primitive_proc.C_proc)
#define is_primitive(x) (PRIMITIVE_PROC == (x)->type)
/* CHARACTER */
#define char_value(x) ((x)->values.character.value)
/* STRING */
#define string_value(x) ((x)->values.string.value)
/* COMPOUND_PROC */
#define compound_proc_parameters(x) ((x)->values.compound_proc.parameters)
#define compound_proc_body(x) ((x)->values.compound_proc.raw_body)
#define compound_proc_environment(x) ((x)->values.compound_proc.environment)
#define is_compound(x) (COMPOUND_PROC == (x)->type)
#define is_function(x) (is_primitive(x) || is_compound(x))
/* BOOLEAN */
#define is_bool(x) (BOOLEAN == (x)->type)
#define bool_value(x) ((x)->values.boolean.value)
#define is_true(x) (is_bool(x) && 1 == bool_value(x))
#define is_false(x) (is_bool(x) && 0 == bool_value(x))
/* UNDEFINED */
#define is_undefined(x) (UNDEFINED == (x)->type)
/* FILE_IN_PORT */
#define in_port_stream(x) ((x)->values.file_in_port.stream)
#define in_port_linum(x) ((x)->values.file_in_port.line_num)
/* FILE_OUT_PORT */
#define out_port_stream(x) ((x)->values.file_out_port.stream)
/* EOF */
#define is_eof(x) (EOF_OBJECT == (x)->type)

/*
 * pair_cadr: second element
 * pair_caddr: third element
 */
#define pair_cadr(x) pair_car(pair_cdr(x))
#define pair_cddr(x) pair_cdr(pair_cdr(x))
#define pair_caddr(x) pair_car(pair_cddr(x))
#define pair_caar(x) pair_car(pair_car(x))
#define pair_cdar(x) pair_cdr(pair_car(x))
#define pair_cdddr(x) pair_cdr(pair_cddr(x))

#endif
