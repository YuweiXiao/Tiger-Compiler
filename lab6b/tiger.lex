%{
#include <string.h>
#include "util.h"
#include "symbol.h"
#include "errormsg.h"
#include "absyn.h"
#include "prabsyn.h"
#include "y.tab.h"

int charPos=1;
int startComment = 0;
int yywrap(void)
{
 charPos=1;
 return 1;
}

void adjust(void)
{
 EM_tokPos = charPos;
 charPos += yyleng;
}
/*
* Please don't modify the lines above.
* You can add C declarations of your own below.
*/

%}
  /* You can add lex definitions here. */
digits [0-9]+ 
%Start INITINAL COMMENT
%%
  /* 
  * Below are some examples, which you can wipe out
  * and write reguler expressions and actions of your own.
  */  
<INITINAL>"/*" {adjust(); BEGIN COMMENT;}  // comment
<COMMENT>"*/" {adjust(); BEGIN INITINAL;}  // comment
<COMMENT>.    {fflush(stdout); adjust();}
<COMMENT>"\n"    {adjust();EM_newline();}
<INITINAL>for  { adjust(); return FOR;}     /* key word*/
<INITINAL>while  { adjust(); return WHILE;}
<INITINAL>to  { adjust(); return TO;}
<INITINAL>break  { adjust(); return BREAK;}
<INITINAL>let  { adjust(); return LET;}
<INITINAL>in  { adjust(); return IN;}
<INITINAL>end  { adjust(); return END;}
<INITINAL>function  { adjust(); return FUNCTION;}
<INITINAL>var  { adjust(); return VAR;}
<INITINAL>type  { adjust(); return TYPE;}
<INITINAL>array  { adjust(); return ARRAY;}
<INITINAL>if  { adjust(); return IF;}
<INITINAL>then  { adjust(); return THEN;}
<INITINAL>else  { adjust(); return ELSE;}
<INITINAL>do  { adjust(); return DO;}
<INITINAL>of  { adjust(); return OF;}
<INITINAL>nil  { adjust(); return NIL;}
<INITINAL>","	 {adjust(); return COMMA;}    //symbol
<INITINAL>":"  {adjust(); return COLON;}
<INITINAL>";"  {adjust(); return SEMICOLON;}
<INITINAL>"("  {adjust(); return LPAREN;}
<INITINAL>")"  {adjust(); return RPAREN;}
<INITINAL>"["  {adjust(); return LBRACK;}
<INITINAL>"]"  {adjust(); return RBRACK;}
<INITINAL>"{"  {adjust(); return LBRACE;}
<INITINAL>"}"  {adjust(); return RBRACE;}
<INITINAL>"."  {adjust(); return DOT;}
<INITINAL>"+"  {adjust(); return PLUS;}
<INITINAL>"-"  {adjust(); return MINUS;}
<INITINAL>"*"  {adjust(); return TIMES;}
<INITINAL>"/"  {adjust(); return DIVIDE;}
<INITINAL>"="  {adjust(); return EQ;}
<INITINAL>"<>"  {adjust(); return NEQ;}
<INITINAL>"<"  {adjust(); return LT;}
<INITINAL>"<="  {adjust(); return LE;}
<INITINAL>">"  {adjust(); return GT;}
<INITINAL>">="  {adjust(); return GE;}
<INITINAL>"&"  {adjust(); return AND;}
<INITINAL>"|"  {adjust(); return OR;}
<INITINAL>":="  {adjust(); return ASSIGN;}
<INITINAL>[a-zA-Z]["_"|a-zA-Z0-9]* {// identifier
  adjust(); 
  int size = strlen(yytext);
  yylval.sval = checked_malloc(sizeof(char)*size);
  strcpy(yylval.sval, yytext);
  return ID;
} 
<INITINAL>"\""([a-zA-Z0-9]|"/"|"!"|" "|">"|"\\"|"\\n"|"."|"_"|"-"|"\\t")*"\"" {// string
  adjust(); 
  int size = strlen(yytext), i, p = 0;
  if(size == 2) {
    yylval.sval = "";
    // yylval.sval = "(null)";
    return STRING;
  }
  yylval.sval = checked_malloc(sizeof(char)*size);
  for(i = 1; i < size - 1; ++i) {
    // if(yytext[i] == '\\') {
    //   if(yytext[i+1] == 'n') {
    //     yylval.sval[p] = '\n';  
    //   } else if(yytext[i+1] == 't') {
    //     yylval.sval[p] = '\t';  
    //   } else if(yytext[i+1] == '0') { 
    //     yylval.sval[p] = '\0';  
    //   }
    //   i++;
    // } else {
      yylval.sval[p] = yytext[i];  
    // }
    p++;
  }
  yylval.sval[p] = '\0';
  return STRING;
}       
<INITINAL>{digits}   {adjust(); yylval.ival=atoi(yytext); return INT;}
<INITINAL>(" "|"\t")+  {adjust(); continue;} 
<INITINAL>"\n"   {adjust(); EM_newline(); continue;}
<INITINAL>.	 {adjust(); EM_error(EM_tokPos,"illegal token");}
.           {yyless(0);BEGIN INITINAL;}




