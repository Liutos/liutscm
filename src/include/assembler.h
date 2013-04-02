#ifndef ASSEMBLER_H
#define ASSEMBLER_H

#include "types.h"

enum code_type {
  ARGS,
  ARGSD,
  CALL,
  CALLJ,
  CAR,
  CDR,
  CONST,
  FJUMP,
  FN,
  GSET,
  GVAR,
  JUMP,
  LSET,
  LVAR,
  POP,
  PRIM,
  RETURN,
  SAVE,
  TJUMP,
  /* Integer arithmetic operations */
  IADD,
  ISUB,
  IMUL,
  IDIV,
};

struct code_t {
  enum code_type code;
  char *name;
  int arity;
};

extern sexp assemble_code(sexp);

#endif
