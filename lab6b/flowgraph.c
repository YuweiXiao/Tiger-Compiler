#include <stdio.h>
#include "util.h"
#include "symbol.h"
#include "tree.h"
#include "absyn.h"
#include "errormsg.h"
#include "table.h"
#include "flowgraph.h"

NodeInfo NewNodeInfo(AS_instr instr) {
	NodeInfo p = checked_malloc(sizeof(*p));
	switch(instr->kind) {
		case I_OPER:
			p->def = instr->u.OPER.dst;
			p->use = instr->u.OPER.src;
			break;
		case I_MOVE:
			p->def = instr->u.MOVE.dst;
			p->use = instr->u.MOVE.src;
			break;
		case I_LABEL:
			p->def = NULL;
			p->use = NULL;
			break;
	}
	p->instr = instr;
	return p;
}

Temp_tempList FG_def(G_node n) {
	NodeInfo info = G_nodeInfo(n);
	return info->def;
}

Temp_tempList FG_use(G_node n) {
	NodeInfo info = G_nodeInfo(n);
	return info->use;
}

bool FG_isMove(G_node n) {
	NodeInfo info = G_nodeInfo(n);
	return info->instr->kind == I_MOVE;
}

FILE *flowgraph_out = NULL;

G_graph FG_AssemFlowGraph(AS_instrList il, F_frame f) {
	if(flowgraph_out == NULL)
		flowgraph_out = fopen("my11.txt", "w");
	G_graph graph = G_Graph();
	fprintf(flowgraph_out, "------------start------------------------------------------------------------------------------%d\n", G_nodesNumber(graph));
	S_table labelTable = S_empty();
	G_node preNode = NULL;
	AS_instr preInstr = NULL;
	AS_instrList list = il;
	// first loop over all instr, create all node, and get label table
	for(; list != NULL; list = list->tail) {
		AS_instr instr = list->head;
		G_node node = G_Node(graph, NewNodeInfo(instr));
		// printf("%s\n", );
		if(instr->kind == I_LABEL) {
			// printf("---#######################%s$$$$$$$$$$$$$$\n", S_name(instr->u.LABEL.label));
			S_enter(labelTable, instr->u.LABEL.label, node);
		} 
		if(preNode != NULL) {
			if( !(preInstr->kind == I_OPER && strcmp(preInstr->u.OPER.assem, "jmp `j0\n") == 0))
				G_addEdge(preNode, node);
			else {
				fprintf(flowgraph_out, "---#######################$$$$$$$$$$$$$@@@@@@@@@@@@$\n");
			}
		}
		preNode = node;
		preInstr = instr;
	}

	G_nodeList nodeList = G_nodes(graph);
	for(; nodeList != NULL; nodeList = nodeList->tail) {
		G_node node = nodeList->head;
		NodeInfo info = G_nodeInfo(node);
		AS_instr instr = info->instr;
		if(instr->kind == I_OPER && instr->u.OPER.jumps != NULL) {
			Temp_label label = instr->u.OPER.jumps->labels->head;
			// printf("-------------%s---------------\n", S_name(label));
			
			// the jump target should be single.
			assert(instr->u.OPER.jumps->labels->tail == NULL);
	
			G_node next = S_look(labelTable, label);
			assert(next);
			G_addEdge(node, next);
		}
	}

	
	G_show(flowgraph_out, G_nodes(graph), NULL);
    fprintf(flowgraph_out, "-----finish----------------------------------------\n");

	return graph;
}
