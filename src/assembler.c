/*
 * assembler.c
 *
 * Assemble the byte-code
 *
 * Copyright (C) 2013-04-02 liutos <mat.liutos@gmail.com>
 */
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include "assembler.h"
#include "object.h"
#include "types.h"
#include "write.h"

#define C(nm, nargs) {.code=nm, .name=#nm, .arity=nargs}
#define arg1(x) pair_cadr(x)
#define arg2(x) pair_caddr(x)
#define arg3(x) pair_cadddr(x)
#define opcode(x) pair_car(x)

extern int symbol_name_comparator(char *, char *);

struct code_t opcodes[] = {
  C(ARGS, 1),
  C(ARGSD, 1),
  C(CALL, 1),
  C(CALLJ, 1),
  C(CONST, 1),
  C(FJUMP, 1),
  C(FN, 1),
  C(GSET, 1),
  C(GVAR, 1),
  C(JUMP, 1),
  C(LSET, 2),
  C(LVAR, 2),
  C(MC, 1),
  C(POP, 0),
  C(PRIM, 1),
  C(PRIM0, 0),
  C(PRIM1, 0),
  C(PRIM2, 0),
  C(PRIM3, 0),
  C(RETURN, 0),
  C(SAVE, 1),
  C(TJUMP, 1),
  C(IADD, 0),
  C(ISUB, 0),
  C(IMUL, 0),
  C(IDIV, 0),
  C(CAR, 0),
  C(CDR, 0),
  C(EQ, 0),
};

/* Categorize the instruction */
int is_in_set(sexp opcode, char *set[], int len) {
  for (int i = 0; i < len; i++)
    if (opcode == S(set[i])) return yes;
  return no;
}

int get_code_arity(sexp opcode) {
  for (int i = 0; i < sizeof(opcodes) / sizeof(struct code_t); i++)
    if (!symbol_name_comparator(opcodes[i].name, symbol_name(opcode)))
      return opcodes[i].arity;
  port_format(scm_out_port, "Unexpected opcode: %*\n", opcode);
  exit(1);
}

/* The parameter type of the three following functions is a symbol */
int is_const_op(sexp opcode) {
  return get_code_arity(opcode) == 0;
}

int is_unary_op(sexp opcode) {
  return get_code_arity(opcode) == 1;
}

int is_binary_op(sexp opcode) {
  return get_code_arity(opcode) == 2;
}

/* How much bytes should the assemble code occupy?
 * bytes size = 1 byte + arity * 1 byte
 */
int instruction_length(sexp ins) {
  sexp opcode = opcode(ins);
  /* if (is_const_op(opcode)) return 1; */
  /* if (is_unary_op(opcode)) return 1 + 1; */
  /* if (is_binary_op(opcode)) return 1 + 2 * 1; */
  /* else { */
  /*   port_format(scm_out_port, "Unexpected opcode %*\n", opcode); */
  /*   exit(1); */
  /* } */
  return 1 + get_code_arity(opcode) * 1;
}

sexp extract_labels_aux(sexp compiled_code, int offset, int *length) {
  if (is_null(compiled_code)) {
    *length = offset;
    return EOL;
  } else {
    sexp first = pair_car(compiled_code);
    sexp rest = pair_cdr(compiled_code);
    if (is_label(first)) {
      sexp lo = make_pair(first, make_fixnum(offset));
      return make_pair(lo, extract_labels_aux(rest, offset, length));
    } else {
      offset = offset + instruction_length(first);
      /* offset++; */
      return extract_labels_aux(rest, offset, length);
    }
  }
}

/* Returns an a-list contains label-offset pairs. Parameter `length' stores the length of compiled code. */
sexp extract_labels(sexp compiled_code, int *length) {
  assert(is_pair(compiled_code));
  return extract_labels_aux(compiled_code, 0, length);
}

int is_with_label(lisp_object_t code) {
  static char *label_ins[] = {
    "FJUMP", "JUMP", "SAVE", "TJUMP",
  };
  for (int i = 0; i < sizeof(label_ins) / sizeof(char *); i++)
    if (pair_car(code) == S(label_ins[i]))
      return yes;
  return no;
}

lisp_object_t search_label_offset(lisp_object_t label, lisp_object_t label_table) {
  if (is_null(label_table)) {
    fprintf(stderr, "Impossible - SEARCH_LABEL_OFFSET\n");
    exit(1);
  }
  lisp_object_t lo = pair_car(label_table);
  if (label == pair_car(lo))
    return pair_cdr(lo);
  else
    return search_label_offset(label, pair_cdr(label_table));
}

sexp to_opbyte(sexp opcode) {
  for (int i = 0; i < sizeof(opcodes) / sizeof(struct code_t); i++)
    if (!symbol_name_comparator(opcodes[i].name, symbol_name(opcode)))
      return make_fixnum(opcodes[i].code);
  port_format(scm_out_port, "Unexpected opcode: %*\n", opcode);
  exit(1);
}

void write_arg_bytes(sexp code[], int *index, sexp ins) {
  sexp opcode = opcode(ins);
  if (is_const_op(opcode)) return;
  if (is_unary_op(opcode)) {
    code[*index] = arg1(ins);
    (*index)++;
    return;
  }
  if (is_binary_op(opcode)) {
    code[*index] = arg1(ins);
    (*index)++;
    code[*index] = arg2(ins);
    (*index)++;
    return;
  }
  port_format(scm_out_port, "Unexpected ins: %*\n", ins);
  exit(1);
}

/* Convert the byte code stored as a list in COMPILED_PROC into a vector filled of the same code, except the label in instructions with label will be replace by an integer offset. */
sexp vectorize_code(sexp compiled_code, int length, sexp label_table) {
  sexp code_vector = make_vector(length);
  int i = 0;
  while (is_pair(compiled_code)) {
    sexp code = pair_car(compiled_code);
    if (!is_label(code)) {
      if (is_with_label(code) && is_label(arg1(code))) {
        /* port_format(scm_out_port, "Replacing the label of %*\n", code); */
        arg1(code) = search_label_offset(arg1(code), label_table);
        label_table = pair_cdr(label_table);
      }
      vector_data_at(code_vector, i) = to_opbyte(opcode(code));
      i++;
      write_arg_bytes(vector_datum(code_vector), &i, code);
    }
    compiled_code = pair_cdr(compiled_code);
  }
  return code_vector;
}

/* Assembler */
lisp_object_t assemble_code(lisp_object_t compiled_code) {
  assert(is_pair(compiled_code));
  int length;
  lisp_object_t label_table = extract_labels(compiled_code, &length);
  return vectorize_code(compiled_code, length, label_table);
}
