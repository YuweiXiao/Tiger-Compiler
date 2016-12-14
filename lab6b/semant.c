#include <stdio.h>
#include "util.h"
#include "errormsg.h"
#include "symbol.h"
#include "absyn.h"
#include "types.h"
#include "temp.h"
#include "tree.h"
#include "frame.h"
#include "semant.h"

/*Lab4: Your implementation of lab4*/
F_fragList SEM_transProg(A_exp exp){
    // printf("start transProg\n");
    S_table venv = E_base_venv();
    S_table tenv = E_base_tenv();
    struct expty e = transExp(venv, tenv, exp, Tr_outermost());
    Tr_procFrag(e.exp, Tr_outermost()); 
    return Tr_getResult();
}



/*Lab4: Your implementation of lab4*/

struct expty expTy(Tr_exp exp, Ty_ty ty) {
    struct expty e;
    e.exp = exp;
    e.ty = ty;
    return e;
}


static Ty_ty actual_ty(S_table tenv, Ty_ty ty) {
    switch(ty->kind) {
    case Ty_loopVar:
        // printf("loop var\n");
        return actual_ty(tenv, ty->u.loopTy);
    case Ty_array:
        return actual_ty(tenv, ty->u.array);
    case Ty_name: {
        // printf("in actual type name\n");
        if(ty->u.name.ty != NULL) {
            return actual_ty(tenv, ty->u.name.ty);
        } else {
            E_enventry e_enventry = S_look(tenv, ty->u.name.sym);    
            if(e_enventry != NULL && e_enventry->u.var.ty->kind == Ty_name) {
                if(strcmp(S_name(e_enventry->u.var.ty->u.name.sym), S_name(ty->u.name.sym)) != 0) {
                    return actual_ty(tenv, e_enventry->u.var.ty);    
                } else {
                    return ty;
                }
            } else if(e_enventry != NULL) {
                return actual_ty(tenv, e_enventry->u.var.ty);    
            } else {
                return ty;
            }
        }
    }
    default:
        return ty;
  }
}

static Ty_tyList reverseFieldlist(Ty_tyList tyList) {
    Ty_tyList reversed = NULL;
    for(;tyList != NULL; tyList = tyList->tail) {
        reversed = Ty_TyList(tyList->head, reversed);
    }
    return reversed;
}

static struct expty transCallExp(S_table venv, S_table tenv, A_exp a, Tr_level level) {
    // printf("in call exp %s\n", S_name(a->u.call.func));
    E_enventry e_enventry = S_look(venv, a->u.call.func);
    if(e_enventry == NULL || e_enventry->kind != E_funEntry) {
        EM_error(a->pos, "undefined function %s", S_name(a->u.call.func));
        return expTy(Tr_no_opExp(), Ty_Void());
    } else {
        Ty_tyList tParam = e_enventry->u.func.formals;
        int flag = 0;
        A_expList p = a->u.call.args;
        T_expList expList = NULL, end;
        for(; p != NULL; p = p->tail) {
            struct expty e = transExp(venv, tenv, p->head, level);
            // printf("%s -- %d -- %d\n", S_name(a->u.call.func), tParam->head->kind, e.ty->kind);
            if(tParam == NULL) {
                EM_error(a->pos, "too many params in function %s", S_name(a->u.call.func));
                flag = 1;
                break;
            } else if(e.ty == NULL || 
                    actual_ty(tenv, tParam->head)->kind != actual_ty(tenv, e.ty)->kind) {
                // printf("-%d  -%d \n", tParam->head->kind, e_enventry->u.var.ty->kind);
                EM_error(a->pos, "para type mismatch");
                flag = 1;
                break;
            }
            if(expList == NULL) {
                expList = Tr_addExpIntoList(expList, e.exp);
                end = expList;
            } else {
                end = Tr_addExpIntoList(end, e.exp);
            }

            tParam = tParam->tail;
        }
        // still more params
        if(!flag && tParam != NULL) {
            EM_error(a->pos, "para type mismatch");
        } else {
            return expTy(Tr_callExp(e_enventry->u.func.label, level, e_enventry->u.func.level, expList), 
                                        e_enventry->u.func.result);
        }
    }
    return expTy(Tr_no_opExp(), e_enventry->u.func.result);
}

static struct expty transOpExp(S_table venv, S_table tenv, A_exp a, Tr_level level) {
    struct expty left = transExp(venv, tenv, a->u.op.left, level);
    struct expty right = transExp(venv, tenv, a->u.op.right, level);
    switch(a->u.op.oper) {
        case A_plusOp: case A_minusOp: case A_timesOp: case A_divideOp: {
            // printf("%d -- %d\n", left.ty->kind, right.ty->kind);
            if(actual_ty(tenv, left.ty)->kind != Ty_int) {
                EM_error(a->u.op.left->pos, "integer required");
            }
            if(actual_ty(tenv, right.ty)->kind != Ty_int) {
                EM_error(a->u.op.right->pos, "integer required");
            }
            return expTy(Tr_opExp(a->u.op.oper, left.exp, right.exp), Ty_Int());
        }
        default: {
            if( actual_ty(tenv, right.ty)->kind != Ty_nil && 
                    actual_ty(tenv, right.ty)->kind != actual_ty(tenv, left.ty)->kind) {
                EM_error(a->u.op.right->pos, "same type required"); 
            } 
            return expTy(Tr_conditionOpExp(a->u.op.oper, left.exp, right.exp), Ty_Int());
        }
    }
    printf("error - 20\n");
    return expTy(Tr_no_opExp(), Ty_Int());
}

struct expty transIfExp(S_table venv, S_table tenv, A_exp a, Tr_level level) {
    // printf("here %d\n", a->u.iff.test->kind);
    struct expty tExpty = transExp(venv, tenv, a->u.iff.test, level);
    if(tExpty.ty->kind != Ty_int) {
        EM_error(a->u.iff.test->pos, "if expression test should return int type");
    } else {
        struct expty left = transExp(venv, tenv, a->u.iff.then, level);
        struct expty right;
        if(a->u.iff.elsee != NULL) {
            right = transExp(venv, tenv, a->u.iff.elsee, level);
            if(left.ty->kind != right.ty->kind && right.ty->kind != Ty_nil && left.ty->kind != Ty_nil){
                // printf("if exp:%d--%d\n", left.ty->kind, right.ty->kind);
                EM_error(a->pos, "then exp and else exp type mismatch");
            }
            return expTy(Tr_ifExp(tExpty.exp, left.exp, right.exp), left.ty);
        } else {
            if(left.ty != Ty_Void() && left.ty != Ty_Nil()) {
                EM_error(a->u.iff.then->pos, "if-then exp's body must produce no value");
            } 
            
            return expTy(Tr_ifExp(tExpty.exp, left.exp, Tr_no_opExp()), Ty_Void());
        }
    }
    printf("error - 12\n");
    return expTy(Tr_no_opExp(), Ty_Void());
}

struct expty transRecordExp(S_table venv, S_table tenv, A_exp a, Tr_level level) {
    Ty_fieldList ty_fieldList = NULL;
    E_enventry e_enventry = S_look(tenv, a->u.record.typ);
    // the type not exist 
    // or the type is not a variable type
    // or the type is not a record variable type, report error 
    if(e_enventry == NULL || e_enventry->kind != E_varEntry || e_enventry->u.var.ty->kind != Ty_record) {
        EM_error(a->pos, "undefined type %s", S_name(a->u.record.typ));
    } else {
        // loop over the record expression field
        A_efieldList efieldList = a->u.record.fields;
        Tr_exp r = Tr_newTemp();
        Tr_exp init = NULL;
        int size = 0;
        for(; efieldList != NULL; efieldList = efieldList->tail) {
            A_efield field = efieldList->head;                          // get current field
            struct expty tExpty = transExp(venv, tenv, field->exp, level);     // the field value type
            Ty_fieldList list = e_enventry->u.var.ty->u.record;         // get the record type field

            int flag = 0;       // find whether the field exits and check whether the field type conicide.
            int index = 0;
            while(list != NULL) {
                Ty_field current = list->head;
                if( strcmp( S_name(current->name), S_name(field->name)) == 0) {
                    //  printf("%s -- %s\n", S_name(current->name), S_name(field->name));
                    // printf("%d -- %d\n", (actual_ty(tenv, current->ty))->kind, tExpty.ty->kind);
                    flag = 1;
                    // if( tExpty.ty != Ty_Nil() && (actual_ty(tenv, current->ty))->kind != tExpty.ty->kind) {
                    //     EM_error(a->pos, "record field type is not same.-2");
                    // }
                    init = Tr_initHeapVariable(r, index, tExpty.exp, init);
                    break;
                }
                list = list->tail;
                ++index;
            }
            if(!flag) {
                EM_error(a->pos, "record field is not exists");
                break;
            }
            ++size;
            ty_fieldList = Ty_FieldList(Ty_Field(field->name, tExpty.ty), ty_fieldList);
        }
        Tr_exp alloc = Tr_allocMem(r, Tr_intExp(size));
        return expTy(Tr_commbineAllocInitReturn(alloc, init, r), Ty_Record(ty_fieldList));
    }
    
    return expTy(Tr_no_opExp(), Ty_Record(ty_fieldList));
}

struct expty transExp(S_table venv, S_table tenv, A_exp a, Tr_level level) {
    E_enventry e_enventry;
    struct expty tExpty, left, right;

    switch(a->kind) {
        case A_varExp:
            // printf("var exp\n");
            return transVar(venv, tenv, a->u.var, level);
        case A_nilExp:
            return expTy(Tr_no_opExp(), Ty_Nil());
        case A_intExp: 
            // printf("int exp \n");
            return expTy(Tr_intExp(a->u.intt), Ty_Int());
        case A_stringExp:
            return expTy(Tr_stringExp(a->u.stringg), Ty_String());
        case A_callExp:
            // printf("call\n");
            return transCallExp(venv, tenv, a, level);
        case A_opExp:
            // printf("op exp\n");
            return transOpExp(venv, tenv, a, level);
        case A_recordExp:
            return transRecordExp(venv, tenv, a, level);
        case A_seqExp: {
            A_expList p = a->u.seq;
            Tr_exp exp = NULL;
            for(; p != NULL; p = p->tail) {
                tExpty = transExp(venv, tenv, p->head, level);
                if(exp == NULL) {
                    exp = tExpty.exp;
                } else {
                    exp = Tr_seqExp(exp, tExpty.exp);
                }
            }
            if(exp == NULL)
                exp = Tr_no_opExp();
            return expTy(exp, tExpty.ty);
        }
        case A_assignExp:
            // printf("in assign\n");
            left = transVar(venv, tenv, a->u.assign.var, level);
            right = transExp(venv, tenv, a->u.assign.exp, level);
            if(left.ty && left.ty->kind == Ty_loopVar) {
                EM_error(a->pos, "loop variable can't be assigned");
            }
            if(left.ty && right.ty && actual_ty(tenv, left.ty)->kind != actual_ty(tenv, right.ty)->kind) {
                EM_error(a->pos, "unmatched assign exp");
            }
            return expTy(Tr_assignExp(left.exp, right.exp), Ty_Void());
        case A_ifExp:
            // printf("if start\n");
            return transIfExp(venv, tenv, a, level);
        case A_whileExp: {
            // printf("in while %d\n", a->u.whilee.test->kind);
            tExpty = transExp(venv, tenv, a->u.whilee.test, level);
          // if(tExpty.ty->kind != Ty_int) {
          //   EM_error(a->pos, "while loop test should be integer expression.");
          // }
            right = transExp(venv, tenv, a->u.whilee.body, level);
            // printf("here\n");
            if(right.ty != Ty_Void()) {
                EM_error(a->pos, "while body must produce no value");
            }
            return expTy(Tr_LoopExp(tExpty.exp, right.exp), Ty_Void());
        }
        case A_forExp:
            printf("should not in for. for is translate to while\n");
            left = transExp(venv, tenv, a->u.forr.lo, level);
            right = transExp(venv, tenv, a->u.forr.hi, level);
            if(left.ty->kind != Ty_int || right.ty->kind != Ty_int) 
                EM_error(a->pos, "for exp's range type is not integer");
            
            S_beginScope(venv);
            Tr_access access = Tr_allocLocal(level, TRUE);
            S_enter(venv, a->u.forr.var, E_VarEntry(access, Ty_LoopVar(Ty_Int())));
            tExpty = transExp(venv, tenv, a->u.forr.body, level);
            S_endScope(venv);
            return tExpty;
        case A_breakExp:
            return expTy(NULL, Ty_Void());
        case A_letExp: {
            //printf("in let exp\n");
            S_beginScope(venv);
            S_beginScope(tenv);
            A_decList d = a->u.let.decs;
            Tr_exp exp = NULL, tmp;
            for(; d!= NULL; d = d->tail) {
                tmp = transDec(venv, tenv, d->head, level);
                if(exp == NULL)
                    exp = tmp;
                else {
                    exp = Tr_seqStm(exp, tmp);
                }
            }
            tExpty = transExp(venv, tenv, a->u.let.body, level);
            exp = Tr_seqExp(exp, tExpty.exp);
            S_endScope(venv);
            S_endScope(tenv);
            return expTy(exp, tExpty.ty);
        }
        case A_arrayExp:{
            e_enventry = S_look(tenv, a->u.array.typ);  // get array element type
            // array element type not found or it is a not a variable entry.
            // if(e_enventry == NULL || actual_ty(tenv, e_enventry->u.var.ty)->kind != Ty_array) {
                // EM_error(a->pos, "array type not exits or it is not a variable type");
            // } 
            tExpty = transExp(venv, tenv, a->u.array.size, level);
            // the size type is not int
            if(tExpty.ty->kind != Ty_int) {
                EM_error(a->u.array.size->pos, "array size should be integer");
            } else {
                right = transExp(venv, tenv, a->u.array.init, level);
                if(actual_ty(tenv, e_enventry->u.var.ty)->kind != actual_ty(tenv, right.ty)->kind) {
                    EM_error(a->u.array.init->pos, "type mismatch");
                }
                return expTy(Tr_arrayExp(tExpty.exp, right.exp), Ty_Array(e_enventry->u.var.ty));    
            }
            return expTy(Tr_no_opExp(), Ty_Array(Ty_Int()));
        }
    } 
    printf("error - 15\n");
}

struct expty transVar(S_table venv, S_table tenv, A_var v, Tr_level level) {
    E_enventry e_enventry;
    struct expty tExpty, left, right;
    switch(v->kind) {
        case A_simpleVar: {
            // printf("simple var\n");
            e_enventry = S_look(venv, v->u.simple);
            if(e_enventry == NULL) {
                EM_error(v->pos, "undefined variable %s", S_name(v->u.simple));
                return expTy(Tr_no_opExp(), Ty_Int());
            }
            //TODO useless???
            if(e_enventry->kind == E_funEntry) {
                EM_error(v->pos, "var should not be func");
            }
            return expTy(Tr_simpleVar(e_enventry->u.var.access, level), e_enventry->u.var.ty);
        }
        case A_fieldVar:
            // printf("trans field var\n");
            tExpty = transVar(venv, tenv, v->u.field.var, level);
            tExpty.ty = actual_ty(tenv, tExpty.ty);
            if(tExpty.ty == NULL || tExpty.ty->kind != Ty_record) {
                EM_error(v->pos, "not a record type");
                return expTy(Tr_no_opExp(), Ty_Int());
            }
            Ty_fieldList fieldlist = tExpty.ty->u.record;
            int index = 0;
            for(;fieldlist != NULL; fieldlist = fieldlist->tail) {
                Ty_field field = fieldlist->head;
                if( strcmp( S_name(field->name), S_name(v->u.field.sym)) == 0) {
                    return expTy(Tr_fieldVar(tExpty.exp, index), field->ty);
                }
                ++index;
            }
            EM_error(v->pos, "field %s doesn't exist", S_name(v->u.field.sym));
            return expTy(Tr_no_opExp(), Ty_Int());
        case A_subscriptVar: {
            left = transVar(venv, tenv, v->u.subscript.var, level);
            if(left.ty == NULL || left.ty->kind != Ty_array) {
                EM_error(v->pos, "array type required");
            }
            right = transExp(venv, tenv, v->u.subscript.exp, level);
            if(right.ty == NULL || actual_ty(tenv,right.ty)->kind != Ty_int) {
                EM_error(v->pos, "error -5");
            }
            return expTy(Tr_arrayVar(left.exp, right.exp), Ty_Array(actual_ty(tenv, left.ty)));       
        }
    }
    assert(0);
    return expTy(Tr_no_opExp(), Ty_Nil());
}


static void transFuncDec(S_table venv, S_table tenv, A_dec d, Tr_level level) {
    E_enventry e_enventry;
    S_table tmpTable = S_empty();
    A_fundec funcdec;
    A_fundecList funclist = d->u.function;

    // first round check, 
    // function param & return value type, 
    // enable recursive call
    for(;funclist != NULL; funclist = funclist->tail) {
        funcdec = funclist->head;
        // check same function name decleared.
        e_enventry = S_look(tmpTable, funcdec->name);
        if(e_enventry != NULL) {
            EM_error(d->pos, "two functions have the same name");
        }
        S_enter(tmpTable, funcdec->name, E_FunEntry(NULL, NULL, NULL, NULL));
    
        Ty_tyList formals = NULL;                   // function parameter type list
        A_fieldList fieldlist = funcdec->params;    // function parameter field list
        U_boolList escapeBoolList = NULL;
        // loop over parameter field list
        for(;fieldlist != NULL; fieldlist = fieldlist->tail) {
            A_field field = fieldlist->head;            // get current field
            e_enventry = S_look(tenv, field->typ);      // find field type in tenv
            // field not find or it is a function type
            if(e_enventry == NULL) {
                EM_error(d->pos, "function params type not decleared");
            }
            formals = Ty_TyList(e_enventry->u.var.ty, formals); // add to parameter type list
            escapeBoolList = U_BoolList(TRUE, escapeBoolList);
        }
        // TODO reverse escape BoolList
        formals = reverseFieldlist(formals);
        Temp_label funcLabel = Temp_namedlabel(S_name(funcdec->name));
        Tr_level newLevel = Tr_newLevel(level, funcLabel, escapeBoolList);
        
        // find return value type
        if(funcdec->result == NULL) {
            S_enter(venv, funcdec->name, E_FunEntry(newLevel, funcLabel, formals, Ty_Void()));
        } else {
            e_enventry = S_look(tenv, funcdec->result);
            // return value type not find or it is a function type
            if(e_enventry == NULL) {
                EM_error(d->pos, "function result type not decleared");
            }
            // add function decleartion in tenv
            S_enter(venv, funcdec->name, E_FunEntry(newLevel, funcLabel, formals, e_enventry->u.var.ty));
        }
    }

    // loop over each function declearion.
    for(funclist = d->u.function;funclist != NULL; funclist = funclist->tail) {
        S_beginScope(tenv);
        S_beginScope(venv);

        // add param into venv
        funcdec = funclist->head;
        A_fieldList fieldlist = funcdec->params;
        E_enventry func = S_look(venv, funcdec->name);
        // printf("function name:%s \n", S_name(funcdec->name));
        
        Tr_accessList accessList = func->u.func.level->accessList->tail;
        for(;fieldlist != NULL; fieldlist = fieldlist->tail) {
            e_enventry = S_look(tenv, fieldlist->head->typ);    // find field type in tenv  
            assert(accessList);
            Tr_access access = accessList->head;
            accessList = accessList->tail;
            // Tr_access access = Tr_allocLocal(func->u.func.level, TRUE);
            S_enter(venv, fieldlist->head->name, E_VarEntry(access, e_enventry->u.var.ty)); // add to parameter type list
        }
        
        
        struct expty tExpty = transExp(venv, tenv, funcdec->body, func->u.func.level);
        Tr_procFrag(tExpty.exp, func->u.func.level);
        
        if(funcdec->result == NULL) {
            if(tExpty.ty != NULL && tExpty.ty != Ty_Void() && tExpty.ty != Ty_Nil()) {
                EM_error(d->pos, "procedure returns value");
            }
        }
        S_endScope(venv);
        S_endScope(tenv);
    }
}

Tr_exp transDec(S_table venv, S_table tenv, A_dec d, Tr_level level) {
    E_enventry e_enventry;
    struct expty tExpty;
    switch(d->kind) {
        case A_functionDec:
            // printf("in funcdec\n");
            transFuncDec(venv, tenv, d, level);
            return Tr_no_opExp();
        case A_varDec:
            // printf("var dec\n");
            tExpty = transExp(venv, tenv, d->u.var.init, level);

            if(d->u.var.typ != NULL) {
                e_enventry = S_look(tenv, d->u.var.typ);
                if(e_enventry == NULL || e_enventry->kind == E_funEntry) {
                    EM_error(d->pos, "variable type not exist");
                }
                // printf("%d  %d \n", tExpty.ty->kind, e_enventry->u.var.ty->kind);
                if(actual_ty(tenv, tExpty.ty) != Ty_Nil() && tExpty.ty->kind != e_enventry->u.var.ty->kind) {
                    EM_error(d->pos, "type mismatch");  
                }   
            } else {
                if(tExpty.ty == Ty_Nil()) {
                    EM_error(d->pos, "init should not be nil without type specified");
                }
            }
            Tr_access access = Tr_allocLocal(level, TRUE);
            S_enter(venv, d->u.var.var, E_VarEntry(access, tExpty.ty));
            return Tr_initVariable(access, tExpty.exp);
        case A_typeDec: {
            // printf("in type dec\n");
            S_table tmpTable = S_empty();
            A_nametyList nametylist = d->u.type;
            for(; nametylist != NULL; nametylist = nametylist->tail) {
                e_enventry = S_look(tmpTable, nametylist->head->name);
                if(e_enventry != NULL) {
                    EM_error(d->pos, "two types have the same name");
                }
                S_enter(tmpTable, nametylist->head->name, E_VarEntry(NULL, Ty_Name(nametylist->head->name, NULL)));
                S_enter(tenv, nametylist->head->name, E_VarEntry(NULL, Ty_Name(nametylist->head->name, NULL)));
            }
            // printf("here\n");
            for(nametylist = d->u.type; nametylist != NULL; nametylist = nametylist->tail) {
                A_namety namety = nametylist->head;
                Ty_ty ty = transTy(tenv, namety->ty);
                Ty_ty actualTy = actual_ty(tenv, ty);
                
                // printf("right := left , %s := %s\n", S_name(namety->name), S_name(actualTy->u.name.sym));
                if(actualTy->kind == Ty_name && strcmp(S_name(namety->name), S_name(actualTy->u.name.sym)) == 0){
                    EM_error(d->pos, "illegal type cycle");
                }
                S_enter(tenv, namety->name, E_VarEntry(NULL, ty));
            }
            return Tr_no_opExp();
        }
    }
    printf("error-10\n");
}

Ty_ty transTy (S_table tenv, A_ty a) {
    E_enventry e_enventry;
    switch(a->kind) {
        case A_nameTy:
            e_enventry = S_look(tenv, a->u.name);
            if(e_enventry == NULL) {
                EM_error(a->pos, "type namety declearation error");
            }
            Ty_ty preTy = e_enventry->u.var.ty;
            Ty_ty ty = e_enventry->u.var.ty;
            while(ty->kind == Ty_name && ty->u.name.ty != NULL) {
                // printf("in while loop: %s\n", S_name(ty->u.name.sym));
                e_enventry = S_look(tenv, ty->u.name.sym);
                if(e_enventry == NULL) {
                  EM_error(a->pos, "type namety declearation error");
                } 
                preTy = ty;
                ty = e_enventry->u.var.ty;
            }
            return Ty_Name(a->u.name, preTy);
        case A_recordTy: {
            // printf("record type\n");
            A_fieldList fieldlist = a->u.record;
            Ty_fieldList ty_fieldList = NULL;
            for(; fieldlist != NULL; fieldlist = fieldlist->tail) {
                A_field field = fieldlist->head;
                e_enventry = S_look(tenv, field->typ);
                if(e_enventry == NULL) {
                    EM_error(a->pos, " undefined type %s", S_name(field->typ));
                    break;
                }
                // printf("name:%s  type:%d\n", S_name(field->name), e_enventry->u.var.ty->kind);
                ty_fieldList = Ty_FieldList(Ty_Field(field->name, e_enventry->u.var.ty), ty_fieldList);
            }
            return Ty_Record(ty_fieldList);
        }
        case A_arrayTy: {
            e_enventry = S_look(tenv, a->u.array);
            if(e_enventry == NULL || e_enventry->kind == E_funEntry) {
                EM_error(a->pos, "type array declearation error");
            }
            return Ty_Array(e_enventry->u.var.ty);
        }
    }
    assert(0);
} 
