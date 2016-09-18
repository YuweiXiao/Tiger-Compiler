/* This file is not complete.  You should fill it in with your
   solution to the programming exercise. */
#include <stdio.h>
#include "prog1.h"
#include "slp.h"
int maxargs(A_stm stm);
void interp(A_stm stm);


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

void interp(A_stm stm) {

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
