#include <iostream>
#include "formulaFactory.hpp"
#include "y.tab.h"
#include "satChecker.hpp"
#include <sstream>

// The singleton instance of our formula factories
FormulaFactory formulaFactory;

// Produced by the parser run.
extern int mainFormulaNr;

// The following declaration is not produced by every version of bison
int yyparse (void);

int yyerror (const char *s) {                                                     
    std::cerr << "Parser error: " << s << std::endl;
    return 0;
}

int main(int argv, const char **args) {
    int maxBound = -1;
    for (int i=1;i<argv;i++) {
        std::string current = args[i];
        if (current=="--max-bound") {
            if (i==argv-1) {
                std::cerr << "Error: No number after parameter --max-bound\n";
                return 1;
            } else {
                std::istringstream is(args[i+1]);
                is >> maxBound;
                if (is.fail()) {
                    std::cerr << "Error: No valid number given after --max-bound\n";
                    return 1;
                }
                i++;
            }
        } else {
            std::cerr << "Error: Did not understand option '" << current << "'\n";
            return 1;
        }
    }
    try {
        int returnValue = yyparse();
        if (returnValue==0) {
            //std::cout << "=========================[Parsed Formula]==========================\n";
            //formulaFactory.printFormula(mainFormulaNr);
            std::cout << "==================[Satisfiability Checking Result]=================\n";
            SatisfiabilityChecker checker(mainFormulaNr);
            checker.run(maxBound);
            return 0;
        }
    } catch (std::string error) {
        std::cerr << "Error: " << error << std::endl;
    }
}

