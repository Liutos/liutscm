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

#include "compiler.h"
#include "eval.h"
#include "types.h"
#include "object.h"

#define BUFFER_SIZE 10

#define seq(...) sequenzie(__VA_ARGS__, NULL)
#define gen(...) generate_code(__VA_ARGS__, NULL)
#define gen_args(x) gen("ARGS", x)
#define gen_argsdot(x) gen("ARGS.", x)
#define gen_call(x) gen("CALL", x)
#define gen_callj(x) gen("CALLJ", x)
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
#define gen_save(k) gen("SAVE", k)

extern sexp make_lambda_form(sexp, sexp);

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

sexp gen_var(sexp x, sexp env) {
  int i, j;
  if (!is_variable_found(x, env, &i, &j))
    return gen_gvar(x);
  else
    return gen_lvar(make_fixnum(i), make_fixnum(j));
}

/* Concatenates a series of list of instructions */
lisp_object_t sequenzie(lisp_object_t pair, ...) {
  va_list ap;
  va_start(ap, pair);
  return nconc_pair(pair, sequenzie_aux(ap));
}

/* Compiler */
lisp_object_t compile_constant(lisp_object_t val, int is_val, int is_more) {
  /* return gen_const(val); */
  if (!is_val) return EOL;
  return seq(gen_const(val), is_more ? EOL: gen_return());
}

lisp_object_t compile_set(lisp_object_t var, lisp_object_t environment) {
  int i, j;
  if (!is_variable_found(var, environment, &i, &j))
    return gen_gset(var);
  else
    return gen_lset(make_fixnum(i), make_fixnum(j));
}

sexp compile_begin(sexp actions, sexp env, int is_val, int is_more) {
  if (is_null(actions))
    return compile_constant(EOL, is_val, is_more);
  if (is_null(pair_cdr(actions)))
    return compile_object(pair_car(actions), env, is_val, is_more);
  else
    return seq(compile_object(pair_car(actions), env, no, yes),
               gen_pop(),
               compile_begin(pair_cdr(actions), env, is_val, is_more));
}

sexp gen_args_ins(sexp pars, int n) {
tail_loop:
  if (is_null(pars)) return gen_args(make_fixnum(n));
  if (is_symbol(pars)) return gen_argsdot(make_fixnum(n));
  if (is_pair(pars) && is_symbol(pair_car(pars))) {
    pars = pair_cdr(pars);
    n++;
    goto tail_loop;
  }
  fprintf(stderr, "Illegal argument list\n");
  exit(1);
}

sexp make_proper_list(sexp dotable_list) {
  if (is_null(dotable_list)) return EOL;
  sexp head = dotable_list;
  while (is_pair(pair_cdr(dotable_list)))
    dotable_list = pair_cdr(dotable_list);
  if (!is_null(pair_cdr(dotable_list)))
    pair_cdr(dotable_list) = make_pair(pair_cdr(dotable_list), EOL);
  return head;
}

sexp compile_lambda(sexp args, sexp body, sexp env) {
  sexp pars = make_proper_list(args);
  sexp new_env = extend_environment(pars, EOL, env);
  /* sexp code = */
  /*     seq(gen_args(make_fixnum(pair_length(args))), */
  /*         compile_begin(body, new_env, yes, no), */
  /*         gen_return()); */
  sexp code =
      seq(gen_args_ins(args, 0),
          compile_begin(body, new_env, yes, no));
  return make_compiled_proc(args, code, new_env);
}

sexp compile_arguments(sexp args, sexp env) {
  if (is_null(args)) return EOL;
  sexp first = compile_object(pair_car(args), env, yes, yes);
  return seq(first, compile_arguments(pair_cdr(args), env));
}

sexp compile_var(sexp object, sexp env, int is_val, int is_more) {
  /* int i, j; */
  /* if (!is_variable_found(object, env, &i, &j)) */
  /*   return gen_gvar(object); */
  /* else */
  /*   return gen_lvar(make_fixnum(i), make_fixnum(j)); */
  /* return gen_var(object, env); */
  if (!is_val) return EOL;
  return seq(gen_var(object, env), is_more ? EOL: gen_return());
}

sexp compile_assignment(sexp object, sexp env, int is_val, int is_more) {
  sexp value = compile_object(assignment_value(object), env, is_val, is_more);
  return seq(value, compile_set(assignment_variable(object), env));
}

sexp compile_if(sexp object, sexp env, int is_val, int is_more) {
  /* sexp l1 = make_label(); */
  /* sexp l2 = make_label(); */
  /* return seq(compile_object(if_test_part(object), env, is_val, is_more), */
  /*            gen_fjump(l1), */
  /*            compile_object(if_then_part(object), env, is_val, is_more), */
  /*            gen_jump(l2), */
  /*            make_list1(l1), */
  /*            compile_object(if_else_part(object), env, is_val, is_more), */
  /*            make_list1(l2)); */

  /* optimize: (if #f x y) => y */
  if (is_false(if_test_part(object)))
    return compile_object(if_else_part(object), env, is_val, is_more);
  /* optimize: (if #t x y) => x */
  if (is_self_eval(if_test_part(object)))
    return compile_object(if_then_part(object), env, is_val, is_more);
  /* optimize: (if p x x) => (begin p x) */
  if (if_then_part(object) == if_else_part(object))
    return seq(compile_object(if_test_part(object), env, no, yes),
               compile_object(if_then_part(object), env, yes, no));

  sexp pcode = compile_object(if_test_part(object), env, is_val, is_more);
  sexp tcode = compile_object(if_then_part(object), env, is_val, is_more);
  sexp ecode = compile_object(if_else_part(object), env, is_val, is_more);
  sexp l1 = make_label();
  sexp l2 = is_more ? make_label(): EOL;
  return seq(pcode, gen_fjump(l1), tcode,
             (is_more ? gen_jump(l2): EOL),
             make_list1(l1), ecode,
             (is_more ? make_list1(l2): EOL));
}

sexp compile_application(sexp object, sexp env, int is_val, int is_more) {
  /* int length = pair_length(application_operands(object)); */
  /* return seq(compile_arguments(application_operands(object), env), */
  /*            compile_object(application_operator(object), env, is_val, is_more), */
  /*            gen_call(make_fixnum(length))); */
  sexp operator = application_operator(object);
  sexp operands = application_operands(object);
  int len = pair_length(operands);
  if (is_more) {
    sexp k = make_label();
    return seq(gen_save(k),
               compile_arguments(operands, env),
               compile_object(operator, env, yes, yes),
               gen_callj(make_fixnum(len)),
               make_list1(k),
               (is_val ? EOL: gen_pop()));
  } else
    return seq(compile_arguments(operands, env),
               compile_object(operator, env, yes, yes),
               gen_callj(make_fixnum(len)));
}

/* Generate a list of instructions based-on a stack-based virtual machine. */
sexp compile_object(sexp object, sexp env, int is_val, int is_more) {
  if (is_variable_form(object))
    return compile_var(object, env, is_val, is_more);
  if (is_quote_form(object))
    return compile_constant(quotation_text(object), is_val, is_more);
  if (is_assignment_form(object))
    return compile_assignment(object, env, is_val, is_more);
  if (is_define_form(object))
    return compile_object(define2set(object), env, is_val, is_more);
  if (is_if_form(object))
    return compile_if(object, env, is_val, is_more);
  if (is_begin_form(object))
    return compile_begin(begin_actions(object), env, is_val, is_more);
  if (is_lambda_form(object)) {
    sexp args = lambda_parameters(object);
    sexp body = lambda_body(object);
    return gen_fn(compile_lambda(args, body, env));
  }
  if (is_application_form(object))
    return compile_application(object, env, is_val, is_more);
  return compile_constant(object, is_val, is_more);
}

sexp compile_as_fn(sexp obj, sexp env) {
  return compile_lambda(EOL, make_list1(obj), env);
}
