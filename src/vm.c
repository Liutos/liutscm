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

#include "assembler.h"
#include "eval.h"
#include "object.h"
#include "types.h"
#include "write.h"

#define SR(x) if (S(#x) == name) return x
#define nth_pop(stack, n) stack = pair_nthcdr(stack, n)
#define pop(stack) stack = pair_cdr(stack)

/* #define pop_to(stack, var)                      \ */
/*   sexp var = first(stack);                      \ */
/*   pop(stack); */
#define pop_to(stack, var) sexp var = vector_pop(stack)

#define push(e, stack) stack = make_pair(e, stack)

enum code_type code_name(lisp_object_t code) {
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
  /* sexp args = EOL; */
  /* for (; n > 0; n--) { */
  /*   pop_to(stack, e); */
  /*   push(e, args); */
  /* } */
  /* return args; */
  sexp head, cur, pre;
  pre = head = make_pair(EOL, EOL);
  for (; n > 0; n--) {
    pop_to(stack, e);
    cur = make_pair(e, EOL);
    pair_cdr(pre) = cur;
    pre = cur;
  }
  return pair_cdr(head);
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
void move_args(int n, sexp stack, sexp *env) {
  *env = extend_environment(EOL, EOL, *env);
  sexp bindings = environment_bindings(*env);
  for (; n > 0; n--) {
    pop_to(stack, arg);
    push(make_pair(EOL, arg), bindings);
  }
  environment_bindings(*env) = bindings;
}

void move_argsd(int nargs, int n, sexp stack, sexp *env) {
  sexp bindings, cur, pre;
  pre = bindings = make_pair(EOL, EOL);
  for (int i = 0; i < n; i++) {
    cur = make_pair(make_pair(EOL, vector_pop(stack)), EOL);
    pair_cdr(pre) = cur;
    pre = cur;
  }
  sexp rest = make_arguments(stack, nargs - n);
  pair_cdr(cur) = make_pair(make_pair(EOL, rest), EOL);
  /* *env = extend_environment(EOL, pair_cdr(bindings), *env); */
  *env = make_environment(pair_cdr(bindings), *env);
}

sexp top(sexp stack) {
  if (is_null(stack)) return EOL;
  return pair_car(stack);
}

sexp next_arg(sexp code_vector, int *index) {
  (*index)++;
  return vector_data_at(code_vector, *index);
}

/* Run the code generated from compiling an S-exp by function `assemble_code'. */
sexp run_compiled_code(sexp obj, sexp env, sexp stack) {
  assert(is_vector(stack));
  assert(is_compiled_proc(obj));
  sexp code = compiled_proc_code(obj);
  int nargs = 0;
  code = assemble_code(code);
  int pc = 0;
  while (pc < vector_length(code)) {
    assert(is_vector(code));
    sexp ins = vector_data_at(code, pc);
    /* port_format(scm_out_port, "Processing: %s\n", */
    /*             make_string(opcodes[code_name(ins)].name)); */
    switch (code_name(ins)) {
      /* Function call/return instructions */
      case ARGS: {
        sexp n = next_arg(code, &pc);
        if (nargs != fixnum_value(n)) {
          port_format(scm_out_port,
                      "Wrong argument number: %d but expecting %d\n",
                      n, make_fixnum(nargs));
          exit(1);
        }
        move_args(fixnum_value(n), stack, &env);
        pc++;
      } break;
      case ARGSD: {
        int n = fixnum_value(vector_data_at(code, ++pc));
        if (nargs < n) {
          port_format(scm_out_port, "Unscientific!\n");
          exit(1);
        }
        move_argsd(nargs, n, stack, &env);
        /* port_format(scm_out_port, "%*\n", environment_bindings(env)); */
        /* exit(0); */
        /* int top = vector_pos(stack); */
        /* printf("n is %d\ttop is %d\n", n, top); */
        /* vector_pos(stack) = top - n; */
        /* printf("vector_pos(stack) is %d\n", vector_pos(stack)); */
        /* sexp rest = make_arguments(stack, nargs - n); */
        /* /\* nth_pop(stack, (nargs - n)); *\/ */
        /* vector_data_at(stack, top - 1 - n) = rest; */
        /* vector_pos(stack) = top; */
        /* port_format(scm_out_port, "%*\n", stack); */
        /* exit(0); */
        /* env = extend_environment(EOL, EOL, env); */
        /* sexp bindings = environment_bindings(env); */
        /* push(make_pair(EOL, rest), bindings); */
        /* for (; n > 0; n--) { */
        /*   pop_to(stack, arg); */
        /*   push(make_pair(EOL, arg), bindings); */
        /* } */
        /* environment_bindings(env) = bindings; */
        pc++;
      } break;
      case CALLJ: {
        nargs = fixnum_value(vector_data_at(code, ++pc));
        pop_to(stack, proc);
        code = assemble_code(compiled_proc_code(proc));
        env = compiled_proc_env(proc);
        pc = 0;
      } break;
      case FN: {
        sexp fn = vector_data_at(code, ++pc);
        sexp pars = compiled_proc_args(fn);
        sexp code = compiled_proc_code(fn);
        vector_push(make_compiled_proc(pars, code, env), stack);
        pc++;
      } break;
      case MC: {
        sexp fn = vector_data_at(code, ++pc);
        sexp pars = macro_proc_pars(fn);
        sexp code = macro_proc_body(fn);
        vector_push(make_macro_procedure(pars, code, env), stack);
        pc++;
      } break;
      case PRIM: {
        pop_to(stack, op);
        sexp n = vector_data_at(code, ++pc);
        sexp args = make_arguments(stack, fixnum_value(n));
        /* nth_pop(stack, fixnum_value(n)); */
        port_format(scm_err_port, "This shouldn't happen!\n");
        exit(1);
        vector_push(eval_application(op, args), stack);
        pc++;
      } break;
      case PRIM0: {
        pop_to(stack, op);
        assert(is_primitive(op));
        vector_push((proc0(op))(), stack);
        pc++;
      } break;
      case PRIM1: {
        pop_to(stack, op);
        pop_to(stack, arg1);
        vector_push(proc1(op)(arg1), stack);
        pc++;
      } break;
      case PRIM2: {
        pop_to(stack, op);
        pop_to(stack, arg1);
        pop_to(stack, arg2);
        vector_push(proc2(op)(arg1, arg2), stack);
        pc++;
      } break;
      case PRIM3: {
        pop_to(stack, op);
        pop_to(stack, arg1);
        pop_to(stack, arg2);
        pop_to(stack, arg3);
        vector_push(proc3(op)(arg1, arg2, arg3), stack);
        pc++;
      } break;
      case RETURN: {                    /* No vector operations */
        pop_to(stack, value);
        if (is_return_info(vector_top(stack))) {
          /* Restores the stack-based machine context */
          pop_to(stack, info);
          code = return_code(info);
          env = return_env(info);
          pc = return_pc(info);
          vector_push(value, stack);
        } else {
          vector_push(value, stack);
          goto halt;
        }
        pc++;
      } break;
      case SAVE: {
        sexp l = next_arg(code, &pc);
        vector_push(make_return_info(code, fixnum_value(l), env), stack);
        pc++;
      } break;

        /* Variable/Stack manipulation instructions */
      case CONST: {
        sexp obj = next_arg(code, &pc);
        vector_push(obj, stack);
        pc++;
      } break;
      case GSET: {
        sexp value = vector_top(stack);
        sexp var = next_arg(code, &pc);
        set_binding(var, value, env);
        pc++;
      } break;
      case GVAR: {
        sexp var = next_arg(code, &pc);
        sexp value = get_variable_value(var, env);
        if (is_undefined(value)) {
          port_format(scm_out_port, "Unbound variable: %*\n", var);
          exit(1);
        }
        vector_push(value, stack);
        pc++;
      } break;
      case LSET: {
        int i = fixnum_value(next_arg(code, &pc));
        int j = fixnum_value(next_arg(code, &pc));
        set_variable_by_index(i, j, vector_top(stack), env);
        pc++;
      } break;
      case LVAR: {
        sexp i = next_arg(code, &pc);
        sexp j = next_arg(code, &pc);
        vector_push(get_variable_by_index(fixnum_value(i), fixnum_value(j), env), stack);
        pc++;
      } break;
      case POP: vector_pop(stack); pc++; break;

        /* Branching instructions */
      case FJUMP: {
        pop_to(stack, e);
        sexp l = next_arg(code, &pc);
        if (is_false(e)) pc = fixnum_value(l);
        else pc++;
      } break;
      case JUMP: pc = fixnum_value(next_arg(code, &pc)); break;
      case TJUMP: {
        pop_to(stack, e);
        pc++;
        if (is_true(e)) pc = fixnum_value(next_arg(code, &pc));
        else pc++;
      } break;

        /* Primitive functions */
      case CAR: {
        pop_to(stack, pair);
        vector_push(pair_car(pair), stack);
        pc++;
      } break;
      case CDR: {
        pop_to(stack, pair);
        vector_push(pair_cdr(pair), stack);
        pc++;
      } break;
        /* Integer arithmetic operations */
      case IADD: {
        pop_to(stack, n1);
        pop_to(stack, n2);
        vector_push(make_fixnum(fixnum_value(n1) + fixnum_value(n2)), stack);
        pc++;
      } break;
      case ISUB: {
        pop_to(stack, n1);
        pop_to(stack, n2);
        vector_push(make_fixnum(fixnum_value(n1) - fixnum_value(n2)), stack);
        pc++;
      } break;
      case IMUL: {
        pop_to(stack, n1);
        pop_to(stack, n2);
        vector_push(make_fixnum(fixnum_value(n1) * fixnum_value(n2)), stack);
        pc++;
      } break;
      case IDIV: {
        pop_to(stack, n1);
        pop_to(stack, n2);
        vector_push(make_fixnum(fixnum_value(n1) / fixnum_value(n2)), stack);
        pc++;
      } break;
      case EQ: {
        pop_to(stack, o2);
        pop_to(stack, o1);
        vector_push(o2 == o1 ? true_object: false_object, stack);
        pc++;
      } break;

      default :
        fprintf(stderr, "run_compiled_code - Unknown code ");
        /* write_object(pair_car(ins), make_file_out_port(stdout)); */
        port_format(scm_out_port, "%s",
                    make_string(opcodes[code_name(ins)].name));
        /* port_format(scm_out_port, "%*\n", env); */
        return stack;
    }
    /* port_format(scm_out_port, "stack: %*\n", stack); */
  }
halt:
  /* return vector_top(stack); */
  return vector_pop(stack);
}
