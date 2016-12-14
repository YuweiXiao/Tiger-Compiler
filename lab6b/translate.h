#ifndef TRANSLATE_H
#define TRANSLATE_H
#include "temp.h"
#include "tree.h"
#include "absyn.h"
#include "frame.h"
/* Lab5: your code below */

typedef struct Tr_exp_ *Tr_exp;

typedef struct Tr_access_ *Tr_access;
typedef struct Tr_level_ *Tr_level;
typedef struct Tr_accessList_ *Tr_accessList;
struct Tr_access_ {
    //Lab5: your code here
    Tr_level level;
    F_access access;
};
struct Tr_accessList_ {
    Tr_access head;
    Tr_accessList tail; 
};
struct Tr_level_ {
    //Lab5: your code here
    Tr_level parent;
    int levelIndex;
    F_frame frame;
    Tr_accessList accessList;
};

Tr_exp Tr_simpleVar(Tr_access access, Tr_level level);
Tr_exp Tr_arrayVar(Tr_exp, Tr_exp);
Tr_exp Tr_fieldVar(Tr_exp, int index);
Tr_exp Tr_opExp(A_oper, Tr_exp left, Tr_exp right);
Tr_exp Tr_assignExp(Tr_exp, Tr_exp);
Tr_exp Tr_conditionOpExp(A_oper, Tr_exp left, Tr_exp right);
Tr_exp Tr_ifExp(Tr_exp e1, Tr_exp e2, Tr_exp e3);
Tr_exp Tr_intExp(int i);
Tr_exp Tr_initVariable(Tr_access, Tr_exp);
Tr_exp Tr_seqStm(Tr_exp, Tr_exp);
Tr_exp Tr_seqExp(Tr_exp, Tr_exp);
Tr_exp Tr_stringExp(string str);
Tr_exp Tr_arrayExp(Tr_exp size, Tr_exp init);
Tr_exp Tr_commbineAllocInitReturn(Tr_exp alloc, Tr_exp, Tr_exp);
Tr_exp Tr_allocMem(Tr_exp r, Tr_exp size);
Tr_exp Tr_initVariable();
Tr_exp Tr_initHeapVariable(Tr_exp temp, int index, Tr_exp value, Tr_exp seq);
Tr_exp Tr_newTemp();
Tr_exp Tr_LoopExp(Tr_exp condition, Tr_exp body);
Tr_exp Tr_callExp(Temp_label, Tr_level, Tr_level, T_expList);
T_expList Tr_addExpIntoList(T_expList, Tr_exp);
void   Tr_procFrag(Tr_exp body, Tr_level level);
Tr_exp Tr_no_opExp();

void Tr_procEntryExit(Tr_level level, Tr_exp body, Tr_accessList formals);
F_fragList Tr_getResult();

Tr_level Tr_outermost();
Tr_level Tr_newLevel(Tr_level parent, Temp_label name, U_boolList formals);

Tr_accessList Tr_formals(Tr_level level);
Tr_access Tr_allocLocal(Tr_level level, bool escape);

#endif
