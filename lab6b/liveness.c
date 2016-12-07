#include <stdio.h>
#include "util.h"
#include "symbol.h"
#include "temp.h"
#include "tree.h"
#include "absyn.h"
#include "assem.h"
#include "frame.h"
#include "graph.h"
#include "flowgraph.h"
#include "liveness.h"
#include "table.h"

static void enterLiveMap(G_table t, G_node flowNode, My_Temp_TempList temps) {
	G_enter(t, flowNode, temps);
}

static My_Temp_TempList lookupLiveMap(G_table t, G_node flowNode) {
	My_Temp_TempList q = G_look(t, flowNode);
	if( q == NULL ) {
		return My_Empty_Temp_TempList();
	}
}

Live_moveList Live_MoveList(G_node src, G_node dst, Live_moveList tail) {
	Live_moveList lm = (Live_moveList) checked_malloc(sizeof(*lm));
	lm->src = src;
	lm->dst = dst;
	lm->tail = tail;
	return lm;
}

Temp_temp Live_gtemp(G_node n) {
	return G_nodeInfo(n);
}


static TAB_table temp2GnodeTable;
// Find G_node in TAB_table using Temp_temp
// If Temp_temp t is not found in table, create a new g_node in graph,
// and add it into table.
static G_node lookupTemp2GnodeTAB(G_graph graph, Temp_temp t) {
	G_node node = TAB_look(temp2GnodeTable, t);
	if(node == NULL) {
		node = G_Node(graph, t);
		TAB_enter(temp2GnodeTable, t, node);
	}
	return node;
}

void addEdge(G_graph g, Temp_temp u, Temp_temp v) {
	if(u == v)
		return;
	G_addEdge(lookupTemp2GnodeTAB(g, u), lookupTemp2GnodeTAB(g, v));
}

struct Live_graph Live_liveness(G_graph flow) {
	G_table liveInMap = G_empty();
	G_table liveOutMap = G_empty();
	// nodes tobe update, like a queue.
	My_G_nodeList edgeNodes =  cloneFromGnodeList(G_nodes(flow));
		
	temp2GnodeTable = TAB_empty();
	struct Live_graph lg = {G_Graph(), NULL};
	Temp_tempList tmp;

	// Construct live in & live out Map based on control flow graph
	// While the set is not empty, keep updating
	while(emptyMyGnodeList(edgeNodes) == FALSE) {
		// Get node from queue, get instr's <liveIn>, <liveOut>
		G_node curNode = popMyGnodeList(edgeNodes);
		My_Temp_TempList curNodeLiveOut = lookupLiveMap(liveOutMap, curNode);
		My_Temp_TempList curNodeLiveIn = lookupLiveMap(liveInMap, curNode);

		My_Temp_TempList use = cloneFromTempList(FG_use(curNode));
		My_Temp_TempList def = cloneFromTempList(FG_def(curNode));
		My_Temp_TempList liveIn = cloneMyTempList(curNodeLiveOut);

		// remove <use> from <liveIn> temperaily, so the <def> will not interfere with <use>
		if(FG_isMove(curNode) == TRUE) {
			liveIn = subMyTempList(liveIn, use);
			lg.moves = Live_MoveList(lookupTemp2GnodeTAB(lg.graph, FG_def(curNode)->head),
									 lookupTemp2GnodeTAB(lg.graph, FG_use(curNode)->head), lg.moves);
		}
		liveIn = unionMyTempList(liveIn, def);
		
		// construct interference graph
		Temp_tempList d = def->head;
		Temp_tempList l = liveIn->head;
		for(; d; d = d->tail) {
			for(; l; l = l->tail) {
				addEdge(lg.graph, d->head, l->head);
					// lg.interfereTemps = Live_InterfereTempList(
					// 		lookupTemp2GnodeTAB(lg.graph, l->head), 
					// 		lookupTemp2GnodeTAB(lg.graph, d->head), lg.interfereTemps);
			}
		}
				
		// calculate live in map based on live out map
		liveIn = unionMyTempList(use, subMyTempList(liveIn, def));
		// if live in map isn't equal to origin one, then update all pre nodes, 
		// ans add these pre nodes into updated node list
		if(isEqualMyTempList(liveIn, curNodeLiveIn) == FALSE) {
			enterLiveMap(liveInMap, curNode, liveIn);
			G_nodeList preNodes = G_pred(curNode);
			for(; preNodes != NULL; preNodes = preNodes->tail) {
				My_Temp_TempList preNodeLiveOut = lookupLiveMap(liveOutMap, preNodes->head);
				preNodeLiveOut = unionMyTempList(liveIn, preNodeLiveOut);
				enterLiveMap(liveOutMap, preNodes->head, preNodeLiveOut);
				if(findInMyGnodeList(edgeNodes, preNodes->head) == FALSE)
					appendMyGnodeList(edgeNodes, preNodes->head);
			}
		}
	}

	return lg;
}



