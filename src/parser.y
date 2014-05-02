%{
#include <stdio.h>
#include "formulaFactory.hpp"
#include <boost/smart_ptr.hpp>
int yylex(void);
void yyerror(const char *);
int mainFormulaNr;
%}

%union 
{
        std::string *string;
        int formulaTablePointer;
}

// Symbols and precedence.
%token <string> WORD
%token ERROR
%left OR MINUS
%left AND
%left RPAREN LPAREN
%left LANGLE RANGLE LSQUARED RSQUARED NOT 
%type <formulaTablePointer> Formula 
%start Expression
%%


Expression:
    Formula { mainFormulaNr = $1; }

Formula: 
      Formula OR Formula { $$ = formulaFactory.getOr($1,$3); }
    | Formula MINUS RANGLE Formula { $$ = formulaFactory.getOr(formulaFactory.getNot($1),$4); }
    | Formula AND Formula { $$ = formulaFactory.getAnd($1,$3); }
    | LPAREN Formula RPAREN { $$ = $2; }
    | NOT Formula { $$ = formulaFactory.getNot($2); }
    | MINUS Formula { $$ = formulaFactory.getNot($2); }
    | LANGLE WORD RANGLE Formula { $$ = formulaFactory.getDiamondTemporalFormula($2,$4); delete $2; }
    | LSQUARED WORD RSQUARED Formula { $$ = formulaFactory.getBoxTemporalFormula($2,$4); delete $2; }
    | WORD { $$ = formulaFactory.getPropositionalFormula($1); delete $1;}
    ;

