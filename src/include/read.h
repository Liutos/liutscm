#ifndef READ_H
#define READ_H

#include <stdio.h>

#include "types.h"

extern lisp_object_t read_object(lisp_object_t);
extern char port_read_char(sexp);

#endif
