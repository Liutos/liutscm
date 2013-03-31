/*
 * vm.c
 *
 * The implementation of the virtual machine
 *
 * Copyright (C) 2013-03-18 liutos <mat.liutos@gmail.com>
 */
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include "eval.h"
#include "object.h"
#include "types.h"
#include "write.h"

#define SR(x) if (S(#x) == name) return x
#define arg1(x) pair_cadr(x)
#define arg2(x) pair_caddr(x)
#define arg3(x) pair_cadddr(x)
#define nth_pop(stack, n) stack = pair_nthcdr(stack, n)
#define pop(stack) stack = pair_cdr(stack)

#define pop_to(stack, var)                      \
  sexp var = first(stack);                      \
  pop(stack);

#define push(e, stack) stack = make_pair(e, stack)
#define C(n) {.code=n, .name=#n}

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
  POP,
  PRIM,
  RETURN,
  SAVE,
  TJUMP,
};

struct code_t {
  enum code_type code;
  char *name;
};

static struct code_t opcodes[] = {
  C(ARGS),
  C(ARGSD),
  C(CALL),
  C(CALLJ),
  C(CONST),
  C(FJUMP),
  C(FN),
  C(GSET),
  C(GVAR),
  C(JUMP),
  C(LSET),
  C(LVAR),
  C(POP),
  C(PRIM),
  C(RETURN),
  C(SAVE),
  C(TJUMP),
};

enum code_type code_name(lisp_object_t code) {
  assert(is_pair(code));
  lisp_object_t name = pair_car(code);
  for (int i = 0; i < sizeof(opcodes) / sizeof(struct code_t); i++)
    if (name == S(opcodes[i].name))
      return opcodes[i].code;
  port_format(scm_out_port, "code_name - Unsupported code: %*\n", pair_car(code));
  exit(1);
}

sexp get_variable_by_index(int i, int j, sexp env) {
  for (; i > 0; i--) env = environment_outer(env);
  sexp bindings = environment_bindings(env);
  for (; j > 0; j--) bindings = pair_cdr(bindings);
  return pair_cdar(bindings);
}

void set_variable_by_index(int i, int j, sexp new_value, sexp env) {
  for (; i > 0; i--) env = environment_outer(env);
  sexp bindings = environment_bindings(env);
  for (; j > 0; j--) bindings = pair_cdr(bindings);
  pair_cdar(bindings) = new_value;
}

lisp_object_t make_arguments(lisp_object_t stack, int n) {
  sexp args = EOL;
  for (; n > 0; n--) {
    pop_to(stack, e);
    push(e, args);
  }
  return args;
}

void push_value2env(lisp_object_t stack, int n, lisp_object_t environment) {
  lisp_object_t vals = environment_vals(environment);
  for (; n > 0; n--) {
    lisp_object_t top = pair_car(stack);
    pop(stack);
    push(top, vals);
  }
  environment_vals(environment) = vals;
}

/* Assembler */
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
    } else
      return extract_labels_aux(rest, offset + 1, length);
  }
}

/* Returns an a-list contains label-offset pairs. Parameter `length' stores the length of compiled code. */
sexp extract_labels(sexp compiled_code, int *length) {
  return extract_labels_aux(compiled_code, 0, length);
}

int is_with_label(lisp_object_t code) {
  switch (code_name(code)) {
    case FJUMP:
    case JUMP:
    case TJUMP: return 1;
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
        arg1(code) = search_label_offset(arg1(code), label_table);
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
  assert(is_pair(compiled_code));
  int length;
  lisp_object_t label_table = extract_labels(compiled_code, &length);
  return vectorize_code(compiled_code, length, label_table);
}

/* Virtual Machine */
void nth_insert_pair(int n, lisp_object_t object, lisp_object_t pair) {
  while (n - 1 != 0) {
    pair = pair_cdr(pair);
    n--;
  }
  lisp_object_t new_cdr = make_pair(object, pair_cdr(pair));
  pair_cdr(pair) = new_cdr;
}

/* Moves n elements from top of `stack' into `env' */
void move_args(int n, sexp *stack, sexp *env) {
  *env = extend_environment(EOL, EOL, *env);
  sexp bindings = environment_bindings(*env);
  for (; n > 0; n--) {
    pop_to(*stack, arg);
    push(make_pair(EOL, arg), bindings);
  }
  environment_bindings(*env) = bindings;
}

sexp top(sexp stack) {
  if (is_null(stack)) return EOL;
  return pair_car(stack);
}

/* Run the code generated from compiling an S-exp by function `assemble_code'. */
sexp run_compiled_code(sexp obj, sexp env, sexp stack) {
  assert(is_compiled_proc(obj));
  sexp code = compiled_proc_code(obj);
  int nargs = 0;
  code = assemble_code(code);
  /* port_format(scm_out_port, "-- %*\n", code); */
  for (int pc = 0; pc < vector_length(code); pc++) {
    assert(is_vector(code));
    sexp ins = vector_data_at(code, pc);
    /* port_format(scm_out_port, "Processing: %*\n", ins); */
    switch (code_name(ins)) {
      /* Function call/return instructions */
      case ARGS: {
        if (nargs != fixnum_value(arg1(ins))) {
          port_format(scm_out_port,
                      "Wrong argument number: %d but expecting %d\n",
                      arg1(ins), make_fixnum(nargs));
          exit(1);
        }
        move_args(fixnum_value(arg1(ins)), &stack, &env);
      } break;
      case ARGSD: {
        int n = fixnum_value(arg1(ins));
        if (nargs < n) {
          port_format(scm_out_port, "Unscientific!\n");
          exit(1);
        }
        sexp rest = make_arguments(stack, nargs - n);
        nth_pop(stack, (nargs - n));
        env = extend_environment(EOL, EOL, env);
        sexp bindings = environment_bindings(env);
        push(make_pair(EOL, rest), bindings);
        for (; n > 0; n--) {
          pop_to(stack, arg);
          push(make_pair(EOL, arg), bindings);
        }
        environment_bindings(env) = bindings;
        /* port_format(scm_out_port, "Current bindings: %*\n", */
        /*             environment_bindings(env)); */
        /* exit(1); */
      } break;
      case CALLJ: {
        nargs = fixnum_value(arg1(ins));
        /* port_format(scm_out_port, "nargs: %d\n", make_fixnum(nargs)); */
        pop_to(stack, proc);
        code = assemble_code(compiled_proc_code(proc));
        env = compiled_proc_env(proc);
        pc = -1;
      } break;
      case FN: {
        sexp fn = arg1(ins);
        sexp pars = compiled_proc_args(fn);
        sexp code = compiled_proc_code(fn);
        push(make_compiled_proc(pars, code, env), stack);
      } break;
      case PRIM: {
        pop_to(stack, op);
        sexp args = make_arguments(stack, fixnum_value(arg1(ins)));
        nth_pop(stack, fixnum_value(arg1(ins)));
        push(eval_application(op, args), stack);
      } break;
      case RETURN: {
        pop_to(stack, value);
        if (is_return_info(top(stack))) {
          port_format(scm_out_port, "WTF - I got a return info\n");
          exit(1);
        }
        push(value, stack);
        goto halt;
      } break;
      case SAVE: push(make_return_info(code, pc, env), stack); break;

        /* Variable/Stack manipulation instructions */
      case CONST: push(arg1(ins), stack); break;
      case GSET: {
        sexp value = top(stack);
        sexp var = arg1(ins);
        set_binding(var, value, env);
      } break;
      case GVAR: {
        sexp var = arg1(ins);
        push(get_variable_value(var, env), stack);
      } break;
      case LSET: {
        int i = fixnum_value(arg1(ins));
        int j = fixnum_value(arg2(ins));
        set_variable_by_index(i, j, top(stack), env);
      } break;
      case LVAR: {
        int i = fixnum_value(arg1(ins));
        int j = fixnum_value(arg2(ins));
        push(get_variable_by_index(i, j, env), stack);
      } break;
      case POP: pop(stack); break;

        /* Branching instructions */
      case FJUMP: {
        pop_to(stack, e);
        if (is_false(e)) pc = fixnum_value(arg1(ins)) - 1;
      } break;
      case JUMP: pc = fixnum_value(arg1(ins)) - 1; break;
      case TJUMP: {
        pop_to(stack, e);
        if (is_true(e)) pc = fixnum_value(arg1(ins)) - 1;
      } break;

      default :
        fprintf(stderr, "run_compiled_code - Unknown code ");
        write_object(pair_car(ins), make_file_out_port(stdout));
        port_format(scm_out_port, "%*\n", env);
        return stack;
    }
    /* port_format(scm_out_port, "stack: %*\n", stack); */
    /* port_format(scm_out_port, "env: %*\n", env); */
  }
halt:
  return top(stack);
}
