#include <stdio.h>
#include "util.h"
#include "errormsg.h"
#include "symbol.h"
#include "absyn.h"
#include "types.h"
#include "env.h"
#include "semant.h"

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

void SEM_transProg(A_exp a){
  S_table venv = E_base_venv();
  S_table tenv = E_base_tenv();
  struct expty e = transExp(venv, tenv, a);
}

static Ty_tyList reverseFieldlist(Ty_tyList tyList) {
  Ty_tyList reversed = NULL;
  for(;tyList != NULL; tyList = tyList->tail) {
    reversed = Ty_TyList(tyList->head, reversed);
  }
  return reversed;
}

static struct expty transCallExp(S_table venv, S_table tenv, A_exp a) {
    E_enventry e_enventry = S_look(venv, a->u.call.func);

    // printf("in call exp %s\n", S_name(a->u.call.func));
    if(e_enventry == NULL || e_enventry->kind != E_funEntry) {
        EM_error(a->pos, "undefined function %s", S_name(a->u.call.func));
        return expTy(NULL, Ty_Void());
    } else {
        Ty_tyList tParam = e_enventry->u.func.formals;
        int flag = 0;
        A_expList p = a->u.call.args;
        for(; p != NULL; p = p->tail) {
            struct expty e = transExp(venv, tenv, p->head);
          
            // printf("%s -- %d -- %d\n", S_name(a->u.call.func), tParam->head->kind, e.ty->kind);
            if(tParam == NULL) {
                EM_error(a->pos, "too many params in function %s", S_name(a->u.call.func));
                flag = 1;
                break;
            } else if(e.ty == NULL || 
                    actual_ty(tenv, tParam->head)->kind != actual_ty(tenv, e.ty)->kind) {
                
                EM_error(a->pos, "para type mismatch");
                flag = 1;
                break;
            }

            // not decleared function or type matches, continue
            // else if( e.ty != NULL && (actual_ty(tParam->head))->kind == (actual_ty(e.ty))->kind) {
            tParam = tParam->tail;
        }
        // still more params
        if(!flag && tParam != NULL) {
          EM_error(a->pos, "para type mismatch");
        }
    }
    return expTy(NULL, e_enventry->u.func.result);
}

static struct expty transOpExp(S_table venv, S_table tenv, A_exp a) {
    struct expty left = transExp(venv, tenv, a->u.op.left);
    struct expty right = transExp(venv, tenv, a->u.op.right);
    switch(a->u.op.oper) {
        case A_plusOp: case A_minusOp:case A_timesOp:case A_divideOp: {
            // printf("%d -- %d\n", left.ty->kind, right.ty->kind);
            if(actual_ty(tenv, left.ty)->kind != Ty_int) {
                EM_error(a->u.op.left->pos, "integer required");
            }
            if(actual_ty(tenv, right.ty)->kind != Ty_int) {
                EM_error(a->u.op.right->pos, "integer required");
            }
            break;
        }
        default: {
            if( actual_ty(tenv, right.ty)->kind != Ty_nil && 
                    actual_ty(tenv, right.ty)->kind != actual_ty(tenv, left.ty)->kind) {
                EM_error(a->u.op.right->pos, "same type required"); 
            } 
            break;
        }
    }
    return expTy(NULL, Ty_Int());
}

struct expty transIfExp(S_table venv, S_table tenv, A_exp a) {
    struct expty tExpty = transExp(venv, tenv, a->u.iff.test);
    if(tExpty.ty->kind != Ty_int) {
        EM_error(a->u.iff.test->pos, "if expression test should return int type");
    } else {
        struct expty left = transExp(venv, tenv, a->u.iff.then);
        struct expty right;
        if(a->u.iff.elsee != NULL) {
            right = transExp(venv, tenv, a->u.iff.elsee);
            if(left.ty->kind != right.ty->kind && right.ty->kind != Ty_nil && left.ty->kind != Ty_nil){
                // printf("if exp:%d--%d\n", left.ty->kind, right.ty->kind);
                EM_error(a->pos, "then exp and else exp type mismatch");
            }
        } else {
            if(left.ty != Ty_Void() && left.ty != Ty_Nil()) {
                EM_error(a->u.iff.then->pos, "if-then exp's body must produce no value");
            } 
        }
        return expTy(NULL, left.ty);
    }
    return expTy(NULL, Ty_Void());
}

struct expty transRecordExp(S_table venv, S_table tenv, A_exp a) {
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
        for(; efieldList != NULL; efieldList = efieldList->tail) {
            
            A_efield field = efieldList->head;                          // get current field
            struct expty tExpty = transExp(venv, tenv, field->exp);     // the field value type
            Ty_fieldList list = e_enventry->u.var.ty->u.record;         // get the record type field

            int flag = 0;       // find whether the field exits and check whether the field type conicide.
            while(list != NULL) {
                Ty_field current = list->head;

                if( strcmp( S_name(current->name), S_name(field->name)) == 0) {
                    flag = 1;
                    //  printf("%s -- %s\n", S_name(current->name), S_name(field->name));
                    // printf("%d -- %d\n", (actual_ty(tenv, current->ty))->kind, tExpty.ty->kind);
                    if( (actual_ty(tenv, current->ty))->kind != tExpty.ty->kind) {
                        EM_error(a->pos, "record field type is not same.-2");
                    }
                    break;
                }
                list = list->tail;
            }
            if(!flag) {
                EM_error(a->pos, "record field is not exists");
                break;
            }
            ty_fieldList = Ty_FieldList(Ty_Field(field->name, tExpty.ty), ty_fieldList);
        }
    }
    return expTy(NULL, Ty_Record(ty_fieldList));
}

struct expty transExp(S_table venv, S_table tenv, A_exp a) {
  E_enventry e_enventry;
  struct expty tExpty, left, right;
  switch(a->kind) {
    case A_varExp:
        return transVar(venv, tenv, a->u.var);
    case A_nilExp:
        return expTy(NULL, Ty_Nil());
    case A_intExp: 
        return expTy(NULL, Ty_Int());
    case A_stringExp:
        return expTy(NULL, Ty_String());
    case A_callExp:
        return transCallExp(venv, tenv, a);
    case A_opExp:
        return transOpExp(venv, tenv, a);
    case A_recordExp:
        return transRecordExp(venv, tenv, a);
    case A_seqExp: {
        A_expList p = a->u.seq;
        for(; p != NULL; p = p->tail) {
            tExpty = transExp(venv, tenv, p->head);
        }
        return expTy(NULL, tExpty.ty);
    }
    case A_assignExp:
        // printf("in assign\n");
        left = transVar(venv, tenv, a->u.assign.var);
        right = transExp(venv, tenv, a->u.assign.exp);
        if(left.ty && left.ty->kind == Ty_loopVar) {
            EM_error(a->pos, "loop variable can't be assigned");
        }
        if(left.ty && right.ty && actual_ty(tenv, left.ty)->kind != actual_ty(tenv, right.ty)->kind) {
            EM_error(a->pos, "unmatched assign exp");
        }
        return expTy(NULL, Ty_Void());
    case A_ifExp:
        return transIfExp(venv, tenv, a);
    case A_whileExp: {
        tExpty = transExp(venv, tenv, a->u.whilee.test);
      // if(tExpty.ty->kind != Ty_int) {
      //   EM_error(a->pos, "while loop test should be integer expression.");
      // }
        tExpty = transExp(venv, tenv, a->u.whilee.body);
        if(tExpty.ty != Ty_Void()) {
            EM_error(a->pos, "while body must produce no value");
        }
        return expTy(NULL, Ty_Void());
    }
    case A_forExp:
        left = transExp(venv, tenv, a->u.forr.lo);
        right = transExp(venv, tenv, a->u.forr.hi);
        if(left.ty->kind != Ty_int || right.ty->kind != Ty_int) 
            EM_error(a->pos, "for exp's range type is not integer");
      
        S_beginScope(venv);
        S_enter(venv, a->u.forr.var, E_VarEntry(Ty_LoopVar(Ty_Int())));
        tExpty = transExp(venv, tenv, a->u.forr.body);
        S_endScope(venv);
        return tExpty;
    case A_breakExp:
        return expTy(NULL, Ty_Void());
    case A_letExp: {
        // printf("in let exp\n");
        S_beginScope(venv);
        S_beginScope(tenv);
        A_decList d = a->u.let.decs;
        for(; d!= NULL; d = d->tail) {
            transDec(venv, tenv, d->head);
        }
        tExpty = transExp(venv, tenv, a->u.let.body);
        S_endScope(venv);
        S_endScope(tenv);
        return tExpty;
    }
    case A_arrayExp:{
        e_enventry = S_look(tenv, a->u.array.typ);  // get array element type
        // array element type not found or it is a not a variable entry.
        if(e_enventry == NULL || e_enventry->u.var.ty->kind != Ty_array) {
            EM_error(a->pos, "array type not exits or it is not a variable type");
        } else {
            tExpty = transExp(venv, tenv, a->u.array.size);
            // the size type is not int
            if(tExpty.ty->kind != Ty_int) {
                EM_error(a->u.array.size->pos, "array size should be integer");
            } else {
                tExpty = transExp(venv, tenv, a->u.array.init);
                if(actual_ty(tenv, e_enventry->u.var.ty) != actual_ty(tenv, tExpty.ty)) {
                    EM_error(a->u.array.init->pos, "type mismatch");
                }
            }
            return expTy(NULL, Ty_Array(e_enventry->u.var.ty));
        }
        return expTy(NULL, Ty_Array(Ty_Int()));
        break;
    }
  } 
}

struct expty transVar(S_table venv, S_table tenv, A_var v) {
    E_enventry e_enventry;
    struct expty tExpty, left, right;
    switch(v->kind) {
        case A_simpleVar: {
            e_enventry = S_look(venv, v->u.simple);
            if(e_enventry == NULL) {
                EM_error(v->pos, "undefined variable %s", S_name(v->u.simple));
                return expTy(NULL, Ty_Int());
            }
            if(e_enventry->kind == E_funEntry) {
                EM_error(v->pos, "var should not be func");
            }
            return expTy(NULL, e_enventry->u.var.ty);
        }
        case A_fieldVar:
            tExpty = transVar(venv, tenv, v->u.field.var);
            if(tExpty.ty == NULL || tExpty.ty->kind != Ty_record) {
                EM_error(v->pos, "not a record type");
                return expTy(NULL, Ty_Int());
            }
            Ty_fieldList fieldlist = tExpty.ty->u.record;
            for(;fieldlist != NULL; fieldlist = fieldlist->tail) {
                Ty_field field = fieldlist->head;
                if( strcmp( S_name(field->name), S_name(v->u.field.sym)) == 0) {
                    return expTy(NULL, field->ty);
                }
            }
            EM_error(v->pos, "field %s doesn't exist", S_name(v->u.field.sym));
            return expTy(NULL, Ty_Int());
        case A_subscriptVar: {
            left = transVar(venv, tenv, v->u.subscript.var);
            if(left.ty == NULL || left.ty->kind != Ty_array) {
                EM_error(v->pos, "array type required");
            }
            right = transExp(venv, tenv, v->u.subscript.exp);
            if(right.ty == NULL || actual_ty(tenv,right.ty)->kind != Ty_int) {
                EM_error(v->pos, "error -5");
            }
            return expTy(NULL, Ty_Array(actual_ty(tenv, left.ty)));       
        }
    }
    return expTy(NULL, Ty_Nil());
}


void transDec(S_table venv, S_table tenv, A_dec d) {
    A_fundecList funclist;
    A_fundec funcdec;
    E_enventry e_enventry;
    A_nametyList nametylist;
    A_namety namety;
    struct expty tExpty;
    switch(d->kind) {
        case A_functionDec: {
            S_table tmpTable = S_empty();
            for(funclist = d->u.function;funclist != NULL; funclist = funclist->tail) {
                funcdec = funclist->head;
                // check same function name decleared.
                e_enventry = S_look(tmpTable, funcdec->name);
                if(e_enventry != NULL) {
                    EM_error(d->pos, "two functions have the same name");
                }
                S_enter(tmpTable, funcdec->name, E_FunEntry( NULL, NULL));

            
                Ty_tyList formals = NULL;                   // function parameter type list
                A_fieldList fieldlist = funcdec->params;    // function parameter field list
                // loop over parameter field list
                for(;fieldlist != NULL; fieldlist = fieldlist->tail) {
            
                    A_field field = fieldlist->head;            // get current field
                    e_enventry = S_look(tenv, field->typ);      // find field type in tenv
                    // field not find or it is a function type
                    if(e_enventry == NULL) {
                        EM_error(d->pos, "function params type not decleared");
                    }
                    formals = Ty_TyList(e_enventry->u.var.ty, formals); // add to parameter type list
                }
                formals = reverseFieldlist(formals);
        
                // find return value type
                if(funcdec->result == NULL) {
                    S_enter(venv, funcdec->name, E_FunEntry( formals, Ty_Void()));
                } else {
                    e_enventry = S_look(tenv, funcdec->result);
                    // return value type not find or it is a function type
                    if(e_enventry == NULL) {
                        EM_error(d->pos, "function result type not decleared");
                    }
                    // add function decleartion in tenv
                    S_enter(venv, funcdec->name, E_FunEntry( formals, e_enventry->u.var.ty));
                }
            }

            // loop over each function declearion.
            for(funclist = d->u.function;funclist != NULL; funclist = funclist->tail) {
                S_beginScope(tenv);
                S_beginScope(venv);

                funcdec = funclist->head;
                A_fieldList fieldlist = funcdec->params;
                for(;fieldlist != NULL; fieldlist = fieldlist->tail) {
                    e_enventry = S_look(tenv, fieldlist->head->typ);    // find field type in tenv
                    S_enter(venv, fieldlist->head->name, E_VarEntry(e_enventry->u.var.ty)); // add to parameter type list
                }
                tExpty = transExp(venv, tenv, funcdec->body);
                if(funcdec->result == NULL) {
                    if(tExpty.ty != NULL && tExpty.ty != Ty_Void() && tExpty.ty != Ty_Nil()) {
                        EM_error(d->pos, "procedure returns value");
                    }
                }
                S_endScope(venv);
                S_endScope(tenv);
            }
            break;
        }
        case A_varDec:
            // printf("var dec\n");
            tExpty = transExp(venv, tenv, d->u.var.init);

            if(d->u.var.typ != NULL) {
                e_enventry = S_look(tenv, d->u.var.typ);
                if(e_enventry == NULL || e_enventry->kind == E_funEntry) {
                    EM_error(d->pos, "variable type not exist");
                }
                if(actual_ty(tenv, tExpty.ty) != Ty_Nil() && tExpty.ty != e_enventry->u.var.ty) {
                    EM_error(d->pos, "type mismatch");  
                }   
            } else {
                if(tExpty.ty == Ty_Nil()) {
                    EM_error(d->pos, "init should not be nil without type specified");
                }
            }
            S_enter(venv, d->u.var.var, E_VarEntry(tExpty.ty));
            break;
        case A_typeDec: {
            // printf("in type dec\n");
            S_table tmpTable = S_empty();
            for(nametylist = d->u.type; nametylist != NULL; nametylist = nametylist->tail) {
                e_enventry = S_look(tmpTable, nametylist->head->name);
                if(e_enventry != NULL) {
                    EM_error(d->pos, "two types have the same name");
                }
                S_enter(tmpTable, nametylist->head->name, E_VarEntry(Ty_Name(nametylist->head->name, NULL)));
                S_enter(tenv, nametylist->head->name, E_VarEntry(Ty_Name(nametylist->head->name, NULL)));
            }
            for(nametylist = d->u.type; nametylist != NULL; nametylist = nametylist->tail) {
                namety = nametylist->head;
                Ty_ty ty = transTy(tenv, namety->ty);
                Ty_ty actualTy = actual_ty(tenv, ty);
                // printf("right := left , %s := %s\n", S_name(namety->name), S_name(actualTy->u.name.sym));
                if(actualTy->kind == Ty_name && strcmp(S_name(namety->name), S_name(actualTy->u.name.sym)) == 0){
                    EM_error(d->pos, "illegal type cycle");
                }
                S_enter(tenv, namety->name, E_VarEntry(ty));
            }
            break;
        }   
        default:
            printf("error-10\n");
    }
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
                if(e_enventry == NULL || e_enventry->kind == E_funEntry) {
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

} 
