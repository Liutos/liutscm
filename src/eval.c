/*
 * eval.c
 *
 * Evaluator for S-exp
 *
 * Copyright (C) 2013-03-13 liutos <mat.liutos@gmail.com>
 */
#include "types.h"
#include "read.h"
#include <stdlib.h>

int is_quote_form(lisp_object_t object) {
  return is_pair(object) && find_or_create_symbol("quote") == pair_car(object);
}

lisp_object_t quotation_text(lisp_object_t quote_form) {
  return pair_cadr(quote_form);
}

lisp_object_t environment_vars(lisp_object_t environment) {
  return pair_caar(environment);
}

lisp_object_t environment_vals(lisp_object_t environment) {
  return pair_cdar(environment);
}

lisp_object_t enclosing_environment(lisp_object_t environment) {
  return pair_cdr(environment);
}

lisp_object_t make_undefined(void) {
  lisp_object_t undefined = malloc(sizeof(struct lisp_object_t));
  undefined->type = UNDEFINED;
  return undefined;
}

lisp_object_t get_variable_value(lisp_object_t var, lisp_object_t environment) {
  while (is_pair(environment)) {
    lisp_object_t vars = environment_vars(environment);
    lisp_object_t vals = environment_vals(environment);
    while (is_pair(vars)) {
      if (pair_car(vars) == var)
        return pair_cdr(vals);
      vars = pair_cdr(vars);
      vals = pair_cdr(vals);
    }
    environment = enclosing_environment(environment);
  }
  return make_undefined();
}

int is_variable(lisp_object_t object) {
  return is_symbol(object);
}

lisp_object_t eval_object(lisp_object_t object, lisp_object_t environment) {
  if (is_quote_form(object))
    return quotation_text(object);
  if (is_variable(object))
    return get_variable_value(object, environment);
  else
    return object;
}
