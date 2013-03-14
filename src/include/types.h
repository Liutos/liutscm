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
  } values;
} *lisp_object_t;

#endif
