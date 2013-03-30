#ifndef EVAL_H
#define EVAL_H

#include "types.h"

extern lisp_object_t eval_object(lisp_object_t, lisp_object_t);
extern lisp_object_t eval_application(lisp_object_t, lisp_object_t);

/* Parse utilities */
/* begin */
extern int is_begin_form(lisp_object_t);
extern lisp_object_t begin_actions(lisp_object_t);
/* define */
extern int is_define_form(sexp);
extern sexp define2set(sexp);
/* if */
extern int is_if_form(lisp_object_t);
extern lisp_object_t if_test_part(lisp_object_t);
extern lisp_object_t if_then_part(lisp_object_t);
extern lisp_object_t if_else_part(lisp_object_t);
/* lambda */
extern int is_lambda_form(lisp_object_t);
extern lisp_object_t lambda_parameters(lisp_object_t);
extern lisp_object_t lambda_body(lisp_object_t);
/* quote */
extern int is_quote_form(lisp_object_t);
extern lisp_object_t quotation_text(lisp_object_t);
/* set! */
extern int is_assignment_form(lisp_object_t);
extern lisp_object_t assignment_variable(lisp_object_t);
extern lisp_object_t assignment_value(lisp_object_t);

extern int is_variable_form(lisp_object_t);
extern int is_application_form(lisp_object_t);
extern lisp_object_t application_operands(lisp_object_t);
extern lisp_object_t application_operator(lisp_object_t);

#endif
