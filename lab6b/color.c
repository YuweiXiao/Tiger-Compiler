#include <stdio.h>
#include "util.h"
#include "symbol.h"
#include "temp.h"
#include "tree.h"
#include "absyn.h"
#include "assem.h"
#include "frame.h"
#include "graph.h"
#include "table.h"
#include "color.h"

/* register(color) number */
int K;

// node work-list, sets and stacks
Temp_map      precolored;
My_G_nodeList initial          ,            //temporary registers, not precolored and not yet processed
              simplifyWorklist ,
              freezeWorklist   ,
              spillWorklist    ,
              spilledNodes     ,
              coalescedNodes   ,
              coloredNodes     ,
              selectStack      ;

// Move sets
My_Live_moveList coalescedMoves   ,
                 constrainedMoves ,
                 frozenMoves      ,       
                 worklistMoves    ,     // moves possiable for coalescing
                 activeMoves      ;     // moves not yet ready for coalescing

// Others DS
int            *degree  = NULL;
My_G_bitMatrix adjSet   = NULL;        // G_node -> G_node
G_table        adjList  = NULL,
               moveList = NULL,        // G_node -> Live_moveList
               alias    = NULL,        // G_node -> G_node
               color    = NULL;        // G_node -> [TODO]color
Temp_map       colorMap;

/* function declearation */
void COL_makeWorklist();
My_Live_moveList COL_nodeMoves(G_node n);
bool COL_moveRelated(G_node n);    
void COL_decrementDegree(G_node n);        
void COL_enableMoves(My_G_nodeList list); 
void COL_simplify();                
void COL_coalesce();   
void COL_combine(G_node u, G_node v);
void COL_freeze();  
void COL_addWorkList(G_node n);
bool COL_OK(G_node t, G_node r);
bool COL_conservative(G_node u, G_node v);
G_node COL_getAlias(G_node); 
void COL_freezeMoves(G_node);
void COL_selectSpill(); 
void COL_assignColors(Temp_tempList);
void COL_addEdge(G_node u, G_node v); 
My_G_nodeList COL_adjacent(G_node n);
bool isPrecolored(G_node n);

My_Live_moveList G_lookupMoveList(G_node n) {
    My_Live_moveList t = G_look(moveList, n);
    if( t == NULL ) {
        t = My_Empty_Live_moveList();
        G_enter(moveList, n, t);
    }
    return t;
}

void COL_assignColors(Temp_tempList regs) {
    bool *okColor = checked_malloc(K*sizeof(bool));
    int i = 0;
    while(emptyMyGnodeList(selectStack) == FALSE) {
        G_node n = popMyGnodeList(selectStack);
        for(; i < K; ++i) {
            okColor[i] = TRUE;
        }
        G_nodeList adj = ((My_G_nodeList)G_look(adjList, n))->head;
        for(; adj; adj = adj->tail) {
            G_node w = COL_getAlias(adj->head);
            if(isPrecolored(w) || findInMyGnodeList(coloredNodes, w)) {
                okColor[(int)G_look(color, w)] = FALSE;
            }
        }
        int k = -1;
        Temp_temp reg;
        Temp_tempList tRegs = regs;
        for(i = 0; i < K; ++i, tRegs = tRegs->tail) {
            if(okColor[i] == TRUE) {
                reg = tRegs->head;
                k = i;
                break;
            }
        }
        if(k == -1) {
            appendMyGnodeList(spilledNodes, n);
        } else {
            appendMyGnodeList(coloredNodes, n);
            G_enter(color, n, k);
            Temp_enter(colorMap, Live_gtemp(n), Temp_look(precolored, reg));
        }
    }
    G_nodeList tList = coalescedNodes->head;
    for(; tList; tList = tList->tail) {
        Temp_enter(colorMap, Live_gtemp(tList->head), 
                    Temp_look(precolored, Live_gtemp(COL_getAlias(tList->head))));
    }
}

void COL_selectSpill() {
    G_nodeList tList = spillWorklist->head;
    G_node p = tList->head;
    for(tList = tList->tail; tList; tList = tList->tail) {
        if(degree[G_nodeKey(p)] < degree[G_nodeKey(tList->head)])
            p = tList->head;
    }

    spillWorklist = subMyGnodeList(spillWorklist, 
                        cloneFromGnodeList(G_NodeList(p, NULL)));
    checkedAppendMyGnodeList(simplifyWorklist, p);
    COL_freezeMoves(p);
}

void COL_freezeMoves(G_node u) {
    My_Live_moveList lists = COL_nodeMoves(u);
    Live_moveList tList = lists->head;
    for(; tList; tList = tList->tail) {
        G_node x = tList->src;
        G_node y = tList->dst;
        G_node v;
        if(COL_getAlias(y) == u) {
            v = COL_getAlias(x);
        } else {
            v = COL_getAlias(y);
        }
        activeMoves = subMyLiveMoveList(activeMoves, 
                            cloneFromLiveMoveList(Live_MoveList(x, y, NULL)));
        checkedAppendMyLiveMoveList(frozenMoves, x, y);
        if(emptyMyLiveMoveList(COL_nodeMoves(v)) == TRUE &&
            degree[G_nodeKey(v)] < K) {
            freezeWorklist =  subMyGnodeList(freezeWorklist, 
                                        cloneFromGnodeList(G_NodeList(v, NULL)));
            checkedAppendMyGnodeList(simplifyWorklist, v);
        }
    }
}   


void COL_freeze() {
    G_node u = popMyGnodeList(freezeWorklist);
    checkedAppendMyGnodeList(simplifyWorklist, u);
    COL_freezeMoves(u);
}


void COL_addEdge(G_node u, G_node v) {
    if(My_G_bitMatrixIsConnect(adjSet, G_nodeKey(u), G_nodeKey(v)) == FALSE
        && u != v) {
        My_G_bitMatrixAdd(adjSet, G_nodeKey(u), G_nodeKey(v));
        My_G_bitMatrixAdd(adjSet, G_nodeKey(v), G_nodeKey(u));
        if(isPrecolored(u) == FALSE) {
            checkedAppendMyGnodeList(G_look(adjList, u), v);
            degree[G_nodeKey(u)] += 1;
        } 
        if(isPrecolored(v) == FALSE) {
            checkedAppendMyGnodeList(G_look(adjList, v), u);
            degree[G_nodeKey(v)] += 1;
        }
    }
}

void COL_combine(G_node u, G_node v) {
    if(findInMyGnodeList(freezeWorklist, v)) {
        freezeWorklist = subMyGnodeList(freezeWorklist, 
                            cloneFromGnodeList(G_NodeList(v, NULL)));
    } else {
        spillWorklist = subMyGnodeList(spillWorklist, 
                            cloneFromGnodeList(G_NodeList(v, NULL)));
    }
    checkedAppendMyGnodeList(coalescedNodes, v);
    G_enter(alias, v, u);
    My_Live_moveList l = G_lookupMoveList(u);
    G_enter(moveList, u, unionMyLiveMoveList(l, G_lookupMoveList(v)));
    My_G_nodeList lists = COL_adjacent(v);
    G_nodeList tList = lists->head;
    for(; tList; tList = tList->tail) {
        COL_addEdge(tList->head, u);
        COL_decrementDegree(tList->head);
    }
    if(degree[G_nodeKey(u)] >= K && findInMyGnodeList(freezeWorklist, u)) {
        freezeWorklist = subMyGnodeList(freezeWorklist, 
                                cloneFromGnodeList(G_NodeList(u, NULL)));
        checkedAppendMyGnodeList(spillWorklist, u);
    }
}

bool isPrecolored(G_node n) {
    if(Temp_look(precolored, Live_gtemp(n)) == NULL)
        return FALSE;
    return TRUE;
}


G_node COL_getAlias(G_node m) {
    assert(m);
    return G_look(alias, m);
}

void COL_addWorkList(G_node n) {
    if(isPrecolored(n) == FALSE  && COL_moveRelated(n) == FALSE 
            && degree[G_nodeKey(n)] < K) {
        freezeWorklist = subMyGnodeList(freezeWorklist, cloneFromGnodeList(G_NodeList(n, NULL)));
        appendMyGnodeList(simplifyWorklist, n);
    }
}

bool COL_OK(G_node t, G_node r) {
    if(degree[G_nodeKey(t)] < K 
        || isPrecolored(t) == TRUE
        || My_G_bitMatrixIsConnect(adjSet, G_nodeKey(t), G_nodeKey(r)) == TRUE)
        return TRUE;
    return FALSE;
}

// u <- v
bool checkGeorge(G_node u, G_node v) {
    My_G_nodeList lists = COL_adjacent(v);
    G_nodeList tList = lists->head;
    for(; tList; tList = tList->tail) {
        if(COL_OK(tList->head, u) == FALSE)
            return FALSE;
    }
    return TRUE;
}

bool COL_conservative(G_node u, G_node v) {
    My_G_nodeList lists = unionMyGnodeList(COL_adjacent(u), COL_adjacent(v));
    int k = 0;
    G_nodeList tList = lists->head;
    for(; tList; tList = tList->tail) {
        if(degree[G_nodeKey(tList->head)] >= K)
            k += 1;
    }
    if(k < K)
        return TRUE;
    return FALSE;
}

void COL_coalesce() {
    G_node x = worklistMoves->head->src;
    G_node y = worklistMoves->head->dst;
    G_node u = COL_getAlias(x);
    G_node v = COL_getAlias(y);
    popMyLiveMoveList(worklistMoves);
    if(isPrecolored(v) == TRUE) {
        G_node t = u;
        u = v;
        v = t;
    }
    if(u == v) {
        appendMyLiveMoveList(coalescedMoves, x, y);
        COL_addWorkList(u);
    } else if(isPrecolored(v) == TRUE 
        || My_G_bitMatrixIsConnect(adjSet, G_nodeKey(u), G_nodeKey(v)) == TRUE) {
        checkedAppendMyLiveMoveList(constrainedMoves, x, y);
        COL_addWorkList(u);
        COL_addWorkList(v);
    } else if( (isPrecolored(u) == TRUE && checkGeorge(u, v))
                || (isPrecolored(u) == FALSE && COL_conservative(u, v)) ) {
        checkedAppendMyLiveMoveList(coalescedMoves, x, y);
        COL_combine(u, v);
        COL_addWorkList(u);
    } else {
        appendMyLiveMoveList(activeMoves, x, y);
    }
}

void COL_enableMoves(My_G_nodeList list) {
    G_nodeList tList = list->head;
    for(; tList; tList = tList->tail) {
        My_Live_moveList moves = COL_nodeMoves(tList->head);
        Live_moveList tMoves = moves->head;
        for(; tMoves; tMoves = tMoves->tail) {
            if(findInMyLiveMoveList(activeMoves, tMoves->src, tMoves->dst) == TRUE) {
                activeMoves = subMyLiveMoveList(activeMoves, 
                                cloneFromLiveMoveList(Live_MoveList(tMoves->src, tMoves->dst, NULL)));
                appendMyLiveMoveList(worklistMoves, tMoves->src, tMoves->dst);
            }
        }
    }
}

void COL_decrementDegree(G_node m) {
    int d = degree[G_nodeKey(m)];
    degree[G_nodeKey(m)] -= 1;
    if( d == K ) {
        My_G_nodeList adj = COL_adjacent(m);
        checkedAppendMyGnodeList(adj, m);
        COL_enableMoves(adj);
        spillWorklist = subMyGnodeList(spillWorklist, 
                                cloneFromGnodeList(G_NodeList(m, NULL)));
        if(COL_moveRelated(m)) {
            checkedAppendMyGnodeList(freezeWorklist, m);
        } else {
            checkedAppendMyGnodeList(simplifyWorklist, m);
        }
    }
}

My_G_nodeList COL_adjacent(G_node n) {
    My_G_nodeList adj = G_look(adjList, n);
    return subMyGnodeList(adj, unionMyGnodeList(selectStack, coalescedNodes));
}

void COL_simplify() {
    G_node n = popMyGnodeList(simplifyWorklist);
    insertFrontMyGnodeList(selectStack, n);
    My_G_nodeList adj = COL_adjacent(n);
    G_nodeList list = adj->head;
    for(; list; list = list->tail) {
        COL_decrementDegree(list->head);
    }
}


My_Live_moveList COL_nodeMoves(G_node n) {
    My_Live_moveList t = G_lookupMoveList(n);
    t = subMyLiveMoveList(t, unionMyLiveMoveList(activeMoves, worklistMoves));
    return t;
} 

bool COL_moveRelated(G_node n) {
    if(emptyMyLiveMoveList(COL_nodeMoves(n)) == TRUE)
        return FALSE;
    else
        return TRUE;    
}

void COL_makeWorklist() {
    G_nodeList list = initial->head;
    for(; list; list = list->tail) {
        if(degree[G_nodeKey(list->head)] >= K) {
            appendMyGnodeList(spillWorklist, list->head);
        } else if(COL_moveRelated(list->head) == TRUE) {
            appendMyGnodeList(freezeWorklist, list->head);
        } else {
            appendMyGnodeList(simplifyWorklist, list->head);
        }
    }
}

void init(int n, Temp_map tPrecolored, Temp_tempList registers) {
    precolored       = tPrecolored; 
    initial          = My_Empty_G_nodeList();
    simplifyWorklist = My_Empty_G_nodeList();
    freezeWorklist   = My_Empty_G_nodeList();
    spillWorklist    = My_Empty_G_nodeList();
    spilledNodes     = My_Empty_G_nodeList();
    coalescedNodes   = My_Empty_G_nodeList();
    coloredNodes     = My_Empty_G_nodeList();
    selectStack      = My_Empty_G_nodeList();
    coalescedMoves   = My_Empty_Live_moveList(),
    constrainedMoves = My_Empty_Live_moveList();
    frozenMoves      = My_Empty_Live_moveList();
    worklistMoves    = My_Empty_Live_moveList();     // moves possiable for coalescing
    activeMoves      = My_Empty_Live_moveList();
    adjSet   = My_G_BitMatrix(n);
    adjList  = G_empty();
    moveList = G_empty();        // G_node -> Live_moveList
    alias    = G_empty();        // G_node -> G_node
    color    = G_empty();
    colorMap = F_preColored();

    degree   = (int *)checked_malloc(sizeof(int) * n);
    int i = 0;
    for(; i < n; ++i)
        degree[i] = 0;

    K = 0;
    while(registers) {
        ++K;
        registers = registers->tail;
    }
}


void build(struct Live_graph lg) {
    G_graph ig = lg.graph;
    G_nodeList list = G_nodes(ig);
    // construct initial list  &  adjSet & alias
    while(list) {
        if(Temp_look(precolored, Live_gtemp(list->head)) == NULL) {
            appendMyGnodeList(initial, list->head);
        }
        degree[G_nodeKey(list->head)] = G_degree(list->head);
        G_enter(alias, list->head, list->head);
        G_enter(adjList, list->head, cloneFromGnodeList(G_adj(list->head)));
        G_nodeList succ = G_succ(list->head);
        while(succ) {
            My_G_bitMatrixAdd(adjSet, G_nodeKey(succ->head), G_nodeKey(list->head));
            My_G_bitMatrixAdd(adjSet, G_nodeKey(list->head), G_nodeKey(succ->head));
            succ = succ->tail;
        }
        list = list->tail;
    }

    worklistMoves = cloneFromLiveMoveList(lg.moves);

    // construct moveList
    Live_moveList tmoveList = lg.moves;
    while(tmoveList) {
        My_Live_moveList t1 = G_lookupMoveList(tmoveList->src);
        appendMyLiveMoveList(t1, tmoveList->src, tmoveList->dst);
        // TODO, check correctness, didn't enter back into moveList
        My_Live_moveList t2 = G_lookupMoveList(tmoveList->src);
        appendMyLiveMoveList(t2, tmoveList->src, tmoveList->dst);
        tmoveList = tmoveList->tail;
    }
}


void doWork(){
    while(!(emptyMyGnodeList(simplifyWorklist) == TRUE 
            && emptyMyLiveMoveList(worklistMoves) == TRUE 
            && emptyMyGnodeList(freezeWorklist) == TRUE
            && emptyMyGnodeList(spillWorklist) == TRUE)) {
        if(emptyMyGnodeList(simplifyWorklist) != TRUE) {
            printf("simplify\n");
            COL_simplify();
        }
        if(emptyMyLiveMoveList(worklistMoves) != TRUE) {
            printf("Coalesce\n");
            COL_coalesce();
        }
        if(emptyMyGnodeList(freezeWorklist) != TRUE) {
            printf("freeze\n");
            COL_freeze();
        }
        if(emptyMyGnodeList(spillWorklist) != TRUE) {
            printf("spill\n");
            COL_selectSpill();
        }
    }
}

struct COL_result COL_color(struct Live_graph lg, Temp_map tPrecolored, Temp_tempList regs) {
	struct COL_result ret = {NULL, NULL};
    init(G_nodesNumber(lg.graph), tPrecolored, regs);
    build(lg);
    COL_makeWorklist();
    doWork();
    COL_assignColors(regs);

    G_nodeList nodes = spilledNodes->head;
    for(; nodes; nodes = nodes->tail) {
        ret.spills = Temp_TempList(Live_gtemp(nodes->head), ret.spills);
    } 


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

