/*
 * compile.c
 *
 *
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

lisp_object_t compile_object(lisp_object_t, lisp_object_t);

int label_counter = 0;

lisp_object_t is_variable_found(lisp_object_t var, lisp_object_t environment) {
  return search_binding_index(var, environment);
}

lisp_object_t pair_conc(lisp_object_t pair1, lisp_object_t pair2) {
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

lisp_object_t generate_code(char *name, lisp_object_t args) {
  return make_list(make_pair(find_or_create_symbol(name), args), NULL);
}

lisp_object_t compile_constant(lisp_object_t val) {
  return make_list(make_pair(find_or_create_symbol("CONST"), val), NULL);
}

lisp_object_t compile_set(lisp_object_t var, lisp_object_t environment) {
  lisp_object_t co = is_variable_found(var, environment);
  if (NULL == co)
    return make_list(make_pair(find_or_create_symbol("GSET"), var), NULL);
  else
    return make_list(make_list(find_or_create_symbol("LSET"), pair_car(co), pair_cdr(co), NULL), NULL);
}

lisp_object_t make_label(void) {
#define BUFFER_SIZE 10
  static char buffer[BUFFER_SIZE];
  int n = sprintf(buffer, "L%d", label_counter);
  label_counter++;
  return find_or_create_symbol(strndup(buffer, n));
}

int pair_length(lisp_object_t pair) {
  if (is_null(pair))
    return 0;
  else
    return 1 + pair_length(pair_cdr(pair));
}

lisp_object_t compile_begin(lisp_object_t actions, lisp_object_t environment) {
  if (is_null(actions))
    return compile_constant(make_empty_list());
  if (is_null(pair_cdr(actions)))
    return compile_object(pair_car(actions), environment);
  else
    return sequenzie(compile_object(pair_car(actions), environment),
                     make_list(make_pair(find_or_create_symbol("POP"), make_empty_list()), NULL),
                     compile_begin(pair_cdr(actions), environment),
                     NULL);
}

lisp_object_t make_compiled_proc(lisp_object_t args, lisp_object_t code, lisp_object_t env) {
  lisp_object_t proc = malloc(sizeof(struct lisp_object_t));
  proc->type = COMPILED_PROC;
  compiled_proc_args(proc) = args;
  compiled_proc_env(proc) = env;
  compiled_proc_code(proc) = code;
  return proc;
}

lisp_object_t compile_lambda(lisp_object_t args, lisp_object_t body, lisp_object_t env) {
  lisp_object_t new_env = extend_environment(args, make_empty_list(), env);
  lisp_object_t code =
      sequenzie(generate_code("ARGS", make_fixnum(pair_length(args))),
                compile_begin(body, new_env),
                generate_code("RETURN", make_empty_list()),
                NULL);
  return make_compiled_proc(args, code, env);
}

lisp_object_t compile_arguments(lisp_object_t args, lisp_object_t environment) {
  if (is_null(args))
    return make_empty_list();
  else
    return pair_conc(compile_object(pair_car(args), environment),
                     compile_arguments(pair_cdr(args), environment));
}

/* Generate a list of instructions based-on a stack-based virtual machine. */
lisp_object_t compile_object(lisp_object_t object, lisp_object_t environment) {
  if (is_variable_form(object)) {
    lisp_object_t co = is_variable_found(object, environment);
    if (NULL == co)
      return make_list(make_pair(find_or_create_symbol("GVAR"), object), NULL);
    else
      return make_list(make_list(find_or_create_symbol("LVAR"), pair_car(co), pair_cdr(co), NULL), NULL);
  }
  if (is_quote_form(object)) {
    return make_list(make_pair(find_or_create_symbol("CONST"), quotation_text(object)), NULL);
  }
  if (is_assignment_form(object)) {
    lisp_object_t value = compile_object(assignment_value(object), environment);
    return pair_conc(value, compile_set(assignment_variable(object), environment));
  }
  if (is_if_form(object)) {
    lisp_object_t l1 = make_label();
    lisp_object_t l2 = make_label();
    return sequenzie(compile_object(if_test_part(object), environment),
                     generate_code("FJUMP", l1),
                     compile_object(if_then_part(object), environment),
                     generate_code("JUMP", l2),
                     make_list(l1, NULL),
                     compile_object(if_else_part(object), environment),
                     make_list(l2, NULL),
                     NULL);
  }
  if (is_begin_form(object)) {
    return compile_begin(begin_actions(object), environment);
  }
  if (is_lambda_form(object)) {
    lisp_object_t args = lambda_parameters(object);
    lisp_object_t body = lambda_body(object);
    return generate_code("FN", compile_lambda(args, body, environment));
  }
  if (is_application_form(object)) {
    int length = pair_length(application_operands(object));
    return sequenzie(compile_arguments(application_operands(object), environment),
                     compile_object(application_operator(object), environment),
                     generate_code("CALL", make_fixnum(length)),
                     NULL);
  }
  return make_list(make_pair(find_or_create_symbol("CONST"), object), NULL);
}
