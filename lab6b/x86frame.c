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

Temp_temp F_EAX() {
    static Temp_temp eax = NULL;
    if(eax == NULL) {
        eax = Temp_newtemp();
    }
    return eax;
}

Temp_temp F_EBX() {
    static Temp_temp t = NULL;
    if(t == NULL) {
        t = Temp_newtemp();
    }
    return t;
}

Temp_temp F_ECX() {
    static Temp_temp t = NULL;
    if(t == NULL) {
        t = Temp_newtemp();
    }
    return t;
}

Temp_temp F_EDX() {
    static Temp_temp t = NULL;
    if(t == NULL) {
        t = Temp_newtemp();
    }
    return t;
}

Temp_temp F_ESI() {
    static Temp_temp t = NULL;
    if(t == NULL) {
        t = Temp_newtemp();
    }
    return t;
}

Temp_temp F_EDI() {
    static Temp_temp t = NULL;
    if(t == NULL) {
        t = Temp_newtemp();
    }
    return t;
}

Temp_temp F_ESP() {
    static Temp_temp t = NULL;
    if(t == NULL) {
        t = Temp_newtemp();
    }
    return t;
}

Temp_temp F_EBP() {
    static Temp_temp t = NULL;
    if(t == NULL) {
        t = Temp_newtemp();
    }
    return t;
}

Temp_temp F_SP() {
    return F_ESP();
}

Temp_temp F_FP() {
    return F_EBP();
}

Temp_temp F_RV() {
    return F_EAX();
}

Temp_temp F_DivUP() {
    return F_EDX();
}
Temp_temp F_DivLOW() {
    return F_EAX();
}

Temp_tempList F_registers() {
    static Temp_tempList registers = NULL;
    if(registers == NULL) {
        registers = Temp_TempList(F_EAX(),
                    Temp_TempList(F_EBX(),
                    Temp_TempList(F_ECX(),
                    Temp_TempList(F_EDX(),
                    Temp_TempList(F_ESI(),
                    Temp_TempList(F_EDI(), NULL))))));
        // Temp_TempList(F_ESP(),
        //             Temp_TempList(F_EBP()
    }
    return registers;
}

Temp_map F_preColored() {
    static Temp_map t = NULL;
    if(t == NULL) {
        t = Temp_empty();
        Temp_enter(t, F_EAX(), "%eax");
        Temp_enter(t, F_EBX(), "%ebx");
        Temp_enter(t, F_ECX(), "%ecx");
        Temp_enter(t, F_EDX(), "%edx");
        Temp_enter(t, F_ESI(), "%esi");
        Temp_enter(t, F_EDI(), "%edi");
        Temp_enter(t, F_ESP(), "%esp");
        Temp_enter(t, F_EBP(), "%ebp");
    }
    return Temp_layerMap(F_tempMap, t);
}

Temp_tempList F_calleeSaves() {
    static Temp_tempList t = NULL;
    if(t == NULL) {
        t = Temp_TempList(F_EBX(),
            Temp_TempList(F_ESI(),
            Temp_TempList(F_EDI(), NULL)));
    }
    return t;
}

Temp_tempList F_callerSaves() {
    static Temp_tempList t = NULL;
    if(t == NULL) {
        t = Temp_TempList(F_EAX(),
            Temp_TempList(F_EDX(),
            Temp_TempList(F_ECX(),NULL)));
    }
    return t;   
}


T_exp F_Exp(F_access acc, T_exp framePtr) {
    if(acc->kind == inReg) {
        return T_Temp(acc->u.reg);
    } else {
        return T_Mem(T_Binop(T_plus, T_Const(acc->u.offset*F_wordSize), framePtr));
    }
}

F_frame F_newFrame(Temp_label name, U_boolList formals) {
    printf("new frame------------------------------------\n");
    F_frame p = checked_malloc(sizeof(*p));
    p->sp = 0;
    p->label = name;
    F_accessList accessList = NULL;
    U_boolList boolList = formals;
    F_accessList tail = NULL, tAccessList;
    int offset = 2;
    for(;boolList != NULL; boolList = boolList->tail) {
        if(boolList->head == TRUE) {
            tAccessList = F_AccessList(InFrame(offset++), NULL);
        } else {
            tAccessList = F_AccessList(InReg(Temp_newtemp()), NULL);
        }
        if(tail == NULL) {
            accessList = tail = tAccessList;
        } else {
            tail->tail = tAccessList;
            tail = tAccessList;
        }
    }
    p->accessList = accessList;
    // printf("out\n");
    return p;
}

F_access F_allocLocal(F_frame f, bool escape) {
    // printf("F_allocLocal start\n");
    F_access access = NULL;
    if(escape == TRUE) {
        f->sp -= 1;
        access = InFrame(f->sp);
    } else {
        access = InReg(Temp_newtemp());
    }
    // f->accessList = F_AccessList(access, f->accessList);
    return access;
}

int F_accessOffset(F_access access) {
    assert(access->kind == inFrame);
    return access->u.offset;
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
    p->u.stringg.label = label;
    p->u.stringg.str = str;
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
    // add calleessaves register move and restore
    Temp_tempList toMove = F_calleeSaves();
    My_Temp_TempList tempList = My_Empty_Temp_TempList();
    for(; toMove; toMove = toMove->tail) {
        Temp_temp t = Temp_newtemp();
        appendMyTempList(tempList, t);
        T_stm move = T_Move(T_Temp(t), T_Temp(toMove->head));
        stm = T_Seq(move, stm);
    }

    Temp_tempList temp = tempList->head;
    toMove = F_calleeSaves();
    for(; temp; temp = temp->tail, toMove = toMove->tail) {
        T_stm move = T_Move(T_Temp(toMove->head), T_Temp(temp->head));
        stm = T_Seq(stm, move);
    }

    return stm;
}


AS_instrList F_procEntryExit2(AS_instrList body) {
    static Temp_tempList returnSink = NULL;
    if(!returnSink) {
        returnSink = Temp_TempList(F_RV(),
                     Temp_TempList(F_FP(),
                     Temp_TempList(F_SP(), NULL)));
                     // Temp_TempList(F_SP(), F_calleeSaves())));
    }
    return AS_splice(body, AS_InstrList(
                    AS_Oper("", NULL, returnSink, NULL), NULL));
}
 

// indication
AS_proc F_procEntryExit3(F_frame frame, AS_instrList body) {
    char buf[1024];
    sprintf(buf, ".text\n.globl %s\n.type %s, @function\n %s:", 
                S_name(frame->label), S_name(frame->label), S_name(frame->label));
    AS_instr pushEBP = AS_Oper("pushl `s0\n", NULL, Temp_TempList(F_FP(), NULL), NULL);
    AS_instr moveESP = AS_Oper("movl `s0, `d0\n", Temp_TempList(F_FP(), NULL), Temp_TempList(F_SP(), NULL), NULL);
    AS_instr minusESP = AS_Oper(createString("addl $%d, `d0\n", (frame->sp - 3)*F_wordSize), 
                            Temp_TempList(F_SP(), NULL), NULL, NULL);
    AS_instr leave = AS_Oper("leave\n", NULL, NULL, NULL);
    AS_instr ret = AS_Oper("ret\n", NULL, NULL, NULL);
    body = AS_splice(AS_InstrList(pushEBP, AS_InstrList(moveESP, AS_InstrList(minusESP, NULL))), body);
    body = AS_splice(body, AS_InstrList(leave, AS_InstrList(ret, NULL)));
    return AS_Proc(String(buf), body, String("\n"));
}


T_exp F_externalCall(string s, T_expList args) {
    return T_Call(T_Name(Temp_namedlabel(s)), args);
}


void F_string(FILE *fp, F_frag frag) {
    char buf[200];
    fprintf(fp, "%s: .ascii \"", S_name(frag->u.stringg.label));

    int i = 0;
    int tSize = strlen(frag->u.stringg.str);
    string str = frag->u.stringg.str;
    int size = 0;
    for(i = 0; i < tSize; ++i) {
        if(str[i] == '\\') {
            if(str[i+1] == 'n' || str[i+1] == 't' || str[i+1] == '0' || str[i+1] == '\\') {
                i++;
            }
        }
        size += 1;
    }
    
    for(i = 0; i < 4; ++i) {
        fprintf(fp, "%c", (char)( (size >> (i * 8)) & 0xff));
    }
    fprintf(fp, "%s\"\n", frag->u.stringg.str);
}
