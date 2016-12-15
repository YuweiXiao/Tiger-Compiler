/*Lab5: This header file is not complete. Please finish it with more definition.*/

#ifndef FRAME_H
#define FRAME_H
#include "assem.h"

typedef struct F_frame_ *F_frame;
typedef struct F_access_* F_access;

typedef struct F_accessList_ *F_accessList;
struct F_accessList_ {
    F_access head;
    F_accessList tail;
};
F_accessList F_AccessList(F_access head, F_accessList tail);

/* declaration for fragments */
typedef struct F_frag_ *F_frag;
struct F_frag_ {
    enum {F_stringFrag, F_procFrag} kind;
  union {
    struct {Temp_label label; string str;} stringg;
    struct {T_stm body; F_frame frame;} proc;
  } u;
};

F_frag F_StringFrag(Temp_label label, string str);
F_frag F_ProcFrag(T_stm body, F_frame frame);

typedef struct F_fragList_ *F_fragList;
struct F_fragList_ {
    F_frag head; 
    F_fragList tail;
};
F_fragList F_FragList(F_frag head, F_fragList tail);


Temp_map F_tempMap;
Temp_tempList F_registers();
Temp_map F_preColored();
Temp_tempList F_calleeSaves();
Temp_tempList F_callerSaves();

F_frame F_newFrame(Temp_label name, U_boolList formals);
Temp_label F_name(F_frame f);
F_accessList F_formals(F_frame f);
F_access F_allocLocal(F_frame f, bool escape);
int F_accessOffset(F_access);
int F_frameMaxOffset(F_frame);

extern const int F_wordSize;
Temp_temp F_DivUP();
Temp_temp F_DivLOW();
Temp_temp F_FP(); 
Temp_temp F_SP(); 
Temp_temp F_RV();
T_exp F_Exp(F_access acc, T_exp framePtr);
T_stm F_procEntryExit1(F_frame frame, T_stm); //TODO
AS_instrList F_procEntryExit2(AS_instrList body); //TODO
AS_proc F_procEntryExit3(F_frame frame, AS_instrList); //TODO

T_exp F_externalCall(string s, T_expList args);
void F_string(FILE *out, F_frag);


#endif



