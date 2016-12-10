#include <stdio.h>
#include <stdlib.h>
#include "util.h"
#include "symbol.h"
#include "absyn.h"
#include "temp.h"
#include "errormsg.h"
#include "tree.h"
#include "assem.h"
#include "frame.h"
#include "codegen.h"
#include "table.h"

Temp_tempList L(Temp_temp h, Temp_tempList t) {
	return Temp_TempList(h, t);
}
static Temp_temp munchExp(T_exp e);
static void munchStm(T_stm s);
static Temp_tempList munchArgs(int n, T_expList expList);
static void munchMoveStm(T_stm s);
static Temp_temp munchMemExp(T_exp e);
static Temp_temp munchOpExp(T_exp e);


static AS_instrList iList = NULL, last = NULL;
static void emit(AS_instr inst) {
	if(last != NULL) {
		last = last->tail = AS_InstrList(inst, NULL);
	} else {
		last = iList = AS_InstrList(inst, NULL);
	}
}

//Lab 6: your code here
AS_instrList F_codegen(F_frame f, T_stmList stmList) {
	T_stmList now;
	for(now = stmList; now; now = now->tail)
		munchStm(now->head);
	return iList;
}


static Temp_temp munchExp(T_exp e) {
	Temp_temp r = Temp_newtemp();
	switch(e->kind) {
		case T_MEM:
			return munchMemExp(e);
		case T_BINOP:
			return munchOpExp(e);
		case T_CONST: {
			emit(AS_Oper(createString("movl $%d, `d0\n", e->u.CONST), L(r, NULL), NULL, NULL));
			// emit(AS_Oper(createString("ADDI `d0 <- r0+%d\n", e->u.CONST), L(r, NULL), NULL, NULL));
			return r;
		}
		case T_TEMP: {
			return e->u.TEMP;
		}
		case T_ESEQ:
			munchStm(e->u.ESEQ.stm);
			return munchExp(e->u.ESEQ.exp);
		case T_NAME: {
			Temp_enter(F_tempMap, r, Temp_labelstring(e->u.NAME));
			break;
		}
		case T_CALL: {
			emit(AS_Oper(String("call `s0\n"), 
				NULL, L(munchExp(e->u.CALL.fun), munchArgs(0, e->u.CALL.args)), NULL));
    		break;
		}
		default:
			printf("in munchExp: unmatch kind :%d\n", e->kind);
	}
	return r;
}


static void munchStm(T_stm s) {
	switch(s->kind) {
		case T_MOVE:
			munchMoveStm(s);
			break;
		case T_LABEL: {
			char buf[100];
			sprintf(buf, "%s:\n", Temp_labelstring(s->u.LABEL));
			emit(AS_Label(String(buf), s->u.LABEL));
			break;
		}
		case T_JUMP: {
			emit(AS_Oper(String("jmp `j0\n"), NULL, NULL, AS_Targets(s->u.JUMP.jumps)));
			// emit(AS_Oper(String("jmp `j0\n"), 
			// 	L(munchExp(s->u.JUMP.exp), NULL), NULL, AS_Targets(s->u.JUMP.jumps)));
			break;
		} 
		case T_CJUMP: {
			emit(AS_Oper(String("cmp `s0, `s1\n"), 
				NULL, L(munchExp(s->u.CJUMP.left), L(munchExp(s->u.CJUMP.right), NULL)), NULL));
			string jump;
			switch(s->u.CJUMP.op) {
				case T_eq:
					jump = "je `j0\n"; break;
				case T_ne:
					jump = "jne `j0\n"; break;
				case T_lt: 
					jump = "jl `j0\n"; break;
				case T_gt: 
					jump = "jg `j0\n"; break;
				case T_le: 
					jump = "jle `j0\n"; break;
				case T_ge:
					jump = "jge `j0\n"; break;
				default:
					printf("error in munchStm, T_CJUMP, not match op : op kind :%d\n", s->u.CJUMP.op);
					assert(0);
			}
			emit(AS_Oper(String(jump), NULL, NULL, AS_Targets(Temp_LabelList(s->u.CJUMP.true, NULL))));
			break;
		}
		case T_EXP: {
			munchExp(s->u.EXP);
			break;
		}
		default:
			printf("in munchStm : no kind match, kind:%d\n", s->kind);
	}
}

static Temp_tempList munchArgs(int n,T_expList expList) {
	if(expList == NULL)
		return NULL;
	Temp_temp r = munchExp(expList->head);
	Temp_tempList remain = munchArgs(n + 1, expList->tail);
	emit(AS_Oper(String("pushl `s0\n"),
		NULL, L(r, NULL), NULL));
	return L(r, remain);
}


/**
 * check memory tree type
 * @param  e memory exp
 * @return   
 */
static int getMemType(T_exp e) {
	T_exp mem = e->u.MEM;
	switch(mem->kind) {
		case T_CONST:
			return 3;
		case T_BINOP:{
			if(mem->u.BINOP.left->kind == T_CONST) 
				return 2;
			else {
				return 1;
			}
		}
		default:
			return 4;
	}
}

static void munchMoveStm(T_stm s) {
	T_exp src = s->u.MOVE.src;
	T_exp dst = s->u.MOVE.dst;
	if(dst->kind == T_MEM) {
		T_exp dstMem = dst->u.MEM;
		int memType = getMemType(dst);
		switch(memType) {
			case 1: 
				//MOVE(MEM(e1 + CONST), e2)
				emit(AS_Oper(createString("movl `s1, %d(`s0)\n", dstMem->u.BINOP.right->u.CONST),
					NULL, L(munchExp(dstMem->u.BINOP.left), L(munchExp(src), NULL)), NULL));
				return;
			case 2:
				//MOVE(MEM(CONST + e1), e2)
				emit(AS_Oper(createString("movl `s1 %d(`s0)\n", dstMem->u.BINOP.left->u.CONST),
					NULL, L(munchExp(dstMem->u.BINOP.right), L(munchExp(src), NULL)), NULL));
				return;
			case 3:
				//MOVE(MEM(CONST), e1)
				emit(AS_Oper(createString("`s0, (%d)\n", dstMem->u.CONST),
					NULL, L(munchExp(src), NULL), NULL));
				return;
		}
		if(src->kind == T_MEM) {
			assert(0);
			//is not support in at&t
			//MOVE(MEM(e1), MEM(e2))
			emit(AS_Oper(String("movl M[`s0] <- M[`s1]\n"),
				NULL, L(munchExp(dstMem), L(munchExp(dst->u.MEM), NULL)), NULL));
		} else {
			//MOVE(MEM(e1), e2)
			emit(AS_Oper(String("movl `s1, (`s0)\n"),
				NULL, L(munchExp(dstMem), L(munchExp(dst), NULL)), NULL));
		}
	} else {
		if(dst->kind == T_TEMP) {
			// MOVE(reg1, reg2)
			emit(AS_Move(String("movl `s0, `d0\n"),
				L(dst->u.TEMP, NULL), L(munchExp(src), NULL)));	
		} else {
			printf("error: in munch move: unknown type :src->kind:%d, dst->kind:%d\n", src->kind, dst->kind);
		}
	}
}


static Temp_temp munchMemExp(T_exp e) {
	int memType = getMemType(e);
	T_exp mem = e->u.MEM;
	switch(memType) {
		case 1: {
			// LOAD register <- M[`s0+ CONST]
			Temp_temp r = Temp_newtemp();
			emit(AS_Oper(createString("movl %d(`s0), `d0\n", mem->u.BINOP.right->u.CONST),
					L(r, NULL), L(munchExp(mem->u.BINOP.left), NULL), NULL));
			return r;
		}
		case 2: {
			//LOAD `d0 <- M[`s0+%d]
			Temp_temp r = Temp_newtemp();
			emit(AS_Oper(createString("movl %d(`s0), `d0\n", mem->u.BINOP.left->u.CONST),
					L(r, NULL), L(munchExp(mem->u.BINOP.right), NULL), NULL));
			return r;
		}
		case 3: {
			//LOAD `d0 <- M[r0+%d]
			Temp_temp r = Temp_newtemp();
			emit(AS_Oper(createString("movl (%d), `d0\n", mem->u.CONST),
				L(r, NULL), NULL, NULL));
			return r;
		}
		case 4: {
			//LOAD `d0 <- M[r0+%d]
			Temp_temp r = Temp_newtemp();
			emit(AS_Oper(String("movl (`s0), `d0\n"),
					L(r, NULL), L(munchExp(mem), NULL), NULL));
			return r;
		}
		default:
			printf("error in munchMemExp : should not be here\n");
	}
	return NULL;
}


static Temp_temp munchOpExp(T_exp e) {
	Temp_temp r = Temp_newtemp();
	string instr, op;
	switch(e->u.BINOP.op) {
		case T_plus: 
			instr = "addl"; op = "+"; break;
		case T_minus: 
			instr = "subl"; op = "-"; break;
		case T_mul: 
			instr = "imull"; op = "*"; break;
		case T_div:
			instr = "DIV"; op = "/"; break;
	}
	char buf[100];
	// Temp_temp r2 = Temp_newtemp();
	if(e->u.BINOP.left->kind == T_CONST) {
		emit(AS_Move(String("movl `s0, `d0\n"),
					L(r, NULL), L(munchExp(e->u.BINOP.right), NULL)));
		sprintf(buf, "%s $%d, `d0\n", instr, e->u.BINOP.left->u.CONST);
		emit(AS_Oper(String(buf), L(r, NULL), NULL, NULL));
	} else if(e->u.BINOP.right->kind == T_CONST) {
		emit(AS_Move(String("movl `s0, `d0\n"),
					L(r, NULL), L(munchExp(e->u.BINOP.left), NULL)));
		sprintf(buf, "%s $%d, `d0\n", instr, e->u.BINOP.right->u.CONST);
		emit(AS_Oper(String(buf), L(r, NULL), NULL, NULL));
	} else {
		r = munchExp(e->u.BINOP.left);
		// Temp_temp right = munchExp(e->u.BINOP.right);
		// emit(AS_Move(String("movl `s0, `d0\n"), L(r, NULL), L(left, NULL)));
		sprintf(buf, "%s `s0, `d0\n", instr);
		emit(AS_Oper(String(buf), L(r, NULL), L(munchExp(e->u.BINOP.right), L(r, NULL)), NULL));
	
		// sprintf(buf, "%s `d0 <- `s0%s`s1\n", instr, op);
		// emit(AS_Oper(String(buf),
			// L(r, NULL), L(left, L(munchExp(e->u.BINOP.right), NULL)), NULL));
	}
	return r;
}
