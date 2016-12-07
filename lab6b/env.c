#include <stdio.h>
#include "util.h"
#include "symbol.h"
#include "types.h"
#include "env.h"


/*Lab4: Your implementation of lab4*/
E_enventry E_VarEntry(Tr_access access, Ty_ty ty) {
	E_enventry t = checked_malloc(sizeof(*t));
	t->kind = E_varEntry;
	t->u.var.access = access;
	t->u.var.ty = ty;
	return t;
}

E_enventry E_FunEntry(Tr_level level, Temp_label label, Ty_tyList formals, Ty_ty result) {
	E_enventry t = checked_malloc(sizeof(*t));
	t->kind = E_funEntry;
	t->u.func.level = level;
	t->u.func.label = label;
	t->u.func.formals = formals;
	t->u.func.result = result;
	return t; 
}

// E_enventry E_VarEntry(Ty_ty ty) {
//   E_enventry t = checked_malloc(sizeof(*t));
//   t->kind = E_varEntry;
//   t->u.var.ty = ty;
//   return t;
// }

// E_enventry E_FunEntry(Ty_tyList formals, Ty_ty result) {
//   E_enventry t = checked_malloc(sizeof(*t));
//   t->kind = E_funEntry;
//   t->u.func.formals = formals;
//   t->u.func.result = result;
//   return t; 
// }

S_table E_base_tenv(void) {
	S_table env = S_empty();
	S_enter(env, S_Symbol("int"), E_VarEntry(NULL, Ty_Int()));
	S_enter(env, S_Symbol("string"), E_VarEntry(NULL, Ty_String()));
	return env;
}

// TODO, still more primitive function to add
S_table E_base_venv(void) {
  	S_table env = S_empty();
  	S_enter(env, S_Symbol("print"), 
			E_FunEntry(
				Tr_outermost(),
				Temp_namedlabel("print"),
				Ty_TyList(Ty_String(), NULL), 
				Ty_Void())
		  );
  	S_enter(env, S_Symbol("ord"), 
			E_FunEntry(
				Tr_outermost(),
				Temp_namedlabel("ord"),
				Ty_TyList(Ty_String(), NULL), 
				Ty_Int())
		  );
  	S_enter(env, S_Symbol("chr"), 
			E_FunEntry(
				Tr_outermost(),
				Temp_namedlabel("chr"),
				Ty_TyList(Ty_Int(), NULL), 
				Ty_String())
		  );
  	S_enter(env, S_Symbol("getchar"), 
			E_FunEntry(
				Tr_outermost(),
				Temp_namedlabel("getchar"),
				NULL, 
				Ty_String())
		  );
  	S_enter(env, S_Symbol("printi"), 
			E_FunEntry(
				Tr_outermost(),
				Temp_namedlabel("printi"),
				Ty_TyList(Ty_Int(), NULL), 
				Ty_Void())
		  );
  	return env;
}
