/*
 * read.h
 *
 *
 *
 * Copyright (C) 2013-03-13 liutos <mat.liutos@gmail.com>
 */
#ifndef READ_H
#define READ_H

#include "types.h"
#include <stdio.h>

extern lisp_object_t read_object(FILE *);
extern hash_table_t make_hash_table(unsigned int (*)(char *), int (*)(char *, char *), unsigned int);
extern hash_table_t symbol_table;
extern unsigned int hash_symbol_name(char *);
extern int symbol_name_comparator(char *, char *);
extern lisp_object_t find_or_create_symbol(char *);
extern lisp_object_t make_true(void);
extern lisp_object_t make_false(void);

#endif
