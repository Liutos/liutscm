/*
 * eval.c
 *
 * Evaluator for S-exp
 *
 * Copyright (C) 2013-03-13 liutos <mat.liutos@gmail.com>
 */
#include <stdio.h>
#include <stdlib.h>

#include "object.h"
#include "types.h"
#include "write.h"

#define DEFACC(fn, acc)                         \
  sexp fn(sexp form) {                          \
    return acc(form);                           \
  }

#define DEFORM(name, sym)                       \
  int name(sexp obj) {                          \
    return is_tag_list(obj, sym);               \
  }

/* extern lisp_object_t apply_proc(lisp_object_t); */
extern lisp_object_t eval_proc(lisp_object_t);

sexp eval_object(sexp, sexp);

int is_tag_list(sexp object, char *symbol_name) {
  return is_pair(object) && S(symbol_name) == pair_car(object);
}

int is_variable_form(sexp object) {
  return is_symbol(object);
}

sexp make_lambda_form(sexp vars, sexp body) {
  return make_pair(S("lambda"), make_pair(vars, body));
}

/* quote */

DEFORM(is_quote_form, "quote")
DEFACC(quotation_text, pair_cadr)

/* define */

DEFORM(is_define_form, "define")
/* DEFACC(definition_variable, pair_cadr) */
/* DEFACC(definition_value, pair_caddr) */

sexp definition_variable(sexp form) {
  if (is_symbol(pair_cadr(form)))
    return pair_cadr(form);
  else
    return pair_car(pair_cadr(form));
}

sexp definition_value(sexp form) {
  if (is_symbol(pair_cadr(form)))
    return pair_caddr(form);
  else
    return make_lambda_form(pair_cdr(pair_cadr(form)), pair_cddr(form));
}

sexp define2set(sexp form) {
  return LIST(S("set!"), definition_variable(form), definition_value(form));
}

/* set! */

DEFORM(is_assignment_form, "set!")
DEFACC(assignment_variable, pair_cadr)
DEFACC(assignment_value, pair_caddr)

/* if */

DEFORM(is_if_form, "if")
DEFACC(if_test_part, pair_cadr)
DEFACC(if_then_part, pair_caddr)

sexp if_else_part(sexp if_form) {
  sexp alt = pair_cdddr(if_form);
  if (is_null(alt)) return EOL;
  else return pair_car(alt);
}

/* lambda */

DEFORM(is_lambda_form, "lambda")
DEFACC(lambda_parameters, pair_cadr)
DEFACC(lambda_body, pair_cddr)

/* begin */

DEFORM(is_begin_form, "begin")
DEFACC(begin_actions, pair_cdr)

/* application case */

int is_application_form(sexp object) {
  return is_pair(object);
}

DEFACC(application_operator, pair_car)
DEFACC(application_operands, pair_cdr)

/* cond */

DEFORM(is_cond_form, "cond")
DEFACC(cond_clauses, pair_cdr)
DEFACC(clause_test, pair_car)

sexp clause_actions(sexp clause) {
  return make_pair(S("begin"), pair_cdr(clause));
}

int is_cond_else_clause(lisp_object_t clause) {
  return S("else") == clause_test(clause);
}

sexp make_if_form(sexp test, sexp then_part, sexp else_part) {
  return LIST(S("if"), test, then_part, else_part);
}

lisp_object_t expand_cond_clauses(lisp_object_t clauses) {
  if (is_null(clauses)) return EOL;
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

/* let */

DEFORM(is_let_form, "let")
DEFACC(let_body, pair_cddr)

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

lisp_object_t let2lambda(lisp_object_t let_form) {
  lisp_object_t vars = let_vars(let_form);
  lisp_object_t vals = let_vals(let_form);
  lisp_object_t body = let_body(let_form);
  lisp_object_t lambda_form = make_lambda_form(vars, body);
  return make_pair(lambda_form, vals);
}

/* and */

DEFORM(is_and_form, "and")
DEFACC(and_tests, pair_cdr)

/* or */

DEFORM(is_or_form, "or")
DEFACC(or_tests, pair_cdr)

/* apply */

/* int is_apply(lisp_object_t proc) { */
/*   return is_primitive(proc) && apply_proc == primitive_C_proc(proc); */
/* } */

lisp_object_t apply_operands_conc(lisp_object_t operands) {
  if (is_null(operands)) return EOL;
  if (is_null(pair_cdr(operands)))
    return pair_car(operands);
  else
    return make_pair(pair_car(operands),
                     apply_operands_conc(pair_cdr(operands)));
}

/* eval */

int is_eval(lisp_object_t proc) {
  return is_primitive(proc) && eval_proc == primitive_C_proc(proc);
}

DEFACC(eval_expression, pair_cadr)
DEFACC(eval_environment, pair_caddr)

/* macro */

DEFORM(is_macro_form, "macro")
DEFACC(macro_parameters, pair_cadr)
DEFACC(macro_body, pair_cddr)

/* evaluators */
sexp eval_begin(sexp actions, sexp env) {
  if (is_null(actions)) return EOL;
  while (!is_null(pair_cdr(actions))) {
    eval_object(pair_car(actions), env);
    actions = pair_cdr(actions);
  }
  return eval_object(pair_car(actions), env);
}

sexp eval_application(sexp operator, sexp operands) {
  if (is_primitive(operator))
    return (primitive_C_proc(operator))(operands);
  if (is_compound(operator)) {
    sexp body = compound_proc_body(operator);
    sexp vars = compound_proc_parameters(operator);
    sexp def_env = compound_proc_environment(operator);
    sexp object = make_pair(find_or_create_symbol("begin"), body);
    sexp env = extend_environment(vars, operands, def_env);
    return eval_object(object, env);
  }
  fprintf(stderr, "Unknown operator type %d\n", operator->type);
  exit(1);
}

sexp eval_arguments(sexp arguments, sexp env) {
  if (is_null(arguments)) return EOL;
  else {
    sexp first = pair_car(arguments);
    return make_pair(eval_object(first, env),
                     eval_arguments(pair_cdr(arguments), env));
  }
}

sexp eval_object(sexp object, sexp environment) {
tail_loop:
  if (is_quote_form(object)) return quotation_text(object);
  if (is_variable_form(object))
    return get_variable_value(object, environment);
  if (is_define_form(object)) {
    /* sexp value = eval_object(definition_value(object), environment); */
    /* add_binding(definition_variable(object), value, environment); */
    /* return value; */
    return eval_object(define2set(object), environment);
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
      sexp object = make_pair(S("begin"), body);
      sexp env = extend_environment(vars, operands, def_env);
      sexp exp = eval_object(object, env);
      return eval_object(exp, environment);
    }
    operands = eval_arguments(operands, environment);
    /* if (is_apply(operator)) { */
    /*   operator = pair_car(operands); */
    /*   operands = apply_operands_conc(pair_cdr(operands)); */
    /* } */
    if (is_eval(operator)) {
      environment = pair_cadr(operands);
      object = pair_car(operands);
      goto tail_loop;
    }
    return eval_application(operator, operands);
  } else return object;
}
