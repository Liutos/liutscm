/*
 * eval.c
 *
 * Evaluator for S-exp
 *
 * Copyright (C) 2013-03-13 liutos <mat.liutos@gmail.com>
 */
#include "types.h"
#include "read.h"

int is_quote_form(lisp_object_t object) {
  return is_pair(object) && find_or_create_symbol("quote") == pair_car(object);
}

lisp_object_t quotation_text(lisp_object_t quote_form) {
  return pair_cadr(quote_form);
}

lisp_object_t eval_object(lisp_object_t object) {
  if (is_quote_form(object))
    return quotation_text(object);
  else
    return object;
}
