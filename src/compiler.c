/*
 * compiler.c
 *
 * Compiler compile the S-exp to stack-based virtual machine
 *
 * Copyright (C) 2013-03-18 liutos <mat.liutos@gmail.com>
 */
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <stdlib.h>

#include "eval.h"
#include "types.h"
#include "object.h"

#define BUFFER_SIZE 10

#define seq(...) sequenzie(__VA_ARGS__, NULL)
#define gen(...) generate_code(__VA_ARGS__, NULL)
#define gen_args(x) gen("ARGS", x)
#define gen_call(x) gen("CALL", x)
#define gen_const(x) gen("CONST", x)
#define gen_fjump(x) gen("FJUMP", x)
#define gen_fn(x) gen("FN", x)
#define gen_gset(x) gen("GSET", x)
#define gen_gvar(x) gen("GVAR", x)
#define gen_jump(x) gen("JUMP", x)
#define gen_lset(i, j) gen("LSET", i, j)
#define gen_lvar(i, j) gen("LVAR", i, j)
#define gen_pop() gen("POP")
#define gen_return() gen("RETURN")

sexp compile_object(sexp, sexp);

int label_counter = 0;

int is_variable_found(sexp var, sexp env, int *i, int *j) {
  return search_binding_index(var, env, i, j);
}

lisp_object_t va_list2pair(va_list ap) {
  lisp_object_t o = va_arg(ap, lisp_object_t);
  if (o)
    return make_pair(o, va_list2pair(ap));
  else
    return make_empty_list();
}

lisp_object_t make_list1(lisp_object_t x) {
  return make_pair(x, EOL);
}

lisp_object_t make_label(void) {
  static char buffer[BUFFER_SIZE];
  int n = sprintf(buffer, "L%d", label_counter);
  label_counter++;
  return find_or_create_symbol(strndup(buffer, n));
}

/* Generate a list contains one instruction */
lisp_object_t generate_code(char *code_name, ...) {
  va_list ap;
  va_start(ap, code_name);
  return make_list1(make_pair(find_or_create_symbol(code_name),
                              va_list2pair(ap)));
}

lisp_object_t sequenzie_aux(va_list ap) {
  lisp_object_t e = va_arg(ap, lisp_object_t);
  if (NULL == e) {
    va_end(ap);
    return make_empty_list();
  } else
    return nconc_pair(e, sequenzie_aux(ap));
}

/* Concatenates a series of list of instructions */
lisp_object_t sequenzie(lisp_object_t pair, ...) {
  va_list ap;
  va_start(ap, pair);
  return nconc_pair(pair, sequenzie_aux(ap));
}

/* Compiler */
lisp_object_t compile_constant(lisp_object_t val) {
  return gen_const(val);
}

lisp_object_t compile_set(lisp_object_t var, lisp_object_t environment) {
  int i, j;
  if (!is_variable_found(var, environment, &i, &j))
    return gen_gset(var);
  else
    return gen_lset(make_fixnum(i), make_fixnum(j));
}

sexp compile_begin(sexp actions, sexp environment) {
  if (is_null(actions))
    return compile_constant(make_empty_list());
  if (is_null(pair_cdr(actions)))
    return compile_object(pair_car(actions), environment);
  else
    return seq(compile_object(pair_car(actions), environment),
               gen_pop(),
               compile_begin(pair_cdr(actions), environment));
}

sexp compile_lambda(sexp args, sexp body, sexp env) {
  sexp new_env = extend_environment(args, make_empty_list(), env);
  sexp code =
      seq(gen_args(make_fixnum(pair_length(args))),
          compile_begin(body, new_env),
          gen_return());
  return make_compiled_proc(args, code, new_env);
}

sexp compile_arguments(sexp args, sexp environment) {
  if (is_null(args))
    return make_empty_list();
  else
    return seq(compile_object(pair_car(args), environment),
               compile_arguments(pair_cdr(args), environment));
}

sexp compile_var(sexp object, sexp env) {
  int i, j;
  if (!is_variable_found(object, env, &i, &j))
    return gen_gvar(object);
  else
    return gen_lvar(make_fixnum(i), make_fixnum(j));
}

sexp compile_assignment(sexp object, sexp env) {
  sexp value = compile_object(assignment_value(object), env);
  return seq(value, compile_set(assignment_variable(object), env));
}

sexp compile_if(sexp object, sexp env) {
  sexp l1 = make_label();
  sexp l2 = make_label();
  return seq(compile_object(if_test_part(object), env),
             gen_fjump(l1),
             compile_object(if_then_part(object), env),
             gen_jump(l2),
             make_list1(l1),
             compile_object(if_else_part(object), env),
             make_list1(l2));
}

sexp compile_application(sexp object, sexp env) {
  int length = pair_length(application_operands(object));
  return seq(compile_arguments(application_operands(object), env),
             compile_object(application_operator(object), env),
             gen_call(make_fixnum(length)));
}

/* Generate a list of instructions based-on a stack-based virtual machine. */
sexp compile_object(sexp object, sexp env) {
  if (is_variable_form(object)) {
    return compile_var(object, env);
  }
  if (is_quote_form(object)) {
    return compile_constant(quotation_text(object));
  }
  if (is_assignment_form(object)) {
    return compile_assignment(object, env);
  }
  if (is_if_form(object)) {
    return compile_if(object, env);
  }
  if (is_begin_form(object)) {
    return compile_begin(begin_actions(object), env);
  }
  if (is_lambda_form(object)) {
    sexp args = lambda_parameters(object);
    sexp body = lambda_body(object);
    return gen_fn(compile_lambda(args, body, env));
  }
  if (is_application_form(object)) {
    return compile_application(object, env);
  }
  return compile_constant(object);
}
