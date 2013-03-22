/*
 * object.h
 *
 *
 *
 * Copyright (C) 2013-03-17 liutos <mat.liutos@gmail.com>
 */
#ifndef OBJECT_H
#define OBJECT_H

#include <stdio.h>

#include "types.h"

extern lisp_object_t make_fixnum(int);
extern lisp_object_t make_eof_object(void);
extern lisp_object_t make_boolean(int);
extern lisp_object_t make_true(void);
extern lisp_object_t make_false(void);
extern lisp_object_t make_character(char);
extern lisp_object_t make_string(char *);
extern lisp_object_t make_empty_list(void);
extern lisp_object_t make_close_object(void);
extern lisp_object_t make_dot_object(void);
extern lisp_object_t make_symbol(char *);
extern lisp_object_t make_pair(lisp_object_t, lisp_object_t);
extern lisp_object_t make_undefined(void);
extern lisp_object_t find_or_create_symbol(char *);
extern void set_binding(lisp_object_t, lisp_object_t, lisp_object_t);
extern lisp_object_t get_variable_value(lisp_object_t, lisp_object_t);
extern void add_binding(lisp_object_t, lisp_object_t, lisp_object_t);
extern lisp_object_t extend_environment(lisp_object_t, lisp_object_t, lisp_object_t);
extern lisp_object_t make_file_in_port(FILE *);
extern lisp_object_t make_file_out_port(FILE *);
extern lisp_object_t make_list(lisp_object_t e, ...);
extern lisp_object_t search_binding_index(lisp_object_t, lisp_object_t);
extern lisp_object_t make_vector(unsigned int);
extern int pair_length(lisp_object_t);
extern lisp_object_t pair_nthcdr(lisp_object_t, int);
extern void inc_ref_count(lisp_object_t);
extern void dec_ref_count(lisp_object_t);
extern struct lisp_object_t *init_heap(void);
extern void free_file_out_port(lisp_object_t);
extern lisp_object_t make_flonum(float);

extern struct lisp_object_t *objects_heap;

#define environment_vars(x) pair_caar(x)
#define environment_vals(x) pair_cdar(x)
#define enclosing_environment(x) pair_cdr(x)

#endif
