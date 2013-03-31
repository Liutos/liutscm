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
#define opcode(x) pair_car(x)
#define pop(stack) stack = pair_cdr(stack)

#define pop_to(stack, var)                      \
  sexp var = first(stack);                      \
  pop(stack);

#define push(e, stack) stack = make_pair(e, stack)
#define C(n) {.code=n, .name=#n}

extern int symbol_name_comparator(char *, char *);

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

char *const_opcodes[] = {
  "POP", "RETURN",
};

char *unary_opcodes[] = {
  "ARGS", "ARGSD", "CALL", "CALLJ", "CONST", "FJUMP", "FN", "GSET", "GVAR",
  "JUMP", "PRIM", "SAVE", "TJUMP",
};

char *binary_opcodes[] = {
  "LSET", "LVAR",
};

enum code_type code_name(lisp_object_t code) {
  /* assert(is_pair(code)); */
  /* lisp_object_t name = pair_car(code); */
  /* for (int i = 0; i < sizeof(opcodes) / sizeof(struct code_t); i++) */
  /*   if (name == S(opcodes[i].name)) */
  /*     return opcodes[i].code; */
  /* port_format(scm_out_port, "code_name - Unsupported code: %*\n", pair_car(code)); */
  /* exit(1); */
  return fixnum_value(code);
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

/* Categorize the instruction */
int is_in_set(sexp opcode, char *set[], int len) {
  for (int i = 0; i < len; i++)
    if (opcode == S(set[i])) return yes;
  return no;
}

int is_const_op(sexp opcode) {
  /* for (int i = 0; i < sizeof(const_opcodes) / sizeof(char *); i++) */
  /*   if (opcode == S(const_opcodes[i])) */
  /*     return yes; */
  /* return no; */
  return is_in_set(opcode, const_opcodes, sizeof(const_opcodes) / sizeof(char *));
}

int is_unary_op(sexp opcode) {
  /* for (int i = 0; i < sizeof(unary_opcodes) / sizeof(char *); i++) */
  /*   if (opcode == S(unary_opcodes[i])) */
  /*     return yes; */
  /* return no; */
  return is_in_set(opcode, unary_opcodes, sizeof(unary_opcodes) / sizeof(char *));
}

int is_binary_op(sexp opcode) {
  /* for (int i = 0; i < sizeof(binary_opcodes) / sizeof(char *); i++) */
  /*   if (opcode == S(binary_opcodes[i])) */
  /*     return yes; */
  /* return no; */
  return is_in_set(opcode, binary_opcodes, sizeof(binary_opcodes) / sizeof(char *));
}

/* How much bytes should the assemble code occupy? */
int instruction_length(sexp ins) {
  sexp opcode = opcode(ins);
  if (is_const_op(opcode)) return 1;
  if (is_unary_op(opcode)) return /* 1 + sizeof(sexp) */1 + 1;
  if (is_binary_op(opcode)) return /* 1 + 2 * sizeof(sexp) */1 + 2 * 1;
  else {
    port_format(scm_out_port, "Unexpected opcode %*\n", opcode);
    exit(1);
  }
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
  switch (code_name(code)) {
    case FJUMP:
    case JUMP:
    case SAVE:
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
      if (is_with_label(code)) {
        arg1(code) = search_label_offset(arg1(code), label_table);
        label_table = pair_cdr(label_table);
      }
      /* vector_data_at(code_vector, i) = code; */
      /* i++; */
      vector_data_at(code_vector, i) = to_opbyte(opcode(code));
      i++;
      write_arg_bytes(vector_datum(code_vector), &i, code);
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
  port_format(scm_out_port, "\n-> %*\n", code);
  /* port_format(scm_out_port, "-- %*\n", code); */
  /* for (int pc = 0; pc < vector_length(code); pc++) { */
  int pc = 0;
  while (pc < vector_length(code)) {
    assert(is_vector(code));
    sexp ins = vector_data_at(code, pc);
    /* port_format(scm_out_port, "Processing: %*\n", ins); */
    switch (code_name(ins)) {
      /* Function call/return instructions */
      case ARGS: {
        pc++;
        sexp n = vector_data_at(code, pc);
        if (nargs != fixnum_value(n)) {
          port_format(scm_out_port,
                      "Wrong argument number: %d but expecting %d\n",
                      n, make_fixnum(nargs));
          exit(1);
        }
        move_args(fixnum_value(n), &stack, &env);
        pc++;
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
        pc++;
      } break;
      case CALLJ: {
        nargs = fixnum_value(arg1(ins));
        /* port_format(scm_out_port, "nargs: %d\n", make_fixnum(nargs)); */
        pop_to(stack, proc);
        code = assemble_code(compiled_proc_code(proc));
        env = compiled_proc_env(proc);
        /* pc = -1; */
        pc = 0;
      } break;
      case FN: {
        sexp fn = arg1(ins);
        sexp pars = compiled_proc_args(fn);
        sexp code = compiled_proc_code(fn);
        push(make_compiled_proc(pars, code, env), stack);
        pc++;
      } break;
      case PRIM: {
        pop_to(stack, op);
        sexp args = make_arguments(stack, fixnum_value(arg1(ins)));
        nth_pop(stack, fixnum_value(arg1(ins)));
        push(eval_application(op, args), stack);
        pc++;
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
      case SAVE: push(make_return_info(code, pc, env), stack); pc++; break;

        /* Variable/Stack manipulation instructions */
      case CONST: push(arg1(ins), stack); pc++; break;
      case GSET: {
        sexp value = top(stack);
        sexp var = arg1(ins);
        set_binding(var, value, env);
        pc++;
      } break;
      case GVAR: {
        sexp var = arg1(ins);
        push(get_variable_value(var, env), stack);
        pc++;
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
        pc++;
      } break;
      case POP: pop(stack); pc++; break;

        /* Branching instructions */
      case FJUMP: {
        pop_to(stack, e);
        if (is_false(e)) pc = fixnum_value(arg1(ins));
      } break;
      case JUMP: pc = fixnum_value(arg1(ins)); break;
      case TJUMP: {
        pop_to(stack, e);
        if (is_true(e)) pc = fixnum_value(arg1(ins));
      } break;

      default :
        fprintf(stderr, "run_compiled_code - Unknown code ");
        write_object(pair_car(ins), make_file_out_port(stdout));
        port_format(scm_out_port, "%*\n", env);
        return stack;
    }
    /* port_format(scm_out_port, "stack: %*\n", stack); */
    /* port_format(scm_out_port, "env: %*\n", env); */
    /* pc++; */
  }
halt:
  return top(stack);
}
