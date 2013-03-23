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

#include "types.h"
#include "object.h"
#include "eval.h"

sexp compile_object(sexp, sexp);

int label_counter = 0;

lisp_object_t is_variable_found(lisp_object_t var, lisp_object_t environment) {
  return search_binding_index(var, environment);
}

lisp_object_t pair_conc(lisp_object_t pair1, lisp_object_t pair2) {
  if (is_null(pair1)) return pair2;
  if (is_null(pair2)) return pair1;
  lisp_object_t tmp = pair1;
  while (!is_null(pair_cdr(tmp)))
    tmp = pair_cdr(tmp);
  pair_cdr(tmp) = pair2;
  return pair1;
}

lisp_object_t sequenzie_aux(va_list ap) {
  lisp_object_t e = va_arg(ap, lisp_object_t);
  if (NULL == e) {
    va_end(ap);
    return make_empty_list();
  } else
    return pair_conc(e, sequenzie_aux(ap));
}

lisp_object_t sequenzie(lisp_object_t pair, ...) {
  va_list ap;
  va_start(ap, pair);
  return pair_conc(pair, sequenzie_aux(ap));
}

#define seq(...) sequenzie(__VA_ARGS__, NULL)

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

lisp_object_t generate_code(char *code_name, ...) {
  va_list ap;
  va_start(ap, code_name);
  return make_list1(make_pair(find_or_create_symbol(code_name),
                              va_list2pair(ap)));
}

#define gen(...) generate_code(__VA_ARGS__, NULL)
#define gen_const(x) gen("CONST", x)

lisp_object_t compile_constant(lisp_object_t val) {
  return gen_const(val);
}

#define gen_gset(x) gen("GSET", x)
#define gen_lset(i, j) gen("LSET", i, j)

lisp_object_t compile_set(lisp_object_t var, lisp_object_t environment) {
  lisp_object_t co = is_variable_found(var, environment);
  if (NULL == co)
    return gen_gset(var);
  else
    return gen_lset(pair_car(co), pair_cdr(co));
}

lisp_object_t make_label(void) {
#define BUFFER_SIZE 10
  static char buffer[BUFFER_SIZE];
  int n = sprintf(buffer, "L%d", label_counter);
  label_counter++;
  return find_or_create_symbol(strndup(buffer, n));
}

#define gen_pop() gen("POP")

lisp_object_t compile_begin(lisp_object_t actions, lisp_object_t environment) {
  if (is_null(actions))
    return compile_constant(make_empty_list());
  if (is_null(pair_cdr(actions)))
    return compile_object(pair_car(actions), environment);
  else
    return seq(compile_object(pair_car(actions), environment),
               gen_pop(),
               compile_begin(pair_cdr(actions), environment));
}

lisp_object_t make_compiled_proc(lisp_object_t args, lisp_object_t code, lisp_object_t env) {
  lisp_object_t proc = malloc(sizeof(struct lisp_object_t));
  proc->type = COMPILED_PROC;
  compiled_proc_args(proc) = args;
  compiled_proc_env(proc) = env;
  compiled_proc_code(proc) = code;
  return proc;
}

#define gen_args(x) gen("ARGS", x)
#define gen_return() gen("RETURN")

lisp_object_t compile_lambda(lisp_object_t args, lisp_object_t body, lisp_object_t env) {
  lisp_object_t new_env = extend_environment(args, make_empty_list(), env);
  lisp_object_t code =
      seq(gen_args(make_fixnum(pair_length(args))),
          compile_begin(body, new_env),
          gen_return());
  return make_compiled_proc(args, code, new_env);
}

lisp_object_t compile_arguments(lisp_object_t args, lisp_object_t environment) {
  if (is_null(args))
    return make_empty_list();
  else
    return pair_conc(compile_object(pair_car(args), environment),
                     compile_arguments(pair_cdr(args), environment));
}

#define gen_gvar(x) gen("GVAR", x)
#define gen_lvar(i, j) gen("LVAR", i, j)
#define gen_fjump(x) gen("FJUMP", x)
#define gen_jump(x) gen("JUMP", x)
#define gen_fn(x) gen("FN", x)
#define gen_call(x) gen("CALL", x)

/* Generate a list of instructions based-on a stack-based virtual machine. */
sexp compile_object(sexp object, sexp environment) {
  if (is_variable_form(object)) {
    lisp_object_t co = is_variable_found(object, environment);
    if (NULL == co)
      return gen_gvar(object);
    else
      return gen_lvar(pair_car(co), pair_cdr(co));
  }
  if (is_quote_form(object)) {
    return compile_constant(quotation_text(object));
  }
  if (is_assignment_form(object)) {
    lisp_object_t value = compile_object(assignment_value(object), environment);
    return pair_conc(value, compile_set(assignment_variable(object), environment));
  }
  if (is_if_form(object)) {
    lisp_object_t l1 = make_label();
    lisp_object_t l2 = make_label();
    return seq(compile_object(if_test_part(object), environment),
               gen_fjump(l1),
               compile_object(if_then_part(object), environment),
               gen_jump(l2),
               make_list1(l1),
               compile_object(if_else_part(object), environment),
               make_list1(l2));
  }
  if (is_begin_form(object)) {
    return compile_begin(begin_actions(object), environment);
  }
  if (is_lambda_form(object)) {
    lisp_object_t args = lambda_parameters(object);
    lisp_object_t body = lambda_body(object);
    return gen_fn(compile_lambda(args, body, environment));
  }
  if (is_application_form(object)) {
    int length = pair_length(application_operands(object));
    return seq(compile_arguments(application_operands(object), environment),
               compile_object(application_operator(object), environment),
               gen_call(make_fixnum(length)));
  }
  return compile_constant(object);
}
