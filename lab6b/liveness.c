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



/* My_Live_MoveList functions */

// make a empty new My_Live_moveList
My_Live_moveList My_Empty_Live_moveList() {
    My_Live_moveList list = (My_Live_moveList)checked_malloc(sizeof *list);
    list->tail = list->head = NULL;
    list->length = 0;
    return list;
}

// construct My_Live_moveList from Live_moveList
My_Live_moveList cloneFromLiveMoveList(Live_moveList list) {
    My_Live_moveList t = My_Empty_Live_moveList();
    while(list) {
        appendMyLiveMoveList(t, list->src, list->dst);
        list = list->tail;
    }
    return t;
}

// clone a My_Live_moveList
My_Live_moveList cloneMyLiveMoveList(My_Live_moveList t1) {
    assert(t1);
    My_Live_moveList list = My_Empty_Live_moveList();
    Live_moveList now = t1->head;
    while(now) {
        appendMyLiveMoveList(list, now->src, now->dst);
        now = now->tail;
    }
    return list;
}

// append a Temp_temp in the end of list
void appendMyLiveMoveList(My_Live_moveList list, G_node src, G_node dst) {
    assert(list);   
    if(list->head == NULL) {
        list->head = list->tail = Live_MoveList(src, dst, NULL);
    } else {
        list->tail->tail = Live_MoveList(src, dst, NULL);
        list->tail = list->tail->tail;
    }
    list->length += 1;
}

// append a Temp_temp in the end of list, if src->dst is already exist in list, then do nothing
void checkedAppendMyLiveMoveList(My_Live_moveList list, G_node src, G_node dst) {
    if(findInMyLiveMoveList(list, src, dst) == FALSE)
        appendMyLiveMoveList(list, src, dst);
}

// find element in My_Live_moveList, return TRUE if t exists, FALSE ow.
int findInMyLiveMoveList(My_Live_moveList list, G_node src, G_node dst) {
    assert(list);
    Live_moveList now = list->head;
    while(now) {
        if(now->src == src && now->dst == dst) {
            return TRUE;
        }
        now = now->tail;
    }
    return FALSE;
}

// determine whether t1, t2 is equal
int isEqualMyLiveMoveList(My_Live_moveList t1, My_Live_moveList t2) {
    if(t1->length != t2->length) {
        return FALSE;
    }
    Live_moveList now = t2->head;
    while(now) {
        if(findInMyLiveMoveList(t1, now->src, now->dst) == FALSE) 
            return FALSE;
        now = now->tail;
    }
    return TRUE;
}
// return My_Live_moveList equal to t1 union t2
My_Live_moveList unionMyLiveMoveList(My_Live_moveList t1, My_Live_moveList t2) {
    assert(t1); assert(t2);
    My_Live_moveList ret = cloneMyLiveMoveList(t1);
    Live_moveList now = t2->head;
    while(now) {
        if(findInMyLiveMoveList(t1, now->src, now->dst) == FALSE) {
            appendMyLiveMoveList(ret, now->src, now->dst);
        }
        now = now->tail;
    }
    return ret;
}

// return My_Live_moveList equal to t1 subtract t2
My_Live_moveList subMyLiveMoveList(My_Live_moveList t1, My_Live_moveList t2) {
    assert(t1); assert(t2);
    My_Live_moveList ret = My_Empty_Live_moveList();
    Live_moveList now = t1->head;
    while(now) {
        if(findInMyLiveMoveList(t2, now->src, now->dst) == FALSE) {
            appendMyLiveMoveList(ret, now->src, now->dst);
        }
        now = now->tail;
    }
    return ret;
}

// determine whether My_Live_moveList is empty
bool emptyMyLiveMoveList(My_Live_moveList t1) {
	assert(t1);
	if(t1->head == NULL) 
		return TRUE;
	return FALSE;
}

// pop first element from My_Live_moveList 
void popMyLiveMoveList(My_Live_moveList t1) {
    if(emptyMyLiveMoveList(t1) == TRUE)
        return;
    t1->head = t1->head->tail;
    if(t1->head == NULL)
        t1->tail = NULL;
}
