/*
 * proc.c
 *
 * Primitive procedures implementation
 *
 * Copyright (C) 2013-03-17 liutos <mat.liutos@gmail.com>
 */
#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "eval.h"
#include "object.h"
#include "read.h"
#include "types.h"
#include "write.h"

#define DEFPROC(Lisp_name, C_proc, is_se, code_name, arity)                   \
  {.type=PRIMITIVE_PROC, .values={.primitive_proc={(C_proc_t)C_proc, is_se, Lisp_name, code_name, to_fixnum(arity)}}}
#define PHEAD(C_proc) lisp_object_t C_proc(lisp_object_t args)

extern int nzero(char);
extern int utf8_strlen(char *);

/* FIXNUM */
/* The following four is defined as instructions */
/* Binary plus */
sexp plus_proc(sexp args) {
  sexp n1 = pair_car(args);
  sexp n2 = pair_cadr(args);
  return make_fixnum(fixnum_value(n1) + fixnum_value(n2));
}

/* Binary minus */
sexp minus_proc(sexp args) {
  sexp n1 = pair_car(args);
  sexp n2 = pair_cadr(args);
  return make_fixnum(fixnum_value(n1) - fixnum_value(n2));
}

/* Binary multiply */
sexp multiply_proc(sexp args) {
  sexp n1 = pair_car(args);
  sexp n2 = pair_cadr(args);
  return make_fixnum(fixnum_value(n1) * fixnum_value(n2));
}

/* Binary divide */
sexp divide_proc(sexp args) {
  sexp n1 = pair_car(args);
  sexp n2 = pair_cadr(args);
  return make_fixnum(fixnum_value(n1) / fixnum_value(n2));
}

/* Binary numeric equal */
sexp fixnum_equal_proc(sexp n1, sexp n2) {
  return fixnum_value(n1) == fixnum_value(n2) ? true_object: false_object;
}

sexp modulo_proc(sexp n1, sexp n2) {
  return make_fixnum(fixnum_value(n1) % fixnum_value(n2));
}

sexp greater_than_proc(sexp n1, sexp n2) {
  return fixnum_value(n1) > fixnum_value(n2) ? true_object: false_object;
}

/* Bitwise and */
sexp bit_and_proc(sexp n1, sexp n2) {
  return make_fixnum(fixnum_value(n1) & fixnum_value(n2));
}

/* Bitwise or */
sexp bit_or_proc(sexp n1, sexp n2) {
  return make_fixnum(fixnum_value(n1) | fixnum_value(n2));
}

/* Bitwise not */
sexp bit_not_proc(sexp n) {
  return make_fixnum(~fixnum_value(n));
}

/* CHAR */
/* Get the encode of a character */
sexp char2code_proc(sexp c) {
  return make_fixnum(char_value(c));
}

/* Return a character with given encode */
sexp code2char_proc(sexp n) {
  return make_character(fixnum_value(n));
}

/* STRING */
/* Get the specific character in a string */
/* A O(n) string referencing function */
sexp string_ref(sexp str, sexp index) {
  assert(is_string(str) || is_wstring(str));
  switch (str->type) {
    case STRING: {
      char *val = string_value(str);
      for (int n = fixnum_value(index); n > 0; n--) {
        int n = nzero(*val);
        val += 1 + n;
      }
      if (nzero(*val) == 0)
        return make_character(*val);
      else {
        sexp wc = make_wchar();
        char c = *val;
        for (int i = 0; i < nzero(c); i++) {
          wchar_value(wc)[i] = *val;
          val++;
        }
        return wc;
      }
    }
    case WSTRING:
      return wstring_value(str)[fixnum_value(index)];
    default :
      port_format(scm_err_port, "Impossible!\n");
      exit(1);
  }
}
/* sexp char_at_proc(sexp str, sexp n) { */
/*   return make_character(string_value(str)[fixnum_value(n)]); */
/* } */

/* sexp string_length_proc(sexp str) { */
/*   unsigned int len = strlen(string_value(str)); */
/*   return make_fixnum(len); */
/* } */
sexp string_length(sexp str) {
  assert(is_string(str) || is_wstring(str));
  if (is_string(str))
    return make_fixnum(utf8_strlen(string_value(str)));
  else
    return make_fixnum(wstring_length(str));
}

/* sexp string_equal_proc(sexp s1, sexp s2) { */
/*   return strcmp(string_value(s1), string_value(s2)) ? false_object: true_object; */
/* } */
sexp string_equalp(sexp s1, sexp s2) {
  assert(s1->type == s2->type);
  assert(is_string(s1));
  return strcmp(string_value(s1), string_value(s2)) ? false_object: true_object;
}

sexp string_set(sexp s, sexp n, sexp c) {
  assert(is_wstring(s) || is_string(s));
  if (is_wstring(s))
    wstring_value(s)[fixnum_value(n)] = c;
  else {
    /* char *seq = string_value(s); */
    /* for (int i = 0; i < n; i++) { */
    /*   char c = *seq; */
    /*   if (nzero(c) == 0) seq++; */
    /*   else seq += nzero(c); */
    /* } */
    port_format(scm_err_port, "Unsupported - I'm a lazy man...\n");
    exit(1);
  }
  return s;
}

/* PAIR */
/* The two following primitives is also defined as instructions */
lisp_object_t pair_car_proc(lisp_object_t args) {
  lisp_object_t list = pair_car(args);
  return pair_car(list);
}

lisp_object_t pair_cdr_proc(lisp_object_t args) {
  lisp_object_t list = pair_car(args);
  return pair_cdr(list);
}

sexp pair_set_car_proc(sexp pair, sexp val) {
  pair_car(pair) = val;
  return pair;
}

sexp pair_set_cdr_proc(sexp pair, sexp val) {
  pair_cdr(pair) = val;
  return pair;
}

/* SYMBOL */
sexp symbol_name_proc(sexp sym) {
  return make_string(symbol_name(sym));
}

/* Create a symbol looks the same as the string argument */
sexp string2symbol_proc(sexp str) {
  return S(string_value(str));
}

/* VECTOR */
sexp vector_ref_proc(sexp vector, sexp n) {
  return vector_data_at(vector, fixnum_value(n));
}

sexp vector_set_proc(sexp vector, sexp n, sexp value) {
  vector_data_at(vector, fixnum_value(n)) = value;
  return value;
}

/* FILE_IN_PORT */
sexp open_in_proc(sexp path) {
  FILE *fp = fopen(string_value(path), "r");
  if (NULL == fp) {
    fprintf(stderr, "Can not open file '%s'\n", string_value(path));
    exit(1);
  }
  return make_file_in_port(fp);
}

sexp read_char_proc(sexp port) {
  char c = port_read_char(port);
  if (nzero(c) == 0)
    return make_character(c);
  else {
    assert(nzero(c) < 6);
    sexp wc = make_wchar();
    wchar_value(wc)[0] = c;
    for (int i = 1; i < nzero(c); i++) {
      char ch = port_read_char(port);
      wchar_value(wc)[i] = ch;
    }
    return wc;
  }
}

sexp close_in_proc(sexp port) {
  fclose(in_port_stream(port));
  return make_undefined();
}

/* Read and parse an S-exp */
sexp read_proc(void) {
  return read_object(scm_in_port);
}

/* FILE_OUT_PORT */
sexp open_out_proc(sexp path) {
  FILE *fp = fopen(string_value(path), "w");
  if (NULL == fp) {
    fprintf(stderr, "Can not open file '%s'\n", string_value(path));
    exit(1);
  }
  return make_file_out_port(fp);
}

sexp write_char_proc(sexp ch, sexp port) {
  fputc(char_value(ch), out_port_stream(port));
  return make_undefined();
}

sexp close_out_proc(sexp port) {
  fclose(out_port_stream(port));
  return make_undefined();
}

/* Write an object to standard output */
sexp write_proc(sexp object) {
  write_object(object, scm_out_port);
  return make_undefined();
}

/* FLONUM */
sexp flonum_plus_proc(sexp n1, sexp n2) {
  return make_flonum(float_value(n1) + float_value(n2));
}

sexp flonum_minus_proc(sexp n1, sexp n2) {
  return make_flonum(float_value(n1) - float_value(n2));
}

sexp flonum_multiply_proc(sexp n1, sexp n2) {
  return make_flonum(float_value(n1) * float_value(n2));
}

sexp flonum_divide_proc(sexp n1, sexp n2) {
  return make_flonum(float_value(n1) / float_value(n2));
}

sexp integer_to_float_proc(sexp n) {
  return make_flonum((float)(fixnum_value(n)));
}

/* Others */
sexp eval_proc(sexp exp, sexp env) {
  return eval_object(exp, env);
}

/* Are the two arguments identical? */
sexp is_identical_proc(sexp o1, sexp o2) {
  return o1 == o2 ? make_true(): make_false();
}

/* Return a symbol indicates the argument's type */
sexp type_of_proc(sexp o) {
  if (is_fixnum(o)) return S("fixnum");
  else if (is_bool(o)) return S("boolean");
  else if (is_char(o)) return S("character");
  else if (is_null(o)) return S("empty-list");
  else {
    switch (o->type) {
      case STRING: return S("string");
      case PAIR: return S("pair");
      case SYMBOL: return S("symbol");
      case PRIMITIVE_PROC: return S("function");
      case FILE_IN_PORT: return S("file-in-port");
      default :
        fprintf(stderr, "Unknown data type: %d\n", o->type);
        exit(1);
    }
  }
}

/* Environment */
/* Return the environment used by the REPL */
sexp get_repl_environment_proc(void) {
  return repl_environment;
}

/* Return the environment with default bindings */
sexp get_startup_environment(void) {
  return startup_environment;
}

/* Return a environment with nothing */
sexp get_null_environment(void) {
  return null_environment;
}

void add_primitive_proc(sexp proc, sexp env) {
  sexp var = S(primitive_name(proc));
  add_binding(var, proc, env);
}

struct lisp_object_t primitive_procs[] = {
  DEFPROC("+i", plus_proc, no, "IADD", 2),
  DEFPROC("-i", minus_proc, no, "ISUB", 2),
  DEFPROC("*i", multiply_proc, no, "IMUL", 2),
  DEFPROC("/i", divide_proc, no, "IDIV", 2),
  DEFPROC("remainder", modulo_proc, no, NULL, 2),
  DEFPROC("=", fixnum_equal_proc, no, NULL, 2),
  DEFPROC(">", greater_than_proc, no, NULL, 2),
  DEFPROC("&", bit_and_proc, no, NULL, 2),
  DEFPROC("|", bit_or_proc, no, NULL, 2),
  DEFPROC("~", bit_not_proc, no, NULL, 1),
  DEFPROC("char->integer", char2code_proc, no, NULL, 1),
  DEFPROC("integer->char", code2char_proc, no, NULL, 1),
  /* DEFPROC("string-ref", char_at_proc, no, NULL, 2), */
  DEFPROC("string-ref", string_ref, no, NULL, 2),
  /* DEFPROC("string-length", string_length_proc, no, NULL, 1), */
  DEFPROC("string-length", string_length, no, NULL, 1),
  /* DEFPROC("string=?", string_equal_proc, no, NULL, 2), */
  DEFPROC("string=?", string_equalp, no, NULL, 2),
  DEFPROC("string-set!", string_set, yes, NULL, 3),
  DEFPROC("car", pair_car_proc, no, "CAR", 1),
  DEFPROC("cdr", pair_cdr_proc, no, "CDR", 1),
  /* DEFPROC("cons", pair_cons_proc, no, NULL, 2), */
  DEFPROC("cons", make_pair, no, NULL, 2),
  DEFPROC("set-car!", pair_set_car_proc, yes, NULL, 2),
  DEFPROC("set-cdr!", pair_set_cdr_proc, yes, NULL, 2),
  DEFPROC("symbol-name", symbol_name_proc, no, NULL, 1),
  DEFPROC("string->symbol", string2symbol_proc, no, NULL, 1),
  /* DEFPROC("apply", apply_proc, yes, NULL, -1), */
  DEFPROC("open-in", open_in_proc, yes, NULL, 1),
  /* DEFPROC("read-char", read_char_proc, yes, NULL, 1), */
  DEFPROC("read-char", read_char, yes, NULL, 1),
  DEFPROC("close-in", close_in_proc, yes, NULL, 1),
  DEFPROC("read", read_proc, yes, NULL, 0),
  DEFPROC("read-byte", read_byte, yes, NULL, 1),
  /* FILE_OUT_PORT */
  DEFPROC("open-out", open_out_proc, yes, NULL, 1),
  DEFPROC("write-char", write_char_proc, yes, NULL, 2),
  DEFPROC("close-out", close_out_proc, yes, NULL, 1),
  DEFPROC("write", write_proc, yes, NULL, 1),
  DEFPROC("vector-ref", vector_ref_proc, no, NULL, 2),
  DEFPROC("vector-set!", vector_set_proc, yes, NULL, 3),
  DEFPROC("+.", flonum_plus_proc, no, NULL, 2),
  DEFPROC("-.", flonum_minus_proc, no, NULL, 2),
  DEFPROC("*.", flonum_multiply_proc, no, NULL, 2),
  DEFPROC("/.", flonum_divide_proc, no, NULL, 2),
  DEFPROC("integer->float", integer_to_float_proc, no, NULL, 1),
  DEFPROC("repl-environment", get_repl_environment_proc, no, NULL, 0),
  /* STRING_IN_PORT */
  /* DEFPROC("string->in-port", string2in_port_proc, no, NULL, 1), */
  /* DEFPROC("read-string-in-port-char", read_sp_char_proc, yes, NULL, 1), */
  /* Others */
  DEFPROC("type-of", type_of_proc, no, NULL, 1),
  DEFPROC("eq?", is_identical_proc, no, NULL, 2),
  DEFPROC("eval", eval_proc, yes, NULL, 2),
};

void init_environment(lisp_object_t environment) {
  int len = sizeof(primitive_procs) / sizeof(struct lisp_object_t);
  for (int i = 0; i < len; i++) {
    sexp pp = &primitive_procs[i];
    add_primitive_proc(pp, environment);
  }
}
