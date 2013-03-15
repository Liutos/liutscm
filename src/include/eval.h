/*
 * eval.h
 *
 *
 *
 * Copyright (C) 2013-03-13 liutos <mat.liutos@gmail.com>
 */
#ifndef EVAL_H
#define EVAL_H

#include "types.h"

extern lisp_object_t eval_object(lisp_object_t, lisp_object_t);
extern lisp_object_t make_empty_environment(void);
extern lisp_object_t make_startup_environment(void);
extern void init_environment(lisp_object_t);

#endif
