#include <stdio.h>
#include "util.h"
#include "table.h"
#include "symbol.h"
#include "absyn.h"
#include "temp.h"
#include "tree.h"
#include "printtree.h"
#include "frame.h"
#include "translate.h"

static F_fragList fragList = NULL;

typedef struct patchList_ *patchList;
struct patchList_ {
    Temp_label *head;
    patchList tail;
};

struct Cx
{
    patchList trues;
    patchList falses;
    T_stm stm;    
};

struct Tr_exp_ {
	//Lab5: your code here
    enum {Tr_ex, Tr_nx, Tr_cx} kind;
    union {
        T_stm nx;
        T_exp exp;
        struct Cx cx;
    } u;
};

   // F_accessList list = F_formals(level->frame);
static Tr_access Tr_Access(Tr_level level, F_access access);
static Tr_accessList Tr_AccessList(Tr_access access, Tr_accessList accessList);
static Tr_exp Tr_Ex(T_exp ex);
static Tr_exp Tr_Nx(T_stm ex);
static Tr_exp Tr_Cx(patchList trues, patchList falses, T_stm stm);
static patchList PatchList(Temp_label *head, patchList tail);
static T_exp unEx(Tr_exp e); 
static T_stm unNx(Tr_exp e); 
static struct Cx unCx(Tr_exp e); 

void doPatch(patchList tList, Temp_label label) {
    for(; tList != NULL; tList = tList->tail) 
        *(tList->head) = label;
}

patchList joinPatch(patchList first, patchList second) {
    if(!first) return second;
    patchList t = first;
    for(; t->tail != NULL; t = t->tail)
        ;
    t->tail = second;
    return first;
}


Tr_level Tr_outermost() {
    static Tr_level p = NULL;
    if(p == NULL) {
        p = checked_malloc(sizeof(*p));
        p->levelIndex = 0;
        p->frame = F_newFrame(Temp_namedlabel("tigermain"), NULL);
        p->accessList = NULL; 
    }
    return p;
}

Tr_level Tr_newLevel(Tr_level parent, Temp_label name, U_boolList formals) {
    Tr_level p = checked_malloc(sizeof(*p));
    p->levelIndex = parent->levelIndex + 1;
    formals = U_BoolList(TRUE, formals); // add static link
    p->frame = F_newFrame(name, formals); 
    p->parent = parent;
    p->accessList = NULL;
    //TODO check whether add static link into Tr_accessList
    F_accessList tList = F_formals(p->frame);
    Tr_accessList accessListHead = NULL, accessListTail = NULL;
    for(; tList; tList = tList->tail) {
        if(accessListHead == NULL) {
            accessListHead = accessListTail = Tr_AccessList(Tr_Access(p, tList->head), NULL);
        } else {
            accessListTail->tail = Tr_AccessList(Tr_Access(p, tList->head), NULL);
            accessListTail = accessListTail->tail;
        }
        // p->accessList = Tr_AccessList(Tr_Access(p, tList->head), p->accessList);
    }
    p->accessList = accessListHead;
    return p;
}

Tr_accessList Tr_formals(Tr_level level) {
    // remove static link
    return level->accessList->tail;
}

Tr_access Tr_allocLocal(Tr_level level, bool escape) {
    if(level == NULL) {
        printf("???\n");
    }
    F_access f_access =  F_allocLocal(level->frame, escape);
    Tr_access access = Tr_Access(level, f_access);
    level->accessList = Tr_AccessList(access, level->accessList);
    return access;
}

static Tr_access Tr_Access(Tr_level level, F_access access) {
    Tr_access p = checked_malloc(sizeof(*p));
    p->level = level;
    p->access = access;
    return p;
}

static Tr_accessList Tr_AccessList(Tr_access access, Tr_accessList accessList) {
    Tr_accessList p = checked_malloc(sizeof(*p));
    p->head = access;
    p->tail = accessList;
    return p;
}


static Tr_exp Tr_Ex(T_exp ex) {
    Tr_exp p = checked_malloc(sizeof(*p));
    p->kind = Tr_ex;
    p->u.exp = ex;
    return p;
}

static Tr_exp Tr_Cx(patchList trues, patchList falses, T_stm stm) {
    Tr_exp p = checked_malloc(sizeof(*p));
    p->kind = Tr_cx;
    p->u.cx.trues = trues;
    p->u.cx.falses = falses;
    p->u.cx.stm = stm;
    return p;
}

static Tr_exp Tr_Nx(T_stm stm) {
    Tr_exp p = checked_malloc(sizeof(*p));
    p->kind = Tr_nx;
    p->u.nx = stm;
    return p;
}

static patchList PatchList(Temp_label *head, patchList tail) {
    patchList p = checked_malloc(sizeof(*p));
    p->head = head;
    p->tail = tail;
    return p;
}

static T_exp unEx(Tr_exp e) {
    if(e == NULL)
        return NULL;
    switch(e->kind) {
        case Tr_ex:
            return e->u.exp;
        case Tr_nx:
            return T_Eseq(e->u.nx, T_Const(0));
        case Tr_cx: {
            Temp_temp r = Temp_newtemp();
            Temp_label t = Temp_newlabel(), f = Temp_newlabel();
            doPatch(e->u.cx.trues, t);
            doPatch(e->u.cx.falses, f);
            return T_Eseq(T_Move(T_Temp(r), T_Const(1)), 
                    T_Eseq(e->u.cx.stm,
                        T_Eseq(T_Label(f),
                            T_Eseq(T_Move(T_Temp(r), T_Const(0)),
                                T_Eseq(T_Label(t), 
                                    T_Temp(r))))));
        }
    }
    printf("unEx::error : can not reach here.\n");
}

static T_stm unNx(Tr_exp e) {
    if(e == NULL)
        return NULL;
    switch(e->kind) {
        case Tr_ex:
            return T_Exp(e->u.exp);
        case Tr_nx:
            return e->u.nx;
        case Tr_cx: {
            Temp_label t = Temp_newlabel();
            doPatch(e->u.cx.trues, t);
            doPatch(e->u.cx.falses, t);
            return e->u.cx.stm;
        }
    }
    printf("unNx::error: can not reach here.\n");
}

static struct Cx unCx(Tr_exp e) {
    if(e == NULL) {
        printf("in un cx, e is null\n");
    }
    switch(e->kind) {
        case Tr_ex: {
            T_stm s1 = T_Cjump(T_ne, e->u.exp, T_Const(0), NULL, NULL);
            struct Cx cx;
            cx.trues = PatchList(&s1->u.CJUMP.true, NULL);
            cx.falses = PatchList(&s1->u.CJUMP.false, NULL);
            cx.stm = s1;
            return cx;
        }
        case Tr_cx: 
            return e->u.cx;
        case Tr_nx:
        default:
            printf("unCx::error: can not reach here.\n");           
    }
}


Tr_exp Tr_simpleVar(Tr_access access, Tr_level currentLevel) {
    Tr_level targetLevel = access->level;
    T_exp current = T_Temp(F_FP());
    int i = targetLevel->levelIndex;
    for(;i < currentLevel->levelIndex; ++i) {
        current = F_Exp(F_formals(currentLevel->frame)->head, current);
        currentLevel = currentLevel->parent;
    }
    T_exp ex = F_Exp(access->access, current);
    return Tr_Ex(ex);
}


Tr_exp Tr_arrayVar(Tr_exp simpleVar, Tr_exp subscript) {
    return Tr_Ex(T_Mem(T_Binop(T_plus, 
                        unEx(simpleVar),
                        T_Binop(T_mul, T_Const(F_wordSize), unEx(subscript)))));
}

Tr_exp Tr_fieldVar(Tr_exp simpleVar, int index) {
    return Tr_Ex(T_Mem(T_Binop(T_plus, 
                        unEx(simpleVar),
                        T_Binop(T_mul, T_Const(F_wordSize), T_Const(index)))));
}

Tr_exp Tr_opExp(A_oper a_oper, Tr_exp left, Tr_exp right) {
    T_binOp t_binOp;
    switch(a_oper) {
        case A_plusOp:
            t_binOp = T_plus;
            break;
        case A_minusOp:
            t_binOp = T_minus;
            break;
        case A_timesOp:
            t_binOp = T_mul;
            break;
        case A_divideOp:
            t_binOp = T_div;
            break;
        default:
            printf("Tr_opExp :: unknown op\n");
    }
    return Tr_Ex(T_Binop(t_binOp, unEx(left), unEx(right)));
}


Tr_exp Tr_conditionOpExp(A_oper a_oper, Tr_exp left, Tr_exp right) {
    T_binOp t_binOp;
    switch(a_oper) {
        case A_eqOp:
            t_binOp = T_eq;
            break;
        case A_neqOp:
            t_binOp = T_ne;
            break;
        case A_ltOp:
            t_binOp = T_lt;
            break;
        case A_leOp:
            t_binOp = T_le;
            break;
        case A_gtOp:
            t_binOp = T_gt;
            break;
        case A_geOp:
            t_binOp = T_ge;
            break;
        default:
            printf("Tr_opExp :: unknown op\n");
    }
    T_stm stm = T_Cjump(t_binOp, unEx(left), unEx(right), NULL, NULL);
    patchList trues = PatchList(&stm->u.CJUMP.true, NULL);
    patchList falses = PatchList(&stm->u.CJUMP.false, NULL);
    return Tr_Cx(trues, falses, stm);
}


// TODO , process Cx expression espcially
Tr_exp Tr_ifExp(Tr_exp condition, Tr_exp true_exp, Tr_exp false_exp) {
    Temp_temp r = Temp_newtemp();
    Temp_label t = Temp_newlabel(), f = Temp_newlabel();
    
    struct Cx cx = unCx(condition);
    doPatch(cx.trues, t);
    doPatch(cx.falses, f);

    Temp_labelList final = Temp_LabelList(Temp_newlabel(), NULL);
    if(true_exp->kind == Tr_nx && false_exp->kind == Tr_nx) {
        
        return Tr_Nx(T_Seq(cx.stm,
                    T_Seq(T_Label(t),
                        T_Seq(true_exp->u.nx,
                            T_Seq(T_Jump(T_Const(0), final),
                                T_Seq(T_Label(f),
                                    T_Seq(false_exp->u.nx,
                                        T_Label(final->head))))))));
    }
    return Tr_Ex(T_Eseq(cx.stm, 
                    T_Eseq(T_Label(t),
                        T_Eseq(T_Move(T_Temp(r), unEx(true_exp)),
                            T_Eseq(T_Jump(T_Const(0), final),
                                T_Eseq(T_Label(f),
                                    T_Eseq(T_Move(T_Temp(r), unEx(false_exp)),
                                        T_Eseq(T_Label(final->head), 
                                            T_Temp(r)))))))));
}


Tr_exp Tr_intExp(int i) {
    return Tr_Ex(T_Const(i));
}

Tr_exp Tr_seqExp(Tr_exp e1, Tr_exp e2) {
    return Tr_Ex(T_Eseq(unNx(e1), unEx(e2)));
}

Tr_exp Tr_seqStm(Tr_exp e1, Tr_exp e2) {
    return Tr_Nx(T_Seq(unNx(e1), unNx(e2)));
}

F_fragList Tr_getResult() {
    return fragList;
}

Tr_exp Tr_stringExp(string str) {
    Temp_label label = Temp_newlabel();
    string tlabel = Temp_labelstring(label);
    char buf[100];
    sprintf(buf, ".%s", tlabel);
    printf("-----------------------------------------------------------%s\n", buf);
    label = Temp_namedlabel(String(buf));
    F_frag strFrag = F_StringFrag(label, str);
    fragList = F_FragList(strFrag, fragList);
    return Tr_Ex(T_Name(label));
}


Tr_exp Tr_arrayExp(Tr_exp size, Tr_exp value) {
    // alloc space for array, register r is array address
    Temp_temp r = Temp_newtemp();
    Temp_temp s = Temp_newtemp(); 
    Temp_temp fullSize = Temp_newtemp();
    T_stm getSize = T_Move(T_Temp(s), unEx(size));  // calculate size, put it into register s
    T_stm getFullSize = T_Move(T_Temp(fullSize), T_Binop(T_mul, T_Temp(s), T_Const(4)));
    T_stm alloc = T_Seq(getSize, 
                        T_Seq(getFullSize, 
                            T_Seq( T_Exp(T_Call( T_Name(Temp_namedlabel("tMalloc"))
                                                                        , T_ExpList(T_Temp(fullSize), NULL))),
                                    T_Move(T_Temp(r), T_Temp(F_RV())))));
    
    Temp_temp i = Temp_newtemp();               // loop i
    Temp_temp v = Temp_newtemp();               // init value
    Temp_temp tmpR = Temp_newtemp();            // address
    T_stm initLoop = T_Move(T_Temp(i), T_Const(0));
    T_stm getValue = T_Move(T_Temp(v), unEx(value));
    T_stm initR = T_Move(T_Temp(tmpR), T_Temp(r));

    T_stm initTest = T_Cjump(T_lt, T_Temp(i), T_Temp(s), NULL, NULL);
    Tr_exp cx = Tr_Cx(PatchList(&initTest->u.CJUMP.true, NULL), PatchList(&initTest->u.CJUMP.false, NULL), initTest);

    
    T_stm body = T_Seq(T_Move(T_Mem(T_Temp(tmpR)), T_Temp(v)),
                    T_Seq(T_Move(T_Temp(i), T_Binop(T_plus, T_Temp(i), T_Const(1))),
                        T_Move(T_Temp(tmpR), T_Binop(T_plus, T_Temp(tmpR), T_Const(F_wordSize))))); 

    Temp_label done = Temp_newlabel();
    Tr_exp loop = Tr_LoopExp(cx, Tr_Nx(body), done);
    T_stm init = T_Seq(initLoop, T_Seq(getValue, T_Seq(initR, unNx(loop))));
    
    
    return Tr_Ex(T_Eseq(T_Seq(alloc, init), T_Temp(r)));
}


Tr_exp Tr_commbineAllocInitReturn(Tr_exp alloc, Tr_exp init, Tr_exp r) {
    return Tr_Ex(T_Eseq(T_Seq(unNx(alloc), unNx(init)), unEx(r)));
}

Tr_exp Tr_allocMem(Tr_exp r, Tr_exp size) {
    T_stm alloc = T_Move(unEx(r), F_externalCall("tMalloc", T_ExpList(unEx(size), NULL)));
    return Tr_Nx(alloc);
}

Tr_exp Tr_break(Temp_label done) {
    T_stm stm = T_Jump(T_Const(0), Temp_LabelList(done, NULL));
    return Tr_Nx(stm);
}

Tr_exp Tr_newTemp() {
    Temp_temp r = Temp_newtemp();
    return Tr_Ex(T_Temp(r));
}

Tr_exp Tr_initHeapVariable(Tr_exp temp, int index, Tr_exp value, Tr_exp seq) {
    T_stm stm = T_Move(T_Mem(T_Binop(T_plus, unEx(temp), T_Const(F_wordSize*index))),
                       unEx(value));
    if(seq == NULL) {
        return Tr_Nx(stm);
    } else {
        return Tr_Nx(T_Seq(stm, unNx(seq)));
    }
}

Tr_exp Tr_LoopExp(Tr_exp condition, Tr_exp body, Temp_label done) {
    Temp_label test = Temp_newlabel();
    Temp_label bodyLabel = Temp_newlabel();
    // Temp_label done = Temp_newlabel();
    Temp_labelList testLabel = Temp_LabelList(test, NULL);
    // printf("here\n");
    struct Cx cond = unCx(condition);
    doPatch(cond.trues, bodyLabel);
    doPatch(cond.falses, done);
    return Tr_Nx(T_Seq(T_Label(test),
                    T_Seq(cond.stm,
                        T_Seq(T_Label(bodyLabel),
                            T_Seq(unNx(body),
                                T_Seq(T_Jump(T_Const(0), testLabel), 
                                    T_Label(done)))))));
}


Tr_exp Tr_callExp(Temp_label name, Tr_level currentLevel, Tr_level funcLevel, T_expList formals) {
    T_exp staticLink = T_Temp(F_FP());
    if(currentLevel->levelIndex >= funcLevel->levelIndex) {
        int index = currentLevel->levelIndex;
        for(; index >= funcLevel->levelIndex; --index) {
            // outermost do not have static link
            if(index == funcLevel->levelIndex && index == 0)
                break;
            // printf("here %d\n", funcLevel->levelIndex);
            staticLink = F_Exp(F_formals(currentLevel->frame)->head, staticLink);
            currentLevel = currentLevel->parent;
            // staticLink = T_Mem(T_Binop(T_plus, staticLink, T_Const(0)));
            // printf("ok %d\n", index);
        }
    }
    if(funcLevel->levelIndex > 0)
        return Tr_Ex(T_Call(T_Name(name), T_ExpList(staticLink, formals)));
    else 
        return Tr_Ex(T_Call(T_Name(name), formals));
}


T_expList Tr_addExpIntoList(T_expList expList, Tr_exp exp) {
    if(expList == NULL)
        return T_ExpList(unEx(exp), NULL);
    while(expList->tail != NULL) {
        expList = expList->tail;
    }
    expList->tail = T_ExpList(unEx(exp), NULL);
    return expList->tail;
}


Tr_exp Tr_assignExp(Tr_exp left, Tr_exp right) {
    return Tr_Nx(T_Move(unEx(left), unEx(right)));
}

Tr_exp Tr_initVariable(Tr_access access, Tr_exp exp) {
    return Tr_Nx(T_Move(F_Exp(access->access, T_Temp(F_FP())), unEx(exp)));
}

Tr_exp Tr_no_opExp() {
    return Tr_Ex(T_Const(0));
}


void   Tr_procFrag(Tr_exp body, Tr_level level) {
    // printf("here1111\n");
    T_stm mov = T_Move(T_Temp(F_RV()), unEx(body));
    fragList = F_FragList(F_ProcFrag(mov, level->frame), fragList);
}
