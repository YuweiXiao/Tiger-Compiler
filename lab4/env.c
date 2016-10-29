#include <stdio.h>
#include "util.h"
#include "symbol.h"
#include "types.h"
#include "env.h"

/*Lab4: Your implementation of lab4*/
E_enventry E_VarEntry(Ty_ty ty) {
  E_enventry t = checked_malloc(sizeof(*t));
  t->kind = E_varEntry;
  t->u.var.ty = ty;
  return t;
}

E_enventry E_FunEntry(Ty_tyList formals, Ty_ty result) {
  E_enventry t = checked_malloc(sizeof(*t));
  t->kind = E_funEntry;
  t->u.func.formals = formals;
  t->u.func.result = result;
  return t; 
}

S_table E_base_tenv(void) {
  S_table env = S_empty();
  S_enter(env, S_Symbol("int"), E_VarEntry(Ty_Int()));
  S_enter(env, S_Symbol("string"), E_VarEntry(Ty_String()));
  return env;
}

// TODO, still more primitive function to add
S_table E_base_venv(void) {
  S_table env = S_empty();
  S_enter(env, S_Symbol("print"), 
            E_FunEntry(
                Ty_TyList(Ty_String(), NULL), 
                Ty_Nil())
          );
  S_enter(env, S_Symbol("ord"), 
            E_FunEntry(
                Ty_TyList(Ty_String(), NULL), 
                Ty_Int())
          );
  S_enter(env, S_Symbol("chr"), 
            E_FunEntry(
                Ty_TyList(Ty_String(), NULL), 
                Ty_Int())
          );
  S_enter(env, S_Symbol("getchar"), 
            E_FunEntry(
                NULL, 
                Ty_String())
          );
  return env;
}
