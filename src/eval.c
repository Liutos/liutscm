/*
 * eval.c
 *
 * Evaluator for S-exp
 *
 * Copyright (C) 2013-03-13 liutos <mat.liutos@gmail.com>
 */
#include <stdlib.h>
#include <stdio.h>

#include "types.h"
#include "read.h"
#include "object.h"

extern void write_object(lisp_object_t, lisp_object_t);

lisp_object_t eval_object(lisp_object_t, lisp_object_t);

int is_tag_list(lisp_object_t object, char *symbol_name) {
  return is_pair(object) && find_or_create_symbol(symbol_name) == pair_car(object);
}

/* QUOTE support */

int is_quote_form(lisp_object_t object) {
  return is_tag_list(object, "quote");
}

lisp_object_t quotation_text(lisp_object_t quote_form) {
  return pair_cadr(quote_form);
}

/* Variable support */

int is_variable_form(lisp_object_t object) {
  return is_symbol(object);
}

/* DEFINE support */

int is_define_form(lisp_object_t object) {
  return is_tag_list(object, "define");
}

lisp_object_t definition_variable(lisp_object_t define_form) {
  return pair_cadr(define_form);
}

lisp_object_t definition_value(lisp_object_t define_form) {
  return pair_caddr(define_form);
}

/* SET! support */

int is_assignment_form(lisp_object_t object) {
  return is_tag_list(object, "set!");
}

lisp_object_t assignment_variable(lisp_object_t assignment_form) {
  return pair_cadr(assignment_form);
}

lisp_object_t assignment_value(lisp_object_t assignment_form) {
  return pair_caddr(assignment_form);
}

/* IF support */

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

/* Procedures application support */

int is_application_form(lisp_object_t object) {
  return is_pair(object);
}

lisp_object_t application_operator(lisp_object_t application_form) {
  return pair_car(application_form);
}

lisp_object_t application_operands(lisp_object_t application_form) {
  return pair_cdr(application_form);
}

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

/* BEGIN support */

int is_begin_form(lisp_object_t object) {
  return is_tag_list(object, "begin");
}

lisp_object_t begin_actions(lisp_object_t begin_form) {
  return pair_cdr(begin_form);
}

sexp eval_begin(sexp actions, sexp env) {
  if (is_null(actions)) return EOL;
  while (!is_null(pair_cdr(actions))) {
    eval_object(pair_car(actions), env);
    actions = pair_cdr(actions);
  }
  return eval_object(pair_car(actions), env);
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

/* APPLY support */

extern lisp_object_t apply_proc(lisp_object_t);

int is_apply(lisp_object_t proc) {
  return is_primitive(proc) && apply_proc == primitive_C_proc(proc);
}

lisp_object_t apply_operands_conc(lisp_object_t operands) {
  if (is_null(operands))
    return make_empty_list();
  if (is_null(pair_cdr(operands)))
    return pair_car(operands);
  else
    return make_pair(pair_car(operands),
                     apply_operands_conc(pair_cdr(operands)));
}

/* EVAL support */

extern lisp_object_t eval_proc(lisp_object_t);

int is_eval(lisp_object_t proc) {
  return is_primitive(proc) && eval_proc == primitive_C_proc(proc);
}

lisp_object_t eval_expression(lisp_object_t eval_form) {
  return pair_cadr(eval_form);
}

lisp_object_t eval_environment(lisp_object_t eval_form) {
  return pair_caddr(eval_form);
}

lisp_object_t eval_application(lisp_object_t operator, lisp_object_t operands) {
  if (is_primitive(operator))
    return (primitive_C_proc(operator))(operands);
  if (is_compound(operator)) {
    lisp_object_t body = compound_proc_body(operator);
    lisp_object_t vars = compound_proc_parameters(operator);
    lisp_object_t def_env = compound_proc_environment(operator);
    lisp_object_t object = make_pair(find_or_create_symbol("begin"), body);
    lisp_object_t environment = extend_environment(vars, operands, def_env);
    return eval_object(object, environment);
  }
  /* if (is_compiled_proc(operator)) { */

  /* } */
  fprintf(stderr, "Unknown operator type %d\n", operator->type);
  exit(1);
}

/* MACRO support */

int is_macro_form(sexp object) {
  return is_tag_list(object, "macro");
}

sexp macro_parameters(sexp macro_form) {
  return pair_cadr(macro_form);
}

sexp macro_body(sexp macro_form) {
  return pair_cddr(macro_form);
}

sexp eval_object(sexp object, sexp environment) {
tail_loop:
  if (is_quote_form(object))
    return quotation_text(object);
  if (is_variable_form(object))
    return get_variable_value(object, environment);
  if (is_define_form(object)) {
    sexp value = eval_object(definition_value(object), environment);
    add_binding(definition_variable(object), value, environment);
    return value;
  }
  if (is_assignment_form(object)) {
    sexp value = eval_object(assignment_value(object), environment);
    set_binding(assignment_variable(object), value, environment);
    return value;
  }
  if (is_if_form(object)) {
    sexp test_part = if_test_part(object);
    sexp then_part = if_then_part(object);
    sexp else_part = if_else_part(object);
    if (!is_false(eval_object(test_part, environment))) {
      object = then_part;
    } else {
      object = else_part;
    }
    goto tail_loop;
  }
  if (is_lambda_form(object)) {
    sexp parameters = lambda_parameters(object);
    sexp body = lambda_body(object);
    return make_lambda_procedure(parameters, body, environment);
  }
  if (is_begin_form(object)) {
    return eval_begin(object, environment);
  }
  if (is_cond_form(object)) {
    object = cond2if(object);
    goto tail_loop;
  }
  if (is_let_form(object)) {
    object = let2lambda(object);
    goto tail_loop;
  }
  if (is_and_form(object)) {
    sexp tests = and_tests(object);
    if (is_null(tests))
      return make_true();
    while (is_pair(pair_cdr(tests))) {
      sexp result = eval_object(pair_car(tests), environment);
      if (is_false(result))
        return make_false();
      tests = pair_cdr(tests);
    }
    return eval_object(pair_car(tests), environment);
  }
  if (is_or_form(object)) {
    sexp tests = or_tests(object);
    if (is_null(tests))
      return make_false();
    while (is_pair(pair_cdr(tests))) {
      sexp result = eval_object(pair_car(tests), environment);
      if (!is_false(result))
        return result;
      tests = pair_cdr(tests);
    }
    return eval_object(pair_car(tests), environment);
  }
  if (is_macro_form(object)) {
    sexp pars = macro_parameters(object);
    sexp body = macro_body(object);
    return make_macro_procedure(pars, body, environment);
  }
  if (is_application_form(object)) {
    sexp operator = application_operator(object);
    sexp operands = application_operands(object);
    operator = eval_object(operator, environment);
    if (!is_function(operator) && !is_macro(operator)) {
      fprintf(stderr, "Illegal functional object ");
      write_object(operator, make_file_out_port(stderr));
      fprintf(stderr, " from ");
      write_object(pair_car(object), make_file_out_port(stderr));
      fputc('\n', stderr);
      exit(1);
    }
    /* Expand the macro before evaluating arguments */
    if (is_macro(operator)) {
      sexp body = macro_proc_body(operator);
      sexp vars = macro_proc_pars(operator);
      sexp def_env = macro_proc_env(operator);
      sexp object = make_pair(find_or_create_symbol("begin"), body);
      sexp env = extend_environment(vars, operands, def_env);
      sexp exp = eval_object(object, env);
      /* return exp; */
      return eval_object(exp, environment);
    }
    operands = eval_arguments(operands, environment);
    if (is_apply(operator)) {
      operator = pair_car(operands);
      operands = apply_operands_conc(pair_cdr(operands));
    }
    if (is_eval(operator)) {
      environment = pair_cadr(operands);
      object = pair_car(operands);
      goto tail_loop;
    }
    return eval_application(operator, operands);
  }
  else
    return object;
}
