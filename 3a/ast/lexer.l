%option yylineno
%pointer

%{
#include <stdlib.h>
#include <string.h>
#include "ast.hpp"
#include "parser.hpp"
#include <iostream>

using namespace std;


void yyerror(const char *);
int line_num = 1;
int getIntValue(const char* i, int t);

%}

/* Insert your lexer here. Please don't just overwrite the file because of the enhanced header section. */

/* 
WRITE ME: put any definitions you need here.  Add your own regular expressions
          below for ALL tokens defined in the manual. You can choose not to use
          definitions here.

*/

/*
  Comments will be the hardest - do nothing
  Dont need to think about the lining of the parser and the lexer.
  Will get shift becuase of something, you can end up without shiftcomflict
  If it is reduce it selects the first production rule
  Resolve the conflicts
*/

/*
  The example below is very simple, but it shows almost everything you will need
  to do for this first part.

  The syntax is as follows:

  regular_expression      {  c code  }

  The lexer will try to match each regular expression.  When it does it will execute
  the c code.
*/

/* 
  \+                    { return T_plus; }
  \*                    { return T_mult; }
  \/                    { return T_div; }
  \-                    { return T_minus; }

  [\*\(\);\+\-\*\/<>=]  { return *yytext; }
  [0-9][0-9]*           { printf("%s ", yytext); }
  [0-9]+     { yylval = atoi(yytext); return NUMBER; }


  .        { yyerror("illegal token"); }
  \n         { nextline(); }
  \t         { ; }
  "//".*\n   { nextline(); }


  int main(){
    yylex();
    return 0;
  }
*/

%x COMMENT

%%
"bool"     { return BOOL; }
"else"     { return ELSE; }
"if"       { return IF; }
"int"      { return INT; }
"while"    { return WHILE; }
"var"      { return VAR; }
"function" { return FUNCTION; }
"intarray" { return INTARRAY; }
"return"   { return RETURN; }

"=="       { return EQT; }
">"        { return GT; }
">="       { return GE; }
"<"        { return LT; }
"<="       { return LE; }
"!="       { return NE; }
"&&"       { return AND; }
"||"       { return OR; }
"!"        { return NOT; }
"+"        { return PLUS; }
"-"        { return MINUS; }
"*"        { return MULT; }
"/"        { return DIVIDE; }

"true"          { return TRUE; }
"false"         { return FALSE; }
[1-9][0-9]*     { yylval.u_base_int = getIntValue(yytext, 0);  return INT; }
0               { yylval.u_base_int = getIntValue(yytext, 0);  return INT; }
0[xX][0-9A-F]+  { yylval.u_base_int = getIntValue(yytext, 1);  return INT; }
0[0-7]*         { yylval.u_base_int = getIntValue(yytext, 2);  return INT; }
[0-1]+b         { yylval.u_base_int = getIntValue(yytext, 3);  return INT; }
[a-zA-Z][0-9a-zA-Z]* { yylval.u_base_charptr = strdup(yytext); return IDENTIFIER; }   

";"        { return SEMICOLON; }
","        { return COMMA; }
"|"        { return ABS; }
"{"        { return SC; }
"}"        { return EC; }
")"        { return RPAREN; }
"("        { return LPAREN; }
"["        { return BAI; }
"]"        { return EAI; }
"="        { return EQ; }


\          { ; }
[ \t\n]    ; 
.          yyerror((char *) "invalid character");

"[$"                    { BEGIN COMMENT; line_num = yylineno; }
<COMMENT>"$]"           BEGIN INITIAL;
<COMMENT>\n             ;
<COMMENT>.              ; 
<COMMENT><<EOF>>        {yylineno = line_num; yyerror((char *) "dangling comment starting");}
%%

int yywrap(void) {
	return 1;
}
