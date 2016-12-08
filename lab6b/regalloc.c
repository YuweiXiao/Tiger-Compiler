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

struct RA_result RA_regAlloc(F_frame f, AS_instrList il) {
    G_graph flowGraph = FG_AssemFlowGraph(il, f);
    assert(flowGraph);
    struct Live_graph lg = Live_liveness(flowGraph);
    Temp_map initial = Temp_layerMap(Temp_empty(), F_preColored());
    Temp_tempList regs = F_registers();
    struct COL_result colorResult = COL_color(lg, initial, regs);

    // G_show(stdout, G_nodes(lg.graph), show);

    if(colorResult.spills != NULL) {
        // rewriteProgram(f, spill, il)
        return RA_regAlloc(f, il);
    }  
    struct RA_result ret = {colorResult.coloring, il};
	return ret;
}

