#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "util.h"
#include "symbol.h"
#include "temp.h"
#include "table.h"
#include "tree.h"
#include "frame.h"

/*Lab5: Your implementation here.*/
const int F_wordSize = 4;

static F_access InFrame(int offset);
static F_access InReg(Temp_temp reg);

struct F_frame_ {
    int sp;
    F_accessList accessList;
    Temp_label label;
};

struct F_access_ {
    enum {inFrame, inReg} kind;
    union {
        int offset;
        Temp_temp reg;
    } u;
};

Temp_temp F_FP() {
    static Temp_temp fp = NULL;
    if(fp == NULL) {
        fp = Temp_newtemp();
    }
    return fp;
}

T_exp F_Exp(F_access acc, T_exp framePtr) {
    if(acc->kind == inReg) {
        return T_Temp(acc->u.reg);
    } else {
        return T_Mem(T_Binop(T_plus, T_Const(acc->u.offset), framePtr));
    }
}

F_frame F_newFrame(Temp_label name, U_boolList formals) {
    // printf("new frame\n");
    F_frame p = checked_malloc(sizeof(*p));
    p->sp = F_wordSize;
    p->label = name;
    F_accessList accessList = NULL;
    U_boolList boolList = formals;
    for(;boolList != NULL; boolList = boolList->tail) {
        if(boolList->head == TRUE) {
            accessList = F_AccessList(InFrame(p->sp), accessList);
        } else {
            accessList = F_AccessList(InReg(Temp_newtemp()), accessList);
        }
        p->sp + F_wordSize;
    }
    p->accessList = accessList;
    return p;
}

F_access F_allocLocal(F_frame f, bool escape) {
    // printf("F_allocLocal start\n");
    F_access access = NULL;
    if(escape) {
        access = InFrame(f->sp);
        f->sp += F_wordSize;
    } else {
        access = InReg(Temp_newtemp());
    }
    f->accessList = F_AccessList(access, f->accessList);
    return access;
}

Temp_label F_name(F_frame f) {
    return f->label;
}

F_accessList F_formals(F_frame f) {
    return f->accessList;
}

F_frag F_StringFrag(Temp_label label, string str) {
	F_frag p = checked_malloc(sizeof(*p));
    p->kind = F_stringFrag;
    p->u.string.label = label;
    p->u.string.str = str;
    return p;
}

F_frag F_ProcFrag(T_stm body, F_frame frame) {
	F_frag p = checked_malloc(sizeof(*p));
    p->kind = F_procFrag;
    p->u.proc.body = body;
    p->u.proc.frame = frame;
    return p;
}

F_fragList F_FragList(F_frag head, F_fragList tail) {
	F_fragList p = checked_malloc(sizeof(*p));
    p->head = head;
    p->tail = tail;
    return p;
}

F_accessList F_AccessList(F_access head, F_accessList tail) {
    F_accessList p = checked_malloc(sizeof(*p));
    p->head = head;
    p->tail = tail;
    return p;
}


static F_access InFrame(int offset) {
    F_access p = checked_malloc(sizeof(*p));
    p->kind = inFrame;
    p->u.offset = offset;
    return p;
}

static F_access InReg(Temp_temp reg) {
    F_access p = checked_malloc(sizeof(*p));
    p->kind = inReg;
    p->u.reg = reg;
    return p;   
}

T_stm F_procEntryExit1(F_frame frame, T_stm stm) {
    return stm;
}

T_exp F_externalCall(string s, T_expList args) {
    return T_Call(T_Name(Temp_namedlabel(s)), args);
}
