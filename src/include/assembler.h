#ifndef ASSEMBLER_H
#define ASSEMBLER_H

#include "types.h"

enum code_type {
  ARGS,
  ARGSD,
  CALL,
  CALLJ,
  CONST,
  FJUMP,
  FN,
  GSET,
  GVAR,
  JUMP,
  LSET,
  LVAR,
  MC,
  POP,
  PRIM,
  PRIM0,
  PRIM1,
  PRIM2,
  PRIM3,
  RETURN,
  SAVE,
  TJUMP,
  /* Integer arithmetic operations */
  IADD,
  ISUB,
  IMUL,
  IDIV,
  /* Pair operations */
  CAR,
  CDR,
  /* Others */
  EQ,
};

struct code_t {
  enum code_type code;
  char *name;
  int arity;
};

extern struct code_t opcodes[];

extern sexp assemble_code(sexp);

#endif
