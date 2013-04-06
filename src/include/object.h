#ifndef OBJECT_H
#define OBJECT_H

#include <stdio.h>

#include "types.h"

#define environment_vars(x) pair_caar(x)
#define environment_vals(x) pair_cdar(x)
#define enclosing_environment(x) pair_cdr(x)

extern hash_table_t symbol_table;

extern sexp global_env;
extern sexp null_environment;
extern sexp repl_environment;
extern sexp startup_environment;

extern sexp scm_err_port;
extern sexp scm_in_port;
extern sexp scm_out_port;

extern struct lisp_object_t *objects_heap;
extern sexp root;

/* extern void reclaim(sexp); */
extern void trigger_gc(void);

extern sexp make_close_object(void);
extern sexp make_dot_object(void);
extern sexp make_empty_list(void);
extern sexp make_eof_object(void);
extern sexp make_false(void);
extern sexp make_true(void);
extern sexp make_undefined(void);

extern sexp make_fixnum(int);
extern sexp make_character(char);

extern sexp make_string(char *);
extern sexp make_pair(sexp, sexp);
extern sexp make_symbol(char *);
extern sexp make_file_in_port(FILE *);
extern sexp make_file_out_port(FILE *);
extern sexp make_flonum(float);
extern sexp make_primitive_proc(C_proc_t);
extern sexp make_lambda_procedure(sexp, sexp, sexp);
extern sexp make_compiled_proc(sexp, sexp, sexp);
extern sexp make_vector(unsigned int);
extern sexp make_return_info(sexp, int, sexp);
extern sexp make_macro_procedure(sexp, sexp, sexp);
/* extern sexp make_string_in_port(char *); */
extern sexp make_wchar(void);
extern sexp make_wstring(char *);

extern sexp make_list(sexp e, ...);
extern sexp nconc_pair(sexp, sexp);
extern int pair_length(sexp);
extern sexp pair_nthcdr(sexp, int);

extern sexp read_byte(sexp);
extern sexp read_char(sexp);

extern int is_self_eval(sexp);

extern hash_table_t make_hash_table(hash_fn_t, comp_fn_t, unsigned int);

extern hash_table_t make_symbol_table(void);
extern sexp find_or_create_symbol(char *);

extern sexp extend_environment(sexp, sexp, sexp);
extern sexp make_startup_environment(void);
extern sexp make_global_env(void);
extern sexp make_repl_environment(void);
extern int is_empty_environment(sexp);
extern sexp make_empty_environment(void);
extern int search_binding_index(sexp, sexp, int *, int *);
extern void add_binding(sexp, sexp, sexp);
extern void set_binding(sexp, sexp, sexp);
extern sexp get_variable_value(sexp, sexp);

extern struct lisp_object_t *init_heap(void);
extern void dec_ref_count(sexp);
extern void inc_ref_count(sexp);

#endif
