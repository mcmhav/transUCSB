%option yylineno
%pointer

%{
#include <stdlib.h>
#include <string.h>
#include "parser.hpp"

void yyerror(const char *);
%}

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

%%
\+                    { return T_plus; }
\*                    { return T_mult; }
\/                    { return T_div; }
\-                    { return T_minus; }
[\*\(\);\+\-\*\/<>=]  { return *yytext; }
[0-9][0-9]*           { printf("%s ", yytext); }
[ \t\n]               ; /* skip whitespace */

.                     yyerror((char *) "invalid character");

%%


/* You shouldn't need to edit any part of this*/

int yywrap(void) {
	return 1;
}

int main(){
  yylex();
  return 0;
}