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
  } values;
} *lisp_object_t;

#define pair_car(x) ((x)->values.pair.car)
#define pair_cdr(x) ((x)->values.pair.cdr)
#define is_pair(x) (PAIR == (x)->type)
#define is_null(x) (EMPTY_LIST == (x)->type)
#define symbol_name(x) ((x)->values.symbol.name)

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

#endif
