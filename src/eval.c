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
extern void write_object(lisp_object_t);

lisp_object_t eval_object(lisp_object_t, lisp_object_t);

int is_tag_list(lisp_object_t object, char *symbol_name) {
  return is_pair(object) && find_or_create_symbol(symbol_name) == pair_car(object);
}

int is_quote_form(lisp_object_t object) {
  return is_pair(object) && find_or_create_symbol("quote") == pair_car(object);
}

lisp_object_t quotation_text(lisp_object_t quote_form) {
  return pair_cadr(quote_form);
}

#define environment_vars(x) pair_caar(x)
#define environment_vals(x) pair_cdar(x)
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

/* int is_primitive_form(lisp_object_t object, lisp_object_t environment) { */
/*   if (is_application_form(object)) { */
/*     lisp_object_t op = eval_object(application_operator(object), environment); */
/*     return is_primitive(op); */
/*   } else */
/*     return 0; */
/* } */

lisp_object_t eval_arguments(lisp_object_t arguments, lisp_object_t environment) {
  if (is_null(arguments))
    return make_empty_list();
  else {
    lisp_object_t first = pair_car(arguments);
    return make_pair(eval_object(first, environment),
                     eval_arguments(pair_cdr(arguments), environment));
  }
}

/* LAMBDA support */

int is_lambda_form(lisp_object_t object) {
  return is_tag_list(object, "lambda");
}

lisp_object_t lambda_parameters(lisp_object_t lambda_form) {
  return pair_cadr(lambda_form);
}

lisp_object_t lambda_body(lisp_object_t lambda_form) {
  return pair_cddr(lambda_form);
}

lisp_object_t make_lambda_procedure(lisp_object_t parameters, lisp_object_t body, lisp_object_t environment) {
  lisp_object_t proc = malloc(sizeof(struct lisp_object_t));
  proc->type = COMPOUND_PROC;
  compound_proc_parameters(proc) = parameters;
  compound_proc_body(proc) = body;
  compound_proc_environment(proc) = environment;
  return proc;
}

/* int is_compound_form(lisp_object_t object, lisp_object_t environment) { */
/*   if (is_application_form(object)) { */
/*     lisp_object_t op = eval_object(application_operator(object), environment); */
/*     return is_compound(op); */
/*   } else */
/*     return 0; */
/* } */

lisp_object_t extend_environment(lisp_object_t vars, lisp_object_t vals, lisp_object_t environment) {
  return make_pair(make_pair(vars, vals), environment);
}

/* BEGIN support */

int is_begin_form(lisp_object_t object) {
  return is_tag_list(object, "begin");
}

lisp_object_t begin_actions(lisp_object_t begin_form) {
  return pair_cdr(begin_form);
}

/* COND support */

int is_cond_form(lisp_object_t object) {
  return is_tag_list(object, "cond");
}

lisp_object_t cond_clauses(lisp_object_t cond_form) {
  return pair_cdr(cond_form);
}

lisp_object_t clause_test(lisp_object_t clause) {
  return pair_car(clause);
}

lisp_object_t clause_actions(lisp_object_t clause) {
  return make_pair(find_or_create_symbol("begin"),
                   pair_cdr(clause));
}

int is_cond_else_clause(lisp_object_t clause) {
  return find_or_create_symbol("else") == clause_test(clause);
}

lisp_object_t make_if_form(lisp_object_t test, lisp_object_t then_part, lisp_object_t else_part) {
  return make_pair(find_or_create_symbol("if"),
                   make_pair(test,
                             make_pair(then_part,
                                       make_pair(else_part, make_empty_list()))));
}

lisp_object_t expand_cond_clauses(lisp_object_t clauses) {
  if (is_null(clauses))
    return make_empty_list();
  lisp_object_t first = pair_car(clauses);
  lisp_object_t rest = pair_cdr(clauses);
  if (is_cond_else_clause(first))
    return clause_actions(first);
  else
    return make_if_form(clause_test(first),
                        clause_actions(first),
                        expand_cond_clauses(rest));
}

lisp_object_t cond2if(lisp_object_t cond_form) {
  return expand_cond_clauses(cond_clauses(cond_form));
}

/* LET support */

int is_let_form(lisp_object_t object) {
  return is_tag_list(object, "let");
}

lisp_object_t let_vars_aux(lisp_object_t bindings) {
  if (is_null(bindings))
    return make_empty_list();
  else {
    lisp_object_t first = pair_car(bindings);
    return make_pair(pair_car(first), let_vars_aux(pair_cdr(bindings)));
  }
}

lisp_object_t let_vars(lisp_object_t let_form) {
  lisp_object_t bindings = pair_cadr(let_form);
  return let_vars_aux(bindings);
}

lisp_object_t let_vals_aux(lisp_object_t bindings) {
  if (is_null(bindings))
    return make_empty_list();
  else {
    lisp_object_t first = pair_car(bindings);
    return make_pair(pair_cadr(first), let_vals_aux(pair_cdr(bindings)));
  }
}

lisp_object_t let_vals(lisp_object_t let_form) {
  lisp_object_t bindings = pair_cadr(let_form);
  return let_vals_aux(bindings);
}

lisp_object_t let_body(lisp_object_t let_form) {
  return pair_cddr(let_form);
}

lisp_object_t make_lambda_form(lisp_object_t vars, lisp_object_t body) {
  return make_pair(find_or_create_symbol("lambda"),
                   make_pair(vars, body));
}

lisp_object_t let2lambda(lisp_object_t let_form) {
  lisp_object_t vars = let_vars(let_form);
  lisp_object_t vals = let_vals(let_form);
  lisp_object_t body = let_body(let_form);
  lisp_object_t lambda_form = make_lambda_form(vars, body);
  return make_pair(lambda_form, vals);
}

/* AND and OR support */

int is_and_form(lisp_object_t object) {
  return is_tag_list(object, "and");
}

lisp_object_t and_tests(lisp_object_t and_form) {
  return pair_cdr(and_form);
}

int is_or_form(lisp_object_t object) {
  return is_tag_list(object, "or");
}

lisp_object_t or_tests(lisp_object_t or_form) {
  return pair_cdr(or_form);
}

lisp_object_t eval_object(lisp_object_t object, lisp_object_t environment) {
tail_loop:
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
    if (eval_object(test_part, environment) == make_true()) {
      /* return eval_object(then_part, environment); */
      object = then_part;
      goto tail_loop;
    } else {
      /* return eval_object(else_part, environment); */
      object = else_part;
      goto tail_loop;
    }
  }
  if (is_lambda_form(object)) {
    lisp_object_t parameters = lambda_parameters(object);
    lisp_object_t body = lambda_body(object);
    return make_lambda_procedure(parameters, body, environment);
  }
  if (is_begin_form(object)) {
    lisp_object_t actions = begin_actions(object);
    if (is_null(actions))
      return make_empty_list();
    while (!is_null(pair_cdr(actions))) {
      eval_object(pair_car(actions), environment);
      actions = pair_cdr(actions);
    }
    /* return eval_object(pair_car(actions), environment); */
    object = pair_car(actions);
    goto tail_loop;
  }
  if (is_cond_form(object)) {
    /* return eval_object(cond2if(object), environment); */
    object = cond2if(object);
    goto tail_loop;
  }
  if (is_let_form(object)) {
    object = let2lambda(object);
    goto tail_loop;
    /* return let2lambda(object); */
  }
  if (is_and_form(object)) {
    lisp_object_t tests = and_tests(object);
    if (is_null(tests))
      return make_true();
    while (is_pair(pair_cdr(tests))) {
      lisp_object_t result = eval_object(pair_car(tests), environment);
      if (is_false(result))
        return make_false();
      tests = pair_cdr(tests);
    }
    return eval_object(pair_car(tests), environment);
  }
  if (is_or_form(object)) {
    lisp_object_t tests = or_tests(object);
    if (is_null(tests))
      return make_false();
    while (is_pair(pair_cdr(tests))) {
      lisp_object_t result = eval_object(pair_car(tests), environment);
      if (!is_false(result))
        return result;
      tests = pair_cdr(tests);
    }
    return eval_object(pair_car(tests), environment);
  }
  if (is_application_form(object)) {
    lisp_object_t operator = application_operator(object);
    lisp_object_t operands = application_operands(object);
    operator = eval_object(operator, environment);
    if (is_undefined(operator)) {
      fprintf(stderr, "Undefined function\n");
      exit(1);
    }
    operands = eval_arguments(operands, environment);
    if (is_primitive(operator))
      return (primitive_C_proc(operator))(operands);
    else {
      lisp_object_t body = compound_proc_body(operator);
      lisp_object_t vars = compound_proc_parameters(operator);
      lisp_object_t def_env = compound_proc_environment(operator);
      object = make_pair(find_or_create_symbol("begin"), body);
      environment = extend_environment(vars, operands, def_env);
      goto tail_loop;
    }
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
