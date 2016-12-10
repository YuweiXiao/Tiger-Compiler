#include <stdio.h>
#include "util.h"
#include "symbol.h"
#include "temp.h"
#include "tree.h"
#include "absyn.h"
#include "assem.h"
#include "frame.h"
#include "graph.h"
#include "color.h"
#include "liveness.h"
#include "regalloc.h"
#include "table.h"
#include "flowgraph.h"


void show(void* t) {
    Temp_temp m = (Temp_temp) t;
    printf("tmp:%d", getTempNum(m));
}

void rewriteProgram(F_frame f, Temp_tempList spills, AS_instrList il) {
    My_Temp_TempList mySpills = cloneFromTempList(spills);
    AS_instrList list = il, pre = NULL;
    TAB_table reg2access = TAB_empty();
    for(; il; il = il->tail) {
        AS_instr instr = il->head;
        if(instr->kind != I_LABEL) {
            Temp_tempList dst, src;
            if(instr->kind == I_MOVE) {
                printf("instr:%s\n", instr->u.MOVE.assem);
                dst = instr->u.MOVE.dst;
                src = instr->u.MOVE.src;
            } else {
                printf("instr:%s\n", instr->u.OPER.assem);
                dst = instr->u.OPER.dst;
                src = instr->u.OPER.src;
            }
            for(; src; src = src->tail) {
                if(findInMyTempList(mySpills, src->head) == TRUE) {
                    F_access access = TAB_look(reg2access, src->head);
                    assert(access);
                    Temp_temp r = Temp_newtemp();
                    printf("new load\n");
                    AS_instr newInstr = AS_Oper(
                            createString("LOAD `d0 <- M[`s0+%d]\n", F_accessOffset(access)),
                            Temp_TempList(r, NULL), Temp_TempList(F_FP(), NULL), NULL);
                    pre->tail = AS_InstrList(newInstr, pre->tail);
                    pre = pre->tail;
                    src->head = r;
                }
            }
            for(; dst; dst = dst->tail) {
                if(findInMyTempList(mySpills, dst->head) == TRUE) {
                    F_access access = F_allocLocal(f, TRUE);
                    TAB_enter(reg2access, dst->head, access);
                    printf("new move\n");
                    //MOVE(MEM(e1 + CONST), e2)
                    AS_instr newInstr = AS_Oper(createString("mov M[`s0+%d] <- `s1\n", F_accessOffset(access)),
                        NULL, Temp_TempList(F_FP(), Temp_TempList(dst->head, NULL)), NULL);
                    il->tail = AS_InstrList(newInstr, il->tail);
                    il = il->tail;
                }
            }
        }
        pre = il;
    }
}

struct RA_result RA_regAlloc(F_frame f, AS_instrList il) {
    G_graph flowGraph = FG_AssemFlowGraph(il, f);
    assert(flowGraph);
    struct Live_graph lg = Live_liveness(flowGraph);
    
    G_show(stdout, G_nodes(lg.graph), show);


    Temp_map initial = Temp_layerMap(Temp_empty(), F_preColored());
    Temp_tempList regs = F_registers();
    struct COL_result colorResult = COL_color(lg, initial, regs);

    

    if(colorResult.spills != NULL) {
        printf("rewrite\n");
        rewriteProgram(f, colorResult.spills, il);
        printf("rewrite complete\n");
        return RA_regAlloc(f, il);
    }  
    struct RA_result ret = {colorResult.coloring, il};
	return ret;
}

