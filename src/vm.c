/*
 * vm.c
 *
 *
 *
 * Copyright (C) 2013-03-18 liutos <mat.liutos@gmail.com>
 */
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "types.h"
#include "object.h"
#include "eval.h"

extern void write_object(lisp_object_t, lisp_object_t);

enum code_type {
  CONST,
  LVAR,
  FJUMP,
  JUMP,
  LSET,
  POP,
  GVAR,
  CALL,
  FN,
  ARGS,
  RETURN,
};

enum code_type code_name(lisp_object_t code) {
#define S(name) find_or_create_symbol(name)
  lisp_object_t name = pair_car(code);
  if (S("CONST") == name)
    return CONST;
  if (S("LVAR") == name)
    return LVAR;
  if (S("FJUMP") == name) return FJUMP;
  if (S("JUMP") == name) return JUMP;
  if (S("LSET") == name) return LSET;
  if (S("POP") == name) return POP;
  if (S("GVAR") == name) return GVAR;
  if (S("CALL") == name) return CALL;
  if (S("FN") == name) return FN;
  if (S("ARGS") == name) return ARGS;
  if (S("RETURN") == name) return RETURN;
  fprintf(stderr, "code_name - Unsupported code: %s\n", symbol_name(pair_car(code)));
  exit(1);
}

/* lisp_object_t code_arg0(lisp_object_t code) { */
/*   return pair_cadr(code); */
/* } */
#define code_arg0(x) pair_cadr(code)
/* lisp_object_t code_arg1(lisp_object_t code) { */
/*   return pair_caddr(code); */
/* } */
#define code_arg1(x) pair_caddr(code)
/* lisp_object_t code_arg2(lisp_object_t code) { */
/*   return pair_cadddr(code); */
/* } */
#define code_arg2(x) pair_cadddr(code)

lisp_object_t get_variable_by_index(int i, int j, lisp_object_t environment) {
  while (i != 0) {
    environment = enclosing_environment(environment);
    i--;
  }
  lisp_object_t vals = environment_vals(environment);
  while (j != 0) {
    vals = pair_cdr(vals);
    j--;
  }
  return pair_car(vals);
}

void set_variable_by_index(int i, int j, lisp_object_t new_value, lisp_object_t environment) {
  while (i-- != 0)
    environment = enclosing_environment(environment);
  lisp_object_t vals = environment_vals(environment);
  while (j-- != 0)
    vals = pair_cdr(vals);
  pair_car(vals) = new_value;
}

lisp_object_t make_arguments(lisp_object_t stack, int n) {
  if (0 == n)
    return make_empty_list();
  else
    return make_pair(pair_car(stack),
                     make_arguments(pair_cdr(stack), n - 1));
}

#define push(e, stack) stack = make_pair(e, stack)
#define pop(stack) stack = pair_cdr(stack)
#define nth_pop(stack, n) stack = pair_nthcdr(stack, n)

void push_value2env(lisp_object_t stack, int n, lisp_object_t environment) {
  lisp_object_t vals = environment_vals(environment);
  for (; n > 0; n--) {
    lisp_object_t top = pair_car(stack);
    pop(stack);
    push(top, vals);
  }
  environment_vals(environment) = vals;
}

/* Run the code generated from compiling an S-exp by function `assemble_code'. */
lisp_object_t run_compiled_code(lisp_object_t compiled_code, lisp_object_t environment, lisp_object_t stack) {
  assert(is_vector(compiled_code));
  int pc = 0;
  while (1) {
    if (pc >= vector_length(compiled_code))
      break;
    lisp_object_t code = /* pair_car(compiled_code); */vector_data_at(compiled_code, pc);
    switch (code_name(code)) {
      case CONST:
        push(code_arg0(code), stack);
        pc++;
        break;
      case LVAR: {
        int i = fixnum_value(code_arg0(code));
        int j = fixnum_value(code_arg1(code));
        push(get_variable_by_index(i, j, environment), stack);
      }
        pc++;
        break;
      case LSET: {
        int i = fixnum_value(code_arg0(code));
        int j = fixnum_value(code_arg1(code));
        lisp_object_t value = pair_car(stack);
        pop(stack);
        set_variable_by_index(i, j, value, environment);
        push(make_undefined(), stack);
      }
        pc++;
        break;
      case FJUMP: {
        lisp_object_t top = pair_car(stack);
        pop(stack);
        if (is_false(top)) {
          pc = fixnum_value(code_arg0(code));
        } else
          pc++;
      }
        break;
      case POP:
        pop(stack);
        pc++;
        break;
      case CALL: {
        lisp_object_t n = code_arg0(code);
        lisp_object_t op = pair_car(stack);
        pop(stack);
        lisp_object_t args = make_arguments(stack, fixnum_value(n));
        nth_pop(stack, fixnum_value(n));
        push(eval_application(op, args), stack);
        pc++;
      }
        break;
      /* case ARGS: { */
      /*   lisp_object_t n = code_arg0(code); */
      /*   push_value2env(stack, fixnum_value(n), environment); */
      /*   nth_pop(stack, fixnum_value(n)); */
      /*   pc++; */
      /* } */
      /*   break; */
      case FN:
        push(code_arg0(code), stack);
        pc++;
        break;
      default :
        fprintf(stderr, "run_compiled_code - Unknown code ");
        write_object(pair_car(code), make_file_out_port(stdout));
        /* exit(1); */
        return stack;
    }
  }
  return stack;
}

int is_label(lisp_object_t code) {
  return is_symbol(code);
}

lisp_object_t extract_labels_aux(lisp_object_t compiled_code, int offset, int *length) {
  if (is_null(compiled_code)) {
    *length = offset;
    return make_empty_list();
  } else {
    lisp_object_t first = pair_car(compiled_code);
    lisp_object_t rest = pair_cdr(compiled_code);
    if (is_label(first)) {
      lisp_object_t lo = make_pair(first, make_fixnum(offset));
      return make_pair(lo, extract_labels_aux(rest, offset, length));
    } else
      return extract_labels_aux(rest, offset + 1, length);
  }
}

lisp_object_t extract_labels(lisp_object_t compiled_code, int *length) {
  return extract_labels_aux(compiled_code, 0, length);
}

int is_with_label(lisp_object_t code) {
  switch (code_name(code)) {
    case FJUMP:
    case JUMP: return 1;
    default : return 0;
  }
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

/* Convert the byte code stored as a list in COMPILED_PROC into a vector filled of the same code, except the label in instructions with label will be replace by an integer offset. */
lisp_object_t vectorize_code(lisp_object_t compiled_code, int length, lisp_object_t label_table) {
  lisp_object_t code_vector = make_vector(length);
  int i = 0;
  while (is_pair(compiled_code)) {
    lisp_object_t code = pair_car(compiled_code);
    if (!is_label(code)) {
      if (is_with_label(code)) {
        code_arg0(code) = search_label_offset(code_arg0(code), label_table);
        label_table = pair_cdr(label_table);
      }
      vector_data_at(code_vector, i) = code;
      i++;
    }
    compiled_code = pair_cdr(compiled_code);
  }
  return code_vector;
}

lisp_object_t assemble_code(lisp_object_t compiled_code) {
  int length;
  lisp_object_t label_table = extract_labels(compiled_code, &length);
  return vectorize_code(compiled_code, length, label_table);
}
