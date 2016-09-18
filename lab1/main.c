/**
 * Author: Xiao Yuwei
 * Description: Lab1
 * Date: 2016.09.18
 * Score: pass
 */

/* This file is not complete.  You should fill it in with your
   solution to the programming exercise. */
#include <stdio.h>
#include <string.h>
#include "prog1.h"
#include "slp.h"
int maxargs(A_stm stm);
void interp(A_stm stm);

typedef struct table *Table_;
struct table
{
	string id;
	int value;
	Table_ tail;
};

struct IntAndTable {
	int i;
	Table_ t;
};

Table_ interpStm(A_stm stm, Table_ t);
struct IntAndTable interpExp(A_exp exp, Table_ t);


//TODO problem, print( (print(3),1), 2) counts 2 or 3 ?
int maxargs(A_stm stm) {
	int ans = 0;
	if(stm->kind == A_compoundStm) {
		int a = maxargs(stm->u.compound.stm1);
		int b = maxargs(stm->u.compound.stm2);
		ans = a > b? a:b;
	} else if(stm->kind == A_assignStm) {
		A_exp exp = stm->u.assign.exp;
		while(1) {
			if(exp->kind != A_eseqExp) {
				break;
			}
			int t = maxargs(exp->u.eseq.stm);
			ans = ans > t? ans:t;
			exp = exp->u.eseq.exp;
		}
	} else {
		A_expList list = stm->u.print.exps;
		while(1) {
			ans++;
			if(list->kind == A_lastExpList) {
				break;
			}
			list = list->u.pair.tail;
		}
	}
	return ans;
}


Table_ Table(string id, int value, struct table *tail) {
	Table_ t = checked_malloc(sizeof(*t));
	t->id = id;
	t->value = value;
	t->tail = tail;
	return t;
}

/**
 * loop up corresponding value of key in table
 * @return     value
 *             -1 if the key does not exist
 */
int lookup(Table_ t, string key) {
	if(t == NULL) {
		printf("\nundefine key : %s!\n", key);
    	return -1;
	}
	if(strcmp(t->id, key) == 0) {
		return t->value;
	}
	return lookup(t->tail, key);
}

struct IntAndTable interpExp(A_exp exp, Table_ t) {
	struct IntAndTable ans, tmp;
	if(exp->kind == A_idExp) {				// id expression
		ans.i = lookup(t, exp->u.id);
	} else if(exp->kind == A_numExp) {		// number expression
		ans.i = exp->u.num;
	} else if(exp->kind == A_opExp) {		// operation expression
		// get the left and right value of operation
		tmp = interpExp(exp->u.op.left ,t);
		int l = tmp.i;
		t = tmp.t;
		tmp = interpExp(exp->u.op.right, t);
		int r = tmp.i;
		t = tmp.t;
		// do operation
		if(exp->u.op.oper == A_plus) {
			ans.i = l + r;
		} else if(exp->u.op.oper == A_minus) {
			ans.i = l - r; 
		} else if(exp->u.op.oper == A_times) {
			ans.i = l * r;
		} else if(exp->u.op.oper == A_div) {
			ans.i = l / r;
		}
	} else {								// seq expression
		t = interpStm(exp->u.eseq.stm, t);
		tmp = interpExp(exp->u.eseq.exp, t);
		t = tmp.t;
		ans.i = tmp.i;
	}
	ans.t = t;								// set new table
	return ans;
}

Table_ interpStm(A_stm stm, Table_ t) {
	if(stm->kind == A_compoundStm) {		// compound statement
		t = interpStm(stm->u.compound.stm1, t);
		t = interpStm(stm->u.compound.stm2, t);
	} else if(stm->kind == A_assignStm) {	// assign statement
		struct IntAndTable tmp = interpExp(stm->u.assign.exp, t);
		t = Table(stm->u.assign.id, tmp.i, tmp.t);
	} else {								// print statement
		A_expList list = stm->u.print.exps;
		struct IntAndTable tmp;
		while(1) {
			if(list->kind == A_lastExpList) {
				tmp = interpExp(list->u.last, t);
				printf("%d\n", tmp.i);
				t = tmp.t;
				break;
			} else {
				tmp = interpExp(list->u.pair.head, t);
				printf("%d ", tmp.i);
				t = tmp.t;
				list = list->u.pair.tail;
			}
		}
	}
	return t;
}

void interp(A_stm stm) {
	interpStm(stm, NULL);
}
/*
 *Please don't modify the main() function
 */
int main()
{
	int args;

	printf("prog\n");
	args = maxargs(prog());
	printf("args: %d\n",args);
	interp(prog());

	printf("prog_prog\n");
	args = maxargs(prog_prog());
	printf("args: %d\n",args);
	interp(prog_prog());

	printf("right_prog\n");
	args = maxargs(right_prog());
	printf("args: %d\n",args);
	interp(right_prog());

	return 0;
}
