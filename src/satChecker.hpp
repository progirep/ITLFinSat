#ifndef __SAT_CHECKER_HPP__
#define __SAT_CHECKER_HPP__

#include "formulaFactory.hpp"
#include <map>
#include <boost/tuple/tuple.hpp>
#include <boost/smart_ptr.hpp>
extern "C" {
  #include "picosat.h"
}

/**
 * @brief The main class for the satisfiability check of an ITL formula.
 * Takes a parsed formula as input
 */
class SatisfiabilityChecker {
private:
    // SAT Solving variables
    PicoSAT *picosat;
    int nofPicosatVariablesUsedSoFar;

    // SAT Encoding Variables
    int wordLengthBoundSoFar;
    int nofVariablesSoFar;
    int nofClausesSoFar;

    /**
     * @brief Maps from Subformula-ID x Interval-Start x Interval-End to SAT variable
     */
    std::map<boost::tuple<int,int,int>,int> satInstanceSubformulaMapping;

    // General Problem Instance Variables
    int mainFormulaNumber;

    // Internal functions
    void extendWordLengthBound(int newLength);
    bool checkSatisfiabilityUnderBound();
    void printSubformulaSATMapping();
    void printAPtoSATMapping();
    void printSatisfiabilityCertificate();
    void printSimplifiedSatisfiabilityCertificate();
    void printAssignment();
    void addTemporalOperatorsNeededForTheEncoding();
    static void drawIntervals(std::set<std::pair<int,int> > &intervals);

public:
    SatisfiabilityChecker(int mainFormula);
    ~SatisfiabilityChecker();
    void run(int maxBound);

};



#endif
