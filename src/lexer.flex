%option noyywrap
%{
#include <stdio.h>
#include <string.h>
#include "formulaFactory.hpp"
#include "y.tab.h"
%}
%%
[a-zA-Z'][0-9a-zA-Z']*    yylval.string=new std::string(yytext,yyleng); return WORD;
"<"                     return LANGLE;
"["                     return LSQUARED;
"]"                     return RSQUARED;
">"                     return RANGLE;
"&"|"&&"                return AND; 
"~"|"!"                 return NOT;
"-"                     return MINUS;
"|"|"||"                return OR;
"("                     return LPAREN;
")"                     return RPAREN;
\n                      /* ignore end of line */;
\r                      /* ignore end of line */;
\t                      /* ignore end of line */;
" "                     /* Leave spaces aside */;
.                       return ERROR; // Everything else
%%
