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

lisp_object_t make_pair(lisp_object_t, lisp_object_t);

int is_tag_list(lisp_object_t object, char *symbol_name) {
  return is_pair(object) && find_or_create_symbol(symbol_name) == pair_car(object);
}

int is_quote_form(lisp_object_t object) {
  return is_pair(object) && find_or_create_symbol("quote") == pair_car(object);
}

lisp_object_t quotation_text(lisp_object_t quote_form) {
  return pair_cadr(quote_form);
}

/* lisp_object_t environment_vars(lisp_object_t environment) { */
/*   return pair_caar(environment); */
/* } */
#define environment_vars(x) pair_caar(x)
/* lisp_object_t environment_vals(lisp_object_t environment) { */
/*   return pair_cdar(environment); */
/* } */
#define environment_vals(x) pair_cdar(x)
/* lisp_object_t enclosing_environment(lisp_object_t environment) { */
/*   return pair_cdr(environment); */
/* } */
#define enclosing_environment(x) pair_cdr(x)

lisp_object_t make_undefined(void) {
  lisp_object_t undefined = malloc(sizeof(struct lisp_object_t));
  undefined->type = UNDEFINED;
  return undefined;
}

lisp_object_t make_empty_list(void);

lisp_object_t make_empty_environment(void) {
  return make_empty_list();
}

lisp_object_t make_startup_environment(void) {
  lisp_object_t vars = make_empty_list();
  lisp_object_t vals = make_empty_list();
  return make_pair(make_pair(vars, vals), make_empty_list());
}

int is_empty_environment(lisp_object_t env) {
  return is_null(env);
}

lisp_object_t search_binding(lisp_object_t var, lisp_object_t env) {
  while (!is_empty_environment(env)) {
    lisp_object_t vars = environment_vars(env);
    lisp_object_t vals = environment_vals(env);
    while (is_pair(vars)) {
      if (pair_car(vars) == var)
        return make_pair(pair_car(vars), pair_car(vals)); /* pair as binding */
      vars = pair_cdr(vars);
      vals = pair_cdr(vals);
    }
    env = enclosing_environment(env);
  }
  return NULL;
}

lisp_object_t get_variable_value(lisp_object_t var, lisp_object_t environment) {
  /* while (is_pair(environment)) { */
  /*   lisp_object_t vars = environment_vars(environment); */
  /*   lisp_object_t vals = environment_vals(environment); */
  /*   while (is_pair(vars)) { */
  /*     if (pair_car(vars) == var) */
  /*       return pair_car(vals); */
  /*     vars = pair_cdr(vars); */
  /*     vals = pair_cdr(vals); */
  /*   } */
  /*   environment = enclosing_environment(environment); */
  /* } */
  /* return make_undefined(); */
  lisp_object_t cell = search_binding(var, environment);
  if (cell)
    return pair_cdr(cell);
  else
    return make_undefined();
}

int is_variable(lisp_object_t object) {
  return is_symbol(object);
}

int is_define_form(lisp_object_t object) {
  return is_tag_list(object, "define");
}

lisp_object_t definition_variable(lisp_object_t define_form) {
  return pair_cadr(define_form);
}

lisp_object_t definition_value(lisp_object_t define_form) {
  return pair_caddr(define_form);
}

void add_binding(lisp_object_t var, lisp_object_t val, lisp_object_t environment) {
  lisp_object_t cell = search_binding(var, environment);
  if (!cell) {
    lisp_object_t vars = environment_vars(environment);
    lisp_object_t vals = environment_vals(environment);
    environment_vars(environment) = make_pair(var, vars);
    environment_vals(environment) = make_pair(val, vals);
  }
}

int is_assignment_form(lisp_object_t object) {
  return is_tag_list(object, "set!");
}

lisp_object_t assignment_variable(lisp_object_t assignment_form) {
  return pair_cadr(assignment_form);
}

lisp_object_t assignment_value(lisp_object_t assignment_form) {
  return pair_caddr(assignment_form);
}

void set_binding(lisp_object_t var, lisp_object_t val, lisp_object_t environment) {
  lisp_object_t tmp = environment;
  while (!is_empty_environment(environment)) {
    lisp_object_t vars = environment_vars(environment);
    lisp_object_t vals = environment_vals(environment);
    while (is_pair(vars)) {
      if (pair_car(vars) == var) {
        pair_car(vals) = val;
        break;
      }
      vars = pair_cdr(vars);
      vals = pair_cdr(vals);
    }
    environment = enclosing_environment(environment);
  }
  add_binding(var, val, tmp);
}

int is_if_form(lisp_object_t object) {
  return is_tag_list(object, "if");
}

lisp_object_t if_test_part(lisp_object_t if_form) {
  return pair_cadr(if_form);
}

lisp_object_t if_then_part(lisp_object_t if_form) {
  return pair_caddr(if_form);
}

lisp_object_t if_else_part(lisp_object_t if_form) {
  lisp_object_t alt = pair_cdddr(if_form);
  if (is_null(alt))
    return make_empty_list();
  else
    return pair_car(alt);
}

lisp_object_t eval_object(lisp_object_t object, lisp_object_t environment) {
  if (is_quote_form(object))
    return quotation_text(object);
  if (is_variable(object))
    return get_variable_value(object, environment);
  if (is_define_form(object)) {
    lisp_object_t value = eval_object(definition_value(object), environment);
    add_binding(definition_variable(object), value, environment);
    return make_undefined();
  }
  if (is_assignment_form(object)) {
    lisp_object_t value = eval_object(assignment_value(object), environment);
    set_binding(assignment_variable(object), value, environment);
    return make_undefined();
  }
  if (is_if_form(object)) {
    lisp_object_t test_part = if_test_part(object);
    lisp_object_t then_part = if_then_part(object);
    lisp_object_t else_part = if_else_part(object);
    if (eval_object(test_part, environment) == make_true())
      return eval_object(then_part, environment);
    else
      return eval_object(else_part, environment);
  }
  else
    return object;
}
