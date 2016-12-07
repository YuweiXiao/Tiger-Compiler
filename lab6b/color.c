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
#include "table.h"
#include "liveness.h"

// node work-list, sets and stacks
My_G_nodeList precolored       = NULL, 
              initial          = NULL,
              simplifyWorklist = NULL,
              freezeWorklist   = NULL,
              spillWorklist    = NULL,
              spilledNodes     = NULL,
              coalescedNodes   = NULL,
              coloredNodes     = NULL,
              selectStack      = NULL;

// Move sets
Live_moveList coalescedMoves    = NULL,
              constrainedMoves  = NULL,
              frozenMoves       = NULL,       
              worklistMoves     = NULL,     // moves possiable for coalescing
              activeMoves       = NULL;     // moves not yet ready for coalescing

// Others DS
G_table adjSet   = NULL,        // G_node -> G_node
        degree   = NULL,        // G_node -> int
        moveList = NULL,        // G_node -> Live_moveList
        alias    = NULL,        // G_node -> G_node
        color    = NULL;        // G_node -> [TODO]color
G_graph adjList  = NULL;


void build() {
    
}


struct COL_result COL_color(G_graph ig, Temp_map initial, Temp_tempList regs) {
	//your code here.
	struct COL_result ret = {NULL, NULL};
	return ret;
}


// void doWork() {
    
//     // G_show(stdout, G_nodes(lg.graph), show);
//     // build()
//     // makeWorkList()   // do initial works
//     while(!(simplifyWorklist == NULL && worklistMoves == NULL && 
//             freezeWorklist == NULL && spillWorklist == NULL)) {
//         if(simplifyWorklist != NULL)
//             //simplify()
//             ;
//         if(worklistMoves != NULL)
//             //Coalesce()
//             ;
//         if(freezeWorklist != NULL)
//             //Freeze()
//             ;
//         if(spillWorklist != NULL)
//             //selectSpill()
//             ;
//     }
//     // AssignColors()
//     if(spilledNodes != NULL) {
//         //rewriteProgram(spillNodes) 
//         //doWork()    
//     }
// }
