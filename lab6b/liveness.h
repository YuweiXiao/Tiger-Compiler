#ifndef LIVENESS_H
#define LIVENESS_H

typedef struct Live_moveList_ *Live_moveList;
struct Live_moveList_ {
	G_node src, dst;
	Live_moveList tail;
};

Live_moveList Live_MoveList(G_node src, G_node dst, Live_moveList tail);

typedef struct My_Live_moveList_ *My_Live_moveList;
struct My_Live_moveList_
{
    int length;
    Live_moveList head;
    Live_moveList tail;
};

// make a empty new My_Live_moveList
My_Live_moveList My_Empty_Live_moveList();
// construct My_Live_moveList from Live_moveList
My_Live_moveList cloneFromLiveMoveList(Live_moveList list);
// clone a My_Live_moveList
My_Live_moveList cloneMyLiveMoveList(My_Live_moveList t1);
// append a Temp_temp in the end of list
void appendMyLiveMoveList(My_Live_moveList list, G_node src, G_node dst);
// append a Temp_temp in the end of list, if src->dst is already exist in list, then do nothing
void checkedAppendMyLiveMoveList(My_Live_moveList list, G_node src, G_node dst);
// find element in My_Live_moveList, return TRUE if t exists, FALSE ow.
int findInMyLiveMoveList(My_Live_moveList list, G_node src, G_node dst);
// determine whether t1, t2 is equal
int isEqualMyLiveMoveList(My_Live_moveList t1, My_Live_moveList t2);
// determine whether My_Live_moveList is empty
bool emptyMyLiveMoveList(My_Live_moveList t1);
// return My_Live_moveList equal to t1 union t2
My_Live_moveList unionMyLiveMoveList(My_Live_moveList t1, My_Live_moveList t2);
// return My_Live_moveList equal to t1 subtract t2
My_Live_moveList subMyLiveMoveList(My_Live_moveList t1, My_Live_moveList t2);
// pop first element from My_Live_moveList 
void popMyLiveMoveList(My_Live_moveList t1);


struct Live_graph {
	G_graph graph;
	Live_moveList moves;
};

Temp_temp Live_gtemp(G_node n);

struct Live_graph Live_liveness(G_graph flow);

#endif
