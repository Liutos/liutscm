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
  } values;
} *lisp_object_t;

#define pair_car(x) ((x)->values.pair.car)
#define pair_cdr(x) ((x)->values.pair.cdr)
#define is_pair(x) (PAIR == (x)->type)
#define is_null(x) (EMPTY_LIST == (x)->type)

#endif
