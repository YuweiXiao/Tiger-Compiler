typedef void *Tr_exp;

struct expty {
  Tr_exp exp;
  Ty_ty ty;
};

void SEM_transProg(A_exp exp);

struct expty transVar(S_table venv, S_table tenv, A_var v);
struct expty transExp(S_table venv, S_table tenv, A_exp a);
void         transDec(S_table venv, S_table tenv, A_dec d);
Ty_ty transTy (              S_table tvec, A_ty  a);
