#ifndef GRAPH_H
#define GRAPH_H
/*
 * graph.h - Abstract Data Type (ADT) for directed graphs
 */

typedef struct G_graph_ *G_graph;  /* The "graph" type */
typedef struct G_node_ *G_node;    /* The "node" type */

typedef struct G_nodeList_ *G_nodeList;
struct G_nodeList_ { G_node head; G_nodeList tail;};


typedef struct My_G_nodeList_ *My_G_nodeList;
struct My_G_nodeList_ {
    G_nodeList head;
    G_nodeList tail;
};
// make a empty new My_G_NodeList
My_G_nodeList My_Empty_G_nodeList();
// construct My_G_NodeList from G_nodeList
My_G_nodeList cloneFromGnodeList(G_nodeList list);
// append a G_node in the end of list
void appendMyGnodeList(My_G_nodeList list, G_node node);
// append a G_node in the end of list, if the node is already exist, then do nothing
void checkedAppendMyGnodeList(My_G_nodeList list, G_node node);
// append a G_node in the front of list
void insertFrontMyGnodeList(My_G_nodeList list, G_node node);
// clone a My_G_NodeList
My_G_nodeList cloneMyGnodeList(My_G_nodeList t1);
// find element in My_G_NodeList, return TRUE if t exists, FALSE ow.
int findInMyGnodeList(My_G_nodeList list, G_node t);
// return My_G_NodeList equal to t1 union t2
My_G_nodeList unionMyGnodeList(My_G_nodeList t1, My_G_nodeList t2);
// return My_G_NodeList equal to t1 subtract t2
My_G_nodeList subMyGnodeList(My_G_nodeList t1, My_G_nodeList t2);
// determine whether My_Gnode_List is empty
bool emptyMyGnodeList(My_G_nodeList t1);
// pop first element from My_G_nodeList 
G_node popMyGnodeList(My_G_nodeList t1);


/* Make a new graph */
G_graph G_Graph(void); 
/* Make a new node in graph "g", with associated "info" */
G_node G_Node(G_graph g, void *info);

/* Get the key of node in graph*/
int G_nodeKey(G_node node);

/* Make a NodeList cell */
G_nodeList G_NodeList(G_node head, G_nodeList tail);

/* Get the list of nodes belonging to "g" */
G_nodeList G_nodes(G_graph g);

/* Get the number of graph's nodes*/
int G_nodesNumber(G_graph g);

/* Tell if "a" is in the list "l" */
bool G_inNodeList(G_node a, G_nodeList l);

/* Make a new edge joining nodes "from" and "to", which must belong
    to the same graph */
void G_addEdge(G_node from, G_node to);

/* Delete the edge joining "from" and "to" */
void G_rmEdge(G_node from, G_node to);

/* Show all the nodes and edges in the graph, using the function "showInfo"
    to print the name of each node */
void G_show(FILE *out, G_nodeList p, void showInfo(void *));

/* Get all the successors of node "n" */
G_nodeList G_succ(G_node n);

/* Get all the predecessors of node "n" */
G_nodeList G_pred(G_node n);

/* Tell if there is an edge from "from" to "to" */
bool G_goesTo(G_node from, G_node n);

/* Tell how many edges lead to or from "n" */
int G_degree(G_node n);

/* Get all the successors and predecessors of "n" */
G_nodeList G_adj(G_node n);

/* Get the "info" associated with node "n" */
void *G_nodeInfo(G_node n);

/* The type of "tables" mapping graph-nodes to information */
typedef struct TAB_table_  *G_table;

/* Make a new table */
G_table G_empty(void);

/* Enter the mapping "node"->"value" to the table "t" */
void G_enter(G_table t, G_node node, void *value);

/* Tell what "node" maps to in table "t" */
void *G_look(G_table t, G_node node);


// TODO based on int
/* bit matrix  */
typedef struct My_G_bitMatrix_ *My_G_bitMatrix;
struct My_G_bitMatrix_ {
    int num;
    bool *matrix;
};
/* create a new bitMatrix, the element number is same to list*/
My_G_bitMatrix My_G_BitMatrix(int n);
/* add edge in matrix, add t1->t2*/
void My_G_bitMatrixAdd(My_G_bitMatrix bitMatrix, int t1, int t2);
/* remove edge in matrix, remove t1->t2*/
void My_G_bitMatrixRemove(My_G_bitMatrix bitMatrix, int t1, int t2);
/* check whether t1 is connect with t2*/
bool My_G_bitMatrixIsConnect(My_G_bitMatrix bitMatrix, int t1, int t2);


#endif
