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
#include <stdio.h>

extern lisp_object_t make_pair(lisp_object_t, lisp_object_t);
extern lisp_object_t make_character(char);
extern lisp_object_t make_string(char *);

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

int is_application_form(lisp_object_t object) {
  return is_pair(object);
}

lisp_object_t application_operator(lisp_object_t application_form) {
  return pair_car(application_form);
}

lisp_object_t application_operands(lisp_object_t application_form) {
  return pair_cdr(application_form);
}

lisp_object_t eval_object(lisp_object_t, lisp_object_t);

lisp_object_t eval_arguments(lisp_object_t arguments, lisp_object_t environment) {
  if (is_null(arguments))
    return make_empty_list();
  else {
    lisp_object_t first = pair_car(arguments);
    return make_pair(eval_object(first, environment),
                     eval_arguments(pair_cdr(arguments), environment));
  }
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
  if (is_application_form(object)) {
    lisp_object_t operator = application_operator(object);
    lisp_object_t operands = application_operands(object);
    operator = eval_object(operator, environment);
    operands = eval_arguments(operands, environment);
    return (primitive_C_proc(operator))(operands);
  }
  else
    return object;
}

extern lisp_object_t make_fixnum(int);

/* Binary plus */
lisp_object_t plus_proc(lisp_object_t args) {
  lisp_object_t n1 = pair_car(args);
  lisp_object_t n2 = pair_cadr(args);
  return make_fixnum(fixnum_value(n1) + fixnum_value(n2));
}

/* Binary minus */
lisp_object_t minus_proc(lisp_object_t args) {
  lisp_object_t n1 = pair_car(args);
  lisp_object_t n2 = pair_cadr(args);
  return make_fixnum(fixnum_value(n1) - fixnum_value(n2));
}

/* Binary multiply */
lisp_object_t multiply_proc(lisp_object_t args) {
  lisp_object_t n1 = pair_car(args);
  lisp_object_t n2 = pair_cadr(args);
  return make_fixnum(fixnum_value(n1) * fixnum_value(n2));
}

/* Binary divide */
lisp_object_t divide_proc(lisp_object_t args) {
  lisp_object_t n1 = pair_car(args);
  lisp_object_t n2 = pair_cadr(args);
  return make_fixnum(fixnum_value(n1) / fixnum_value(n2));
}

/* Binary equal */
lisp_object_t numeric_equal_proc(lisp_object_t args) {
  lisp_object_t n1 = pair_car(args);
  lisp_object_t n2 = pair_cadr(args);
  return fixnum_value(n1) == fixnum_value(n2) ? make_true(): make_false();
}

lisp_object_t mod_proc(lisp_object_t args) {
  lisp_object_t n1 = pair_car(args);
  lisp_object_t n2 = pair_cadr(args);
  return make_fixnum(fixnum_value(n1) % fixnum_value(n2));
}

lisp_object_t greater_than_proc(lisp_object_t args) {
  lisp_object_t n1 = pair_car(args);
  lisp_object_t n2 = pair_cadr(args);
  return fixnum_value(n1) > fixnum_value(n2) ? make_true(): make_false();
}

/* lisp_object_t less_than_proc(lisp_object_t args) { */
/*   lisp_object_t n1 = pair_car(args); */
/*   lisp_object_t n2 = pair_cadr(args); */
/*   return fixnum_value(n1) < fixnum_value(n2) ? make_true(): make_false(); */
/* } */

/* Are the two arguments identical? */
lisp_object_t is_identical_proc(lisp_object_t args) {
  lisp_object_t o1 = pair_car(args);
  lisp_object_t o2 = pair_cadr(args);
  return o1 == o2 ? make_true(): make_false();
}

/* Get the encode of a character */
lisp_object_t char2code_proc(lisp_object_t args) {
  lisp_object_t c = pair_car(args);
  return make_fixnum(char_value(c));
}

/* Return a character with given encode */
lisp_object_t code2char_proc(lisp_object_t args) {
  lisp_object_t n = pair_car(args);
  return make_character(fixnum_value(n));
}

/* Get the specific character in a string */
lisp_object_t char_at_proc(lisp_object_t args) {
  lisp_object_t n = pair_car(args);
  lisp_object_t str = pair_cadr(args);
  return make_character(string_value(str)[fixnum_value(n)]);
}

lisp_object_t pair_car_proc(lisp_object_t args) {
  lisp_object_t list = pair_car(args);
  return pair_car(list);
}

lisp_object_t pair_cdr_proc(lisp_object_t args) {
  lisp_object_t list = pair_car(args);
  return pair_cdr(list);
}

lisp_object_t pair_set_car_proc(lisp_object_t args) {
  lisp_object_t pair = pair_car(args);
  lisp_object_t val = pair_cadr(args);
  pair_car(pair) = val;
  return make_undefined();
}

lisp_object_t pair_set_cdr_proc(lisp_object_t args) {
  lisp_object_t pair = pair_car(args);
  lisp_object_t val = pair_cadr(args);
  pair_cdr(pair) = val;
  return make_undefined();
}

/* Construct a pair by two arguments */
lisp_object_t pair_cons_proc(lisp_object_t args) {
  lisp_object_t o1 = pair_car(args);
  lisp_object_t o2 = pair_cadr(args);
  return make_pair(o1, o2);
}

lisp_object_t symbol_name_proc(lisp_object_t args) {
  lisp_object_t sym = pair_car(args);
  return make_string(symbol_name(sym));
}

/* Create a symbol looks the same as the string argument */
lisp_object_t string2symbol_proc(lisp_object_t args) {
  lisp_object_t str = pair_car(args);
  return find_or_create_symbol(string_value(str));
}

/* Return a symbol indicates the argument's type */
lisp_object_t type_of_proc(lisp_object_t args) {
  lisp_object_t o = pair_car(args);
  switch (o->type) {
    case FIXNUM: return find_or_create_symbol("fixnum");
    case BOOLEAN: return find_or_create_symbol("boolean");
    case CHARACTER: return find_or_create_symbol("character");
    case STRING: return find_or_create_symbol("string");
    case EMPTY_LIST: return find_or_create_symbol("empty_list");
    case PAIR: return find_or_create_symbol("pair");
    case SYMBOL: return find_or_create_symbol("symbol");
    case PRIMITIVE_PROC: return find_or_create_symbol("function");
    default :
      fprintf(stderr, "Unknown data type: %d\n", o->type);
      exit(1);
  }
}

lisp_object_t make_primitive_proc(lisp_object_t (*C_proc)(lisp_object_t)) {
  lisp_object_t proc = malloc(sizeof(struct lisp_object_t));
  proc->type = PRIMITIVE_PROC;
  proc->values.primitive_proc.C_proc = C_proc;
  return proc;
}

void add_primitive_proc(char *Lisp_name, lisp_object_t (*C_proc)(lisp_object_t), lisp_object_t environment) {
  lisp_object_t proc = make_primitive_proc(C_proc);
  lisp_object_t var = find_or_create_symbol(Lisp_name);
  add_binding(var, proc, environment);
}

void init_environment(lisp_object_t environment) {
  add_primitive_proc("+", plus_proc, environment);
  add_primitive_proc("-", minus_proc, environment);
  add_primitive_proc("*", multiply_proc, environment);
  add_primitive_proc("/", divide_proc, environment);
  add_primitive_proc("=", numeric_equal_proc, environment);
  add_primitive_proc("%", mod_proc, environment);
  add_primitive_proc(">", greater_than_proc, environment);
  /* add_primitive_proc("<", less_than_proc, environment); */
  add_primitive_proc("eq", is_identical_proc, environment);
  add_primitive_proc("char->code", char2code_proc, environment);
  add_primitive_proc("code->char", code2char_proc, environment);
  add_primitive_proc("char-at", char_at_proc, environment);
  add_primitive_proc("car", pair_car_proc, environment);
  add_primitive_proc("cdr", pair_cdr_proc, environment);
  add_primitive_proc("cons", pair_cons_proc, environment);
  add_primitive_proc("symbol-name", symbol_name_proc, environment);
  add_primitive_proc("string->symbol", string2symbol_proc, environment);
  add_primitive_proc("type-of", type_of_proc, environment);
  add_primitive_proc("set-car!", pair_set_car_proc, environment);
  add_primitive_proc("set-cdr!", pair_set_cdr_proc, environment);
}
