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

void printTemp_tempList(Temp_tempList list) {
    for(; list; list = list->tail) {
        printf("reg number:%d\n", getTempNum(list->head));
    }
}

void rewriteProgram(F_frame f, Temp_tempList spills, AS_instrList il) {
    My_Temp_TempList mySpills = cloneFromTempList(spills);
    AS_instrList list = il, pre = NULL;
    TAB_table reg2access = TAB_empty();

    Temp_tempList tSpills = spills;
    for(; tSpills; tSpills = tSpills->tail) {
        F_access access = TAB_look(reg2access, tSpills->head);
        assert(access == NULL);
        access = F_allocLocal(f, TRUE);
        TAB_enter(reg2access, tSpills->head, access);
    }

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
            printf("src:\n");
            printTemp_tempList(src);
            printf("dst:\n");
            printTemp_tempList(dst);
            for(; src; src = src->tail) {
                F_access access = TAB_look(reg2access, src->head);
                if(access == NULL)
                    continue;
                printf("reg : number%d\n", getTempNum(src->head));
                Temp_temp r = Temp_newtemp();
                printf("new load\n");
                AS_instr newInstr = AS_Oper(
                        createString("movl %d(`s0), `d0\n", F_accessOffset(access)*F_wordSize),
                        Temp_TempList(r, NULL), Temp_TempList(F_FP(), NULL), NULL);
                src->head = r;
                pre->tail = AS_InstrList(newInstr, pre->tail);
                pre = pre->tail;
            }
            for(; dst; dst = dst->tail) {
                F_access access = TAB_look(reg2access, dst->head);
                if(access == NULL)
                    continue;
                printf("new move\n");
                //MOVE(MEM(e1 + CONST), e2)
                AS_instr newInstr = AS_Oper(createString("movl `s1, %d(`s0)\n", F_accessOffset(access)*F_wordSize),
                    NULL, Temp_TempList(F_FP(), Temp_TempList(dst->head, NULL)), NULL);
                il->tail = AS_InstrList(newInstr, il->tail);
                il = il->tail;
            }
        }
        pre = il;
    }
}

void removeUselessMoves(AS_instrList il, Temp_map color) {
    AS_instrList pre = NULL;
    for(; il; il = il->tail) {
        AS_instr instr = il->head;
        if(instr->kind == I_MOVE) {
            Temp_tempList dst = instr->u.MOVE.dst;
            Temp_tempList src = instr->u.MOVE.src;

            if(dst && src && dst->tail == NULL && src->tail == NULL) {
                if(Temp_look(color, dst->head) == Temp_look(color,src->head)) {
                    if(pre != NULL) {
                        pre->tail = il->tail;
                        continue;
                    }
                }
            }
        }
        pre = il;
    }
}

FILE* out = NULL;

void show(void* t) {
    Temp_temp m = (Temp_temp) t;
    fprintf(out, "tmp:%d", getTempNum(m));
}

void showflow(void* t) {
    NodeInfo m = (NodeInfo) t;
    AS_instr instr = m->instr;
    char *s;
    switch(instr->kind) {
        case I_LABEL:
            s = S_name(instr->u.LABEL.label);
        break;
        case I_MOVE:
            s = instr->u.MOVE.assem;
        break;
        case I_OPER:
            s = instr->u.OPER.assem;
        break;
    }
    fprintf(out, "instr:%s:", s);
}

struct RA_result RA_regAlloc(F_frame f, AS_instrList il) {
    G_graph flowGraph = FG_AssemFlowGraph(il, f);

    if(out == NULL)
        out = fopen("my.txt", "w");
    // G_show(out, G_nodes(flowGraph), NULL);
    // fprintf(out, "---------------------------------------------\n");
    assert(flowGraph);
    struct Live_graph lg = Live_liveness(flowGraph);

    G_show(out, G_nodes(lg.graph), show);

    Temp_map initial = Temp_layerMap(F_tempMap, F_preColored());
    Temp_tempList regs = F_registers();
    struct COL_result colorResult = COL_color(lg, initial, regs);

    // fprintf(out, "------------------------------\n");
    // AS_printInstrList(out, il, 
    //     Temp_layerMap(Temp_layerMap(colorResult.coloring, F_tempMap), Temp_name()));
    // fprintf(out, "--------------------------------!@#!@#!@#\n");

    if(colorResult.spills != NULL) {
        // printf("rewrite\n");
        rewriteProgram(f, colorResult.spills, il);
        // printf("rewrite complete\n");
        return RA_regAlloc(f, il);
    }  
    // removeUselessMoves(il, colorResult.coloring);
    struct RA_result ret = {colorResult.coloring, il};
	return ret;
}


