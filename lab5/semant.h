#include "translate.h"

struct expty {
  Tr_exp exp;
  Ty_ty ty;
};

F_fragList SEM_transProg(A_exp exp);

struct expty transVar(S_table venv, S_table tenv, A_var v, Tr_level level);
struct expty transExp(S_table venv, S_table tenv, A_exp a, Tr_level level);
Tr_exp       transDec(S_table venv, S_table tenv, A_dec d, Tr_level level);
Ty_ty transTy (              S_table tvec, A_ty  a);
