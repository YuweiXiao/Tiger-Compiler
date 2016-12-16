#include "util.h"
#include "symbol.h" 
#include "absyn.h"  
#include <stdlib.h>
#include <stdio.h>
#include "table.h"

static void traverseExp(S_table env, int depth, A_exp e);
static void traverseDec(S_table env, int depth, A_dec e);
static void traverseVar(S_table env, int depth, A_var e);

typedef struct ES_escapeEntry_ *ES_escapeEntry;
struct ES_escapeEntry_ {
    int depth;
    bool *escape;
};

ES_escapeEntry ES_EscapeEntry(int depth, bool *p) {
    ES_escapeEntry s=checked_malloc(sizeof(*s));
    s->depth = depth;
    s->escape = p;
    return s;
}



void Esc_findEscape(A_exp exp) {
    S_table env = S_empty();
    traverseExp(env, 0, exp);
}

static void traverseVar(S_table env, int depth, A_var e) {
    // printf("in var\n");
    switch(e->kind) {
        case A_simpleVar: {
            ES_escapeEntry p = S_look(env, e->u.simple);
            printf("using : %s\n", S_name(e->u.simple));
            if(p != NULL) {
                // assert(p != NULL);
                if(p->depth < depth ) {
                    printf("mark as escape %s\n", S_name(e->u.simple));
                    *(p->escape) = TRUE;
                }
            }
            break;
        }
        case A_fieldVar: {
            traverseVar(env, depth, e->u.field.var);
            break;
        }
        case A_subscriptVar: {
            traverseExp(env, depth, e->u.subscript.exp);
            traverseVar(env, depth, e->u.subscript.var);
            break;
        }
    }
}

static void traverseDec(S_table env, int depth, A_dec e) {
    // printf("dec - %d\n", e->kind);
    switch(e->kind) {
        case A_functionDec: {
            A_fundecList fundecList = e->u.function;
            for(;fundecList; fundecList = fundecList->tail) {
                A_fieldList fieldList = fundecList->head->params;
                S_beginScope(env);
                for(; fieldList; fieldList = fieldList->tail) {
                    printf("put in %s\n", S_name(fieldList->head->name));
                    fieldList->head->escape = TRUE;
                    S_enter(env, fieldList->head->name, ES_EscapeEntry(depth + 1, &fieldList->head->escape));    
                }
                traverseExp(env, depth + 1, fundecList->head->body);
                S_endScope(env);
            }
            break;
        }
        case A_varDec: {
            traverseExp(env, depth, e->u.var.init);
            ES_escapeEntry p = S_look(env, e->u.var.var);
            if(p == NULL) {
                printf("put in %s\n", S_name(e->u.var.var));
                e->u.var.escape = FALSE;
                S_enter(env, e->u.var.var, ES_EscapeEntry(depth, &e->u.var.escape));
            }
            break;
        }
        case A_typeDec:
            break;  
    }
}

static void traverseExp(S_table env, int depth, A_exp e) {
    // printf("here - %d\n", e->kind);
    switch(e->kind) {
        case A_varExp: 
            traverseVar(env, depth, e->u.var);
            break;
        case A_callExp: {
            A_expList expList = e->u.call.args;
            for(;expList; expList = expList->tail) {
                traverseExp(env, depth, expList->head);
            }
            break;
        }
        case A_opExp: 
            traverseExp(env, depth, e->u.op.left);
            traverseExp(env, depth, e->u.op.right);
            break;
        case A_recordExp: {
            A_efieldList efieldList = e->u.record.fields;
            for(;efieldList; efieldList = efieldList->tail) {
                traverseExp(env, depth, efieldList->head->exp);
            }
            break;
        }
        case A_seqExp: {
            A_expList expList = e->u.seq;
            for(; expList; expList = expList->tail) {
                traverseExp(env, depth, expList->head);
            }
            break;
        }
        case A_assignExp: {
            traverseVar(env, depth, e->u.assign.var);
            traverseExp(env, depth, e->u.assign.exp);
            break;
        }
        case A_ifExp: {
            traverseExp(env, depth, e->u.iff.test);
            traverseExp(env, depth, e->u.iff.then);
            if(e->u.iff.elsee)
                traverseExp(env, depth, e->u.iff.elsee);
            break;
        }
        case A_whileExp: {
            traverseExp(env, depth + 1, e->u.whilee.test);
            traverseExp(env, depth + 1, e->u.whilee.body);
            break;
        }
        case A_forExp: 
            assert(0);
            break;
        case A_letExp: {
            A_decList decList = e->u.let.decs;
            S_beginScope(env);
            for(; decList; decList = decList->tail) {
                traverseDec(env, depth, decList->head);    
            }
            traverseExp(env, depth, e->u.let.body);
            S_endScope(env);
            break;            
        }
        case A_arrayExp:
            traverseExp(env, depth, e->u.array.size);
            traverseExp(env, depth, e->u.array.init);
            break;
        case A_nilExp: 
        case A_intExp: 
        case A_stringExp:
        case A_breakExp: 
            // printf("do nothing\n");
            // do nothing
            break;
    }
}


