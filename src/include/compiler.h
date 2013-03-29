#ifndef COMPILE_H
#define COMPILE_H

#include "types.h"

extern sexp compile_object(sexp, sexp, int, int);
extern sexp compile_as_fn(sexp, sexp);

#endif
