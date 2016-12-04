/*Lab4: Your implementation of lab4*/
#ifndef ENV_H
#define ENV_H
#include "types.h"
#include "translate.h"

typedef struct E_enventry_ *E_enventry;

struct E_enventry_ {
	enum {E_varEntry, E_funEntry} kind;
	union { 
		struct { Tr_access access; Ty_ty ty; } var;
		struct { 
			Tr_level level;
			Temp_label label;
			Ty_tyList formals; 
			Ty_ty result;
		} func; 
	} u; 
};

E_enventry E_VarEntry(Tr_access access, Ty_ty);
E_enventry E_FunEntry(Tr_level level, Temp_label label, Ty_tyList formals, Ty_ty result);

// E_enventry E_VarEntry(Ty_ty);
// E_enventry E_FunEntry(Ty_tyList formals, Ty_ty result);

S_table E_base_tenv(void);
S_table E_base_venv(void);

#endif
