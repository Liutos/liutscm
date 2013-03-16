/*
 * types.h
 *
 * Definition of data types in Lisp
 *
 * Copyright (C) 2013-03-13 liutos <mat.liutos@gmail.com>
 */
#ifndef TYPES_H
#define TYPES_H

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
  } values;
} *lisp_object_t;

#define pair_car(x) ((x)->values.pair.car)
#define pair_cdr(x) ((x)->values.pair.cdr)
#define is_pair(x) (PAIR == (x)->type)
#define is_null(x) (EMPTY_LIST == (x)->type)
#define symbol_name(x) ((x)->values.symbol.name)
#define is_symbol(x) (SYMBOL == (x)->type)
#define fixnum_value(x) ((x)->values.fixnum.value)
#define primitive_C_proc(x) ((x)->values.primitive_proc.C_proc)
#define char_value(x) ((x)->values.character.value)
#define string_value(x) ((x)->values.string.value)

typedef struct table_entry_t {
  char *key;
  lisp_object_t value;
  struct table_entry_t *next;
} *table_entry_t;

typedef struct hash_table_t {
  unsigned int (*hash_function)(char *); /* Compute the index from the key */
  int (*comparator)(char *, char *);    /* Compare the first and second arguments. Return zero when they're not equal and non-zero otherwise. */
  table_entry_t *datum;
  unsigned int size;
} *hash_table_t;

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
