%{
#include <stdio.h>
#include <stdlib.h>
#include "util.h"
#include "symbol.h" 
#include "errormsg.h"
#include "absyn.h"

int yylex(void); /* function prototype */

A_exp absyn_root;

void yyerror(char *s)
{
 EM_error(EM_tokPos, "%s", s);
 exit(1);
}
%}


%union {
  int pos;
  int ival;
  string sval;
  A_var var;
  A_exp exp;
  /* et cetera */
  A_expList explist;
  A_dec dec;
  A_decList declist;
  A_fundec fundec;
  A_fundecList fundeclist;
  A_var lvalue;
  A_field field;
  A_fieldList fieldlist;
  A_efield efield;
  A_efieldList efiledlist;
  A_ty ty;
  A_namety namety;
  A_nametyList nametylist;  
}

%token <sval> ID STRING
%token <ival> INT

%token 
  COMMA COLON SEMICOLON LPAREN RPAREN LBRACK RBRACK 
  LBRACE RBRACE DOT 
  PLUS MINUS TIMES DIVIDE EQ NEQ LT LE GT GE
  AND OR ASSIGN
  ARRAY IF THEN ELSE WHILE FOR TO DO LET IN END OF 
  BREAK NIL
  FUNCTION VAR TYPE 

%left LOWEST
%nonassoc OF IF THEN DO WHILE FOR TO ASSIGN
%left ELSE
%left OR
%left AND
%nonassoc EQ NEQ GT LT GE LE
%left PLUS MINUS
%left TIMES DIVIDE
%left UMINUS
%right LBRACK
%nonassoc TYPE FUNCTION VAR

%type <exp> exp program 
/* et cetera */
%type <exp> lvalueExp nilExp literalExp funcCallExp arithmeticExp comparisonExp booleanExp assignExp letExp loopExp ifExp seqExp arrayExp recordExp
%type <explist> argList seqExpList
%type <declist> declist 
%type <dec> dec vardec
%type <fundec> fundec
%type <fundeclist> fundeclist
%type <field> tyfield
%type <fieldlist> tyfieldlist
%type <efield> efield
%type <efiledlist> efiledlist
%type <lvalue> lvalue
%type <ty> ty
%type <namety> tydec
%type <nametylist> tydeclist


%start program

%%

program:   exp    {absyn_root=$1;}


/* expression */
/* {$$=A_VarExp(EM_tokPos,A_SimpleVar(EM_tokPos,S_Symbol($1)));}*/
exp : lvalueExp {$$ = $1;}                  /* var expression*/
    | nilExp     {$$ = $1;}                 /* NIL expression*/
    | seqExp          {$$ = $1;}            /* sequencing expression note: can be ()*/
    | literalExp               {$$ = $1;}   /* literal expression*/               
    | funcCallExp         {$$ = $1;}        /* function call */
    | arithmeticExp         {$$ = $1;}      /* arithmetic expression*/
    | comparisonExp {$$ = $1;}
    | booleanExp {$$ = $1;}
    | letExp           {$$ = $1;}            /* let expression*/
    | loopExp  {$$ = $1;}
    | ifExp {$$ = $1;}
    | arrayExp {$$ = $1;}
    | recordExp {$$ = $1;}
    | assignExp      {$$ = $1;}         /* assignment expression*/
   /* | BREAK    */                   /* break */

/* array creation expression*/
arrayExp : ID LBRACK exp RBRACK OF exp  {$$ = A_ArrayExp(EM_tokPos, S_Symbol($1),$3, $6);}

/* record creation expression */
recordExp : ID LBRACE efiledlist RBRACE {$$ = A_RecordExp(EM_tokPos, S_Symbol($1), $3);}

/* efield */
efield : ID EQ exp {$$ = A_Efield(S_Symbol($1), $3);}

efiledlist : {$$ = NULL;}
           | efield %prec LOWEST {$$ = A_EfieldList($1, NULL);}
           | efield COMMA efiledlist {$$ = A_EfieldList($1, $3);}


/* var expression*/
lvalueExp : lvalue {$$ = A_VarExp(EM_tokPos, $1);}

/* variable */
lvalue : ID  %prec LOWEST {$$ = A_SimpleVar(EM_tokPos, S_Symbol($1));}
       | lvalue DOT ID    {$$ = A_FieldVar(EM_tokPos, $1, S_Symbol($3));}
       | lvalue LBRACK exp RBRACK {$$ = A_SubscriptVar(EM_tokPos, $1, $3);}
       | ID LBRACK exp RBRACK {$$ = A_SubscriptVar(EM_tokPos, A_SimpleVar(EM_tokPos, S_Symbol($1)), $3);}



/* assignment expression*/
assignExp : lvalue ASSIGN exp {$$ = A_AssignExp(EM_tokPos, $1, $3);}

/* for loop expression*/
/* while loop expression*/
loopExp : FOR ID ASSIGN exp TO exp DO exp {$$ = A_ForExp(EM_tokPos, S_Symbol($2), $4, $6, $8);}
        | WHILE exp DO exp {$$ = A_WhileExp(EM_tokPos, $2, $4);}

/* let expression*/
letExp : LET declist IN seqExpList END  {$$ = A_LetExp(EM_tokPos, $2, A_SeqExp(EM_tokPos, $4));}

/* string literal*/
/* Integer literal*/
literalExp : INT {$$ = A_IntExp(EM_tokPos, $1);}
           | STRING {$$ = A_StringExp(EM_tokPos, $1);}

/* NIL expression*/
nilExp : NIL {$$ = A_NilExp(EM_tokPos);}


/* if expression*/
ifExp : IF exp THEN exp ELSE exp  {$$ = A_IfExp(EM_tokPos, $2, $4, $6);}
      | IF exp THEN exp           {$$ = A_IfExp(EM_tokPos, $2, $4, A_NilExp(EM_tokPos));}


/* function call expression*/
funcCallExp : ID LPAREN argList RPAREN {$$ = A_CallExp(EM_tokPos, S_Symbol($1), $3);}

argList : {$$ = NULL;}
        | exp {$$ = A_ExpList($1, NULL);}
        | exp COMMA argList {$$ = A_ExpList($1, $3);}

/* sequence exp */ 
seqExp : LPAREN seqExpList RPAREN {$$ = A_SeqExp(EM_tokPos, $2);}

seqExpList : {$$ = NULL;}
           | exp {$$ = A_ExpList($1, NULL);}
           | exp SEMICOLON seqExpList {$$ = A_ExpList($1, $3);}

/* boolean expression*/
booleanExp : exp AND exp {$$ = A_IfExp(EM_tokPos, $1, $3, A_IntExp(EM_tokPos,0));}
           | exp OR exp {$$ = A_IfExp(EM_tokPos, $1, A_IntExp(EM_tokPos,1), $3);}

/* comparasion */
comparisonExp : exp EQ exp {$$ = A_OpExp(EM_tokPos, A_eqOp, $1, $3);}
              | exp NEQ exp {$$ = A_OpExp(EM_tokPos, A_neqOp, $1, $3);}
              | exp GT exp {$$ = A_OpExp(EM_tokPos, A_gtOp, $1, $3);}
              | exp GE exp {$$ = A_OpExp(EM_tokPos, A_geOp, $1, $3);}
              | exp LE exp {$$ = A_OpExp(EM_tokPos, A_leOp, $1, $3);}
              | exp LT exp {$$ = A_OpExp(EM_tokPos, A_ltOp, $1, $3);}

/* arithmetic expression */
arithmeticExp : exp PLUS exp {$$ = A_OpExp(EM_tokPos, A_plusOp, $1, $3);}
              | exp MINUS exp {$$ = A_OpExp(EM_tokPos, A_minusOp, $1, $3);}
              | exp TIMES exp {$$ = A_OpExp(EM_tokPos, A_timesOp, $1, $3);}
              | exp DIVIDE exp {$$ = A_OpExp(EM_tokPos, A_divideOp, $1, $3);}
              | MINUS exp %prec UMINUS {$$ = A_OpExp(EM_tokPos, A_minusOp, A_IntExp(EM_tokPos, 0), $2);}


/* declarations */

declist : dec %prec LOWEST {$$ = A_DecList($1, NULL);}
        | dec declist {$$ = A_DecList($1, $2);}

dec : tydeclist {$$ = A_TypeDec(EM_tokPos, $1);}
    | vardec {$$ = $1;}
    | fundeclist {$$ = A_FunctionDec(EM_tokPos, $1);}

/* functions */

fundec : FUNCTION ID LPAREN tyfieldlist RPAREN EQ exp  {$$ = A_Fundec(EM_tokPos, S_Symbol($2), $4, NULL, $7);}
        | FUNCTION ID LPAREN tyfieldlist RPAREN COLON ID EQ exp {$$ = A_Fundec(EM_tokPos, S_Symbol($2), $4, S_Symbol($7), $9);}

fundeclist : fundec %prec LOWEST {$$ = A_FundecList($1, NULL);}
           | fundec fundeclist {$$ = A_FundecList($1, $2);}

/* variables */
vardec : VAR ID ASSIGN exp {$$ = A_VarDec(EM_tokPos, S_Symbol($2), NULL, $4);}
        | VAR ID COLON ID ASSIGN exp {$$ = A_VarDec(EM_tokPos, S_Symbol($2),S_Symbol($4), $6);}

// vardeclist : vardec %prec LOWEST 
//            | vardec vardeclist 

/* data type dec */

tydec : TYPE ID EQ ty {$$ = A_Namety(S_Symbol($2), $4);}

// tydec  : tydec_ {$$ = $1;}

tydeclist : tydec %prec LOWEST {$$ = A_NametyList($1, NULL);}
          | tydec tydeclist {$$ = A_NametyList($1, $2);}

ty : ID {$$ = A_NameTy(EM_tokPos, S_Symbol($1)); }
   | LBRACE tyfieldlist RBRACE {$$ = A_RecordTy(EM_tokPos, $2);}
   | ARRAY OF ID {$$ = A_ArrayTy(EM_tokPos, S_Symbol($3));}

tyfield : ID COLON ID {$$ = A_Field(EM_tokPos, S_Symbol($1), S_Symbol($3));}

tyfieldlist : {$$ = NULL;}
            | tyfield {$$ = A_FieldList($1, NULL);}
            | tyfield COMMA tyfieldlist {$$ = A_FieldList($1, $3);}

/* filed */



  
