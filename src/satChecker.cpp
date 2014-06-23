#include "satChecker.hpp"
#include <cstdio>
#include "boost/tuple/tuple_io.hpp"
#include "abstractSatisfiabilityChecker.hpp"

#define INCREMENTAL_SOLVING
#define PICOSAT_ADD_0 { picosat_add(picosat,0); nofClausesSoFar++; }
#define PICOSAT_ADD(x) { assert(x!=0); picosat_add(picosat,x); }

SatisfiabilityChecker::SatisfiabilityChecker(int formula) {
    picosat = picosat_init();
    nofPicosatVariablesUsedSoFar = 0;
    wordLengthBoundSoFar = -1; // Allocate all varibales on the first call to "extendWordLengthBound"
    mainFormulaNumber = formula;
    nofVariablesSoFar = 0;
    nofClausesSoFar = 0;
}

SatisfiabilityChecker::~SatisfiabilityChecker() {
    picosat_reset(picosat);
}

/**
 * @brief Main encoding function. Allocates new SAT variables for the encoding, adds the clauses needed for it, and additionally
 *        introduces assumptions that make sure that no intervals that touch the boundaries of the word are fulfilled.
 *        When assigning the variables, it is always made sure that the variable for the length of the word + 1 are already
 *        registered, such that assumptions about them can be made. The encoding assumes that the formula is written in positive
 *        normal form, i.e., all negations are pushed inwards as much as possible, such that setting variables to FALSE that correspond
 *        to the valuation of variables that touch the boundaries of the word (+1) can always be set to FALSE.
 *
 *        The procedure assumes that "formulaNrs" of the formulaFactory is stripped of all sub-formulas that are not used by the main
 *        formula.
 * @param newLength The new length of the interval.
 */
void SatisfiabilityChecker::extendWordLengthBound(int newLength) {

    // Allocate new AP variables
    const std::map<std::string,int> &apNrs = formulaFactory.getAPNrs();
    for (auto it = apNrs.begin();it!=apNrs.end();it++) {
        for (int i=0;i<newLength+1;i++) {
            for (int j=i;j<newLength+1;j++) {
                if ((i>wordLengthBoundSoFar) || (j>wordLengthBoundSoFar)) {
                    satInstanceSubformulaMapping[boost::make_tuple(it->second,i,j)] = ++nofPicosatVariablesUsedSoFar;
                    //std::cerr << "Allocated SAT Variable: " << boost::make_tuple(it->second,i,j) << " " << nofPicosatVariablesUsedSoFar << std::endl;
                    nofVariablesSoFar++;
                }
            }
        }
    }

    // Allocate new Formula variables
    auto formulaNrs = formulaFactory.getFormulaNrs();
    for (auto it = formulaNrs.begin();it!=formulaNrs.end();it++) {
        for (int i=0;i<newLength+1;i++) {
            for (int j=i;j<newLength+1;j++) {
                if ((i>wordLengthBoundSoFar) || (j>wordLengthBoundSoFar)) {
                    satInstanceSubformulaMapping[boost::make_tuple(it->second,i,j)] = ++nofPicosatVariablesUsedSoFar;
                    nofVariablesSoFar++;
                    //std::cerr << "Allocated SAT Variable for formula: " << boost::make_tuple(it->second,i,j) << " " << nofPicosatVariablesUsedSoFar << std::endl;
                }
            }
        }
    }

    // Allocate new word length boundary variables
    while ((int)(wordLengthBoundaryVariables.size())<newLength+1) {
        wordLengthBoundaryVariables.push_back(++nofPicosatVariablesUsedSoFar);
    }

    // Re-init
#ifndef INCREMENTAL_SOLVING
    picosat_reset(picosat);
    picosat = picosat_init();
#endif

    // Add new formulas for the temporal operators
    for (int i=0;i<newLength;i++) {
        for (int j=i;j<newLength;j++) {
#ifdef INCREMENTAL_SOLVING
            if ((i>=wordLengthBoundSoFar) || (j>=wordLengthBoundSoFar))
#endif
            {
                auto formulaNrs = formulaFactory.getFormulaNrs();
                for (auto it = formulaNrs.begin();it!=formulaNrs.end();it++) {
                    switch (it->first.get<0>()) {
                    case TF_AND:
                        for (auto it2 = it->first.get<1>().begin();it2!=it->first.get<1>().end();it2++) {
                            PICOSAT_ADD(-1*satInstanceSubformulaMapping[boost::make_tuple(it->second,i,j)]);
                            PICOSAT_ADD(satInstanceSubformulaMapping[boost::make_tuple(*it2,i,j)])
                            PICOSAT_ADD_0
                        }
                        break;
                    case TF_OR:
                        PICOSAT_ADD(-1*satInstanceSubformulaMapping[boost::make_tuple(it->second,i,j)]);
                        for (auto it2 = it->first.get<1>().begin();it2!=it->first.get<1>().end();it2++) {
                            //std::cerr << "U: " << i << "," << j << ", Pointer" << *it2 << std::endl;
                            PICOSAT_ADD(satInstanceSubformulaMapping[boost::make_tuple(*it2,i,j)]);
                        }
                        PICOSAT_ADD_0
                        break;
                    case TF_NOT:
                        {
                            int negatedLiteral = formulaFactory.getSingleParameterOfTemporalSubformula(it->second);
                            if (negatedLiteral>0) {
                                throw std::string("TF_NOT may only be applied to atomic propositions!");
                            } else {
                                PICOSAT_ADD(satInstanceSubformulaMapping[boost::make_tuple(it->second,i,j)]);
                                PICOSAT_ADD(satInstanceSubformulaMapping[boost::make_tuple(negatedLiteral,i,j)]);
                                PICOSAT_ADD_0;
                                // Speed up solving: Add a (redundant) clause for the converse direction
                                PICOSAT_ADD(-1*satInstanceSubformulaMapping[boost::make_tuple(it->second,i,j)]);
                                PICOSAT_ADD(-1*satInstanceSubformulaMapping[boost::make_tuple(negatedLiteral,i,j)]);
                                PICOSAT_ADD_0;
                            }
                        }
                        break;
                    case TF_DIAMOND_A:
                        {
                            auto it2 = formulaFactory.getFormulaNrs().find(boost::make_tuple(TF_DIAMOND_B_BAR,it->first.get<1>()));
                            assert(it2!=formulaFactory.getFormulaNrs().end());
                            int relevantBBarSubformula = it2->second;
                            PICOSAT_ADD(-1*satInstanceSubformulaMapping[boost::make_tuple(it->second,i,j)]);
                            PICOSAT_ADD(satInstanceSubformulaMapping[boost::make_tuple(relevantBBarSubformula,j,j)]);
                            PICOSAT_ADD_0;
                        }
                        break;
                    case TF_DIAMOND_B:
                        PICOSAT_ADD(-1*satInstanceSubformulaMapping[boost::make_tuple(it->second,i,j)]);
                        for (int k=j-1;k>=i;k--) {
                            PICOSAT_ADD(satInstanceSubformulaMapping[boost::make_tuple(formulaFactory.getSingleParameterOfTemporalSubformula(it->second),i,k)]);
                        }
                        PICOSAT_ADD_0;
                        break;
                    case TF_DIAMOND_E:
                        PICOSAT_ADD(-1*satInstanceSubformulaMapping[boost::make_tuple(it->second,i,j)]);
                        for (int k=i+1;k<=j;k++) {
                            PICOSAT_ADD(satInstanceSubformulaMapping[boost::make_tuple(formulaFactory.getSingleParameterOfTemporalSubformula(it->second),k,j)]);
                        }
                        PICOSAT_ADD_0;
                        break;
                    case TF_DIAMOND_A_BAR:
                        {
                            PICOSAT_ADD(-1*satInstanceSubformulaMapping[boost::make_tuple(it->second,i,j)]);
                            for (int k=0;k<i;k++) {
                                PICOSAT_ADD(1*satInstanceSubformulaMapping[boost::make_tuple(formulaFactory.getSingleParameterOfTemporalSubformula(it->second),k,i)]);
                            }
                            PICOSAT_ADD_0;
                            break;
                        }
                    case TF_DIAMOND_E_BAR:
                        {
                            PICOSAT_ADD(-1*satInstanceSubformulaMapping[boost::make_tuple(it->second,i,j)]);
                            for (int k=i-1;k>=0;k--) {
                                PICOSAT_ADD(1*satInstanceSubformulaMapping[boost::make_tuple(formulaFactory.getSingleParameterOfTemporalSubformula(it->second),k,j)]);
                            }
                            PICOSAT_ADD_0;
                            break;
                        }
                    case TF_DIAMOND_B_BAR:
                        PICOSAT_ADD(-1*satInstanceSubformulaMapping[boost::make_tuple(it->second,i,j)]);
                        PICOSAT_ADD(1*satInstanceSubformulaMapping[boost::make_tuple(it->second,i,j+1)]);
                        PICOSAT_ADD(satInstanceSubformulaMapping[boost::make_tuple(formulaFactory.getSingleParameterOfTemporalSubformula(it->second),i,j+1)]);
                        PICOSAT_ADD_0;
                        break;
                    case TF_BOX_A:
                        {
                            auto it2 = formulaFactory.getFormulaNrs().find(boost::make_tuple(TF_BOX_B_BAR,it->first.get<1>()));
                            assert(it2!=formulaFactory.getFormulaNrs().end());
                            int relevantBBarSubformula = it2->second;
                            PICOSAT_ADD(-1*satInstanceSubformulaMapping.at(boost::make_tuple(it->second,i,j)));
                            PICOSAT_ADD(satInstanceSubformulaMapping.at(boost::make_tuple(relevantBBarSubformula,j,j)));
                            PICOSAT_ADD_0;
                        }
                        break;
                    case TF_BOX_B:
                        for (int k=j-1;k>=i;k--) {
                            PICOSAT_ADD(-1*satInstanceSubformulaMapping[boost::make_tuple(it->second,i,j)]);
                            PICOSAT_ADD(satInstanceSubformulaMapping[boost::make_tuple(formulaFactory.getSingleParameterOfTemporalSubformula(it->second),i,k)]);
                            PICOSAT_ADD_0;
                        }
                        break;
                    case TF_BOX_E:
                        for (int k=i+1;k<=j;k++) {
                            PICOSAT_ADD(-1*satInstanceSubformulaMapping[boost::make_tuple(it->second,i,j)]);
                            PICOSAT_ADD(satInstanceSubformulaMapping[boost::make_tuple(formulaFactory.getSingleParameterOfTemporalSubformula(it->second),k,j)]);
                            PICOSAT_ADD_0;
                        }
                        break;
                    case TF_BOX_A_BAR:
                        {
                            for (int k=0;k<i;k++) {
                                PICOSAT_ADD(-1*satInstanceSubformulaMapping[boost::make_tuple(it->second,i,j)]);
                                PICOSAT_ADD(1*satInstanceSubformulaMapping[boost::make_tuple(formulaFactory.getSingleParameterOfTemporalSubformula(it->second),k,i)]);
                                PICOSAT_ADD_0;
                            }
                            break;
                        }
                    case TF_BOX_B_BAR:
                        PICOSAT_ADD(-1*satInstanceSubformulaMapping.at(boost::make_tuple(it->second,i,j)));
                        PICOSAT_ADD(1*satInstanceSubformulaMapping.at(boost::make_tuple(it->second,i,j+1)));
                        PICOSAT_ADD(-1*wordLengthBoundaryVariables.at(j+1));
                        PICOSAT_ADD_0;
                        PICOSAT_ADD(-1*satInstanceSubformulaMapping.at(boost::make_tuple(it->second,i,j)));
                        PICOSAT_ADD(-1*wordLengthBoundaryVariables.at(j+1));
                        PICOSAT_ADD(satInstanceSubformulaMapping.at(boost::make_tuple(formulaFactory.getSingleParameterOfTemporalSubformula(it->second),i,j+1)));
                        PICOSAT_ADD_0;
                        break;
                    case TF_BOX_E_BAR:
                        {
                            for (int k=i-1;k>=0;k--) {
                                PICOSAT_ADD(-1*satInstanceSubformulaMapping[boost::make_tuple(it->second,i,j)]);
                                PICOSAT_ADD(1*satInstanceSubformulaMapping[boost::make_tuple(formulaFactory.getSingleParameterOfTemporalSubformula(it->second),k,j)]);
                                PICOSAT_ADD_0;
                            }
                            break;
                        }
                    default:
                        std::cerr << "Code Found:" << it->first.get<0>() << std::endl;
                        std::cerr << "Code Example: " << TF_BOX_E << std::endl;
                        throw std::string("Error: Illegal subformula or unimplemented type during building the incremental SAT instance.");
                    }
                }
            }
        }
    }

    // Make sure that the new formula is satisfied (only to be added in the first iteration).
#ifdef INCREMENTAL_SOLVING
    if (wordLengthBoundSoFar==-1)
#endif
    {
        PICOSAT_ADD(1*satInstanceSubformulaMapping[boost::make_tuple(mainFormulaNumber,0,0)]);
        PICOSAT_ADD_0;
    }

    // Word length is at least newLength
#ifdef INCREMENTAL_SOLVING
    for (int i=std::max(0,wordLengthBoundSoFar);i<newLength;i++) {
#else
    for (int i=0;i<newLength;i++) {
#endif
        PICOSAT_ADD(wordLengthBoundaryVariables.at(i));
        PICOSAT_ADD_0;
    }

    // picosat_print(picosat,stderr);

    // Perform assumptions
    for (auto it = satInstanceSubformulaMapping.begin();it!=satInstanceSubformulaMapping.end();it++) {
        if (it->first.get<2>()==newLength) {
            // All polarities are negative such that we never need to check the word boundaries for Diamond-based temporal operators
            picosat_assume(picosat,-1*it->second);
        }
    }

    // Word is not longer than it is
    picosat_assume(picosat,-1*wordLengthBoundaryVariables[newLength]);

    // Assume main formula to be true
    // mainFormulaNumber
    wordLengthBoundSoFar = newLength;

    // Use preprocessor to find trivial intervals
    AbstractSatisfiabilityChecker abstractor(mainFormulaNumber,newLength);
    auto storage = abstractor.getStorage();
    for (auto it = formulaFactory.getFormulaNrs().begin();it!=formulaFactory.getFormulaNrs().end();it++) {
        for (int i=0;i<newLength;i++) {
            for (int j=i;j<newLength;j++) {
                int satVariable = satInstanceSubformulaMapping[boost::make_tuple(it->second,i,j)];
                auto it2 = storage.find(boost::make_tuple(it->second,i,j));
                if (it2==storage.end()) {
                    // Not reachable! Set to FALSE
                    // std::cerr << "NR" << satVariable << " ";
                    picosat_assume(picosat,-1*satVariable);
                } else {
                    if (it2->second==ThreeValueBool::FALSE) {
                        // std::cerr << "NS" << satVariable << " ";
                        picosat_assume(picosat,-1*satVariable);
                    } else if (it2->second==ThreeValueBool::TRUE) {
                        // std::cerr << "FT" << satVariable << " ";
                        picosat_assume(picosat,1*satVariable);
                    } else if (it2->second==ThreeValueBool::X) {
                        // OK
                    } else {
                        throw std::string("Error: Unexpected result of the abstract satisfiability checker.");
                    }
                }
            }
        }
    }

}

bool SatisfiabilityChecker::checkSatisfiabilityUnderBound() {
    int picosatReturnValue = picosat_sat(picosat,-1);
    if (picosatReturnValue==PICOSAT_SATISFIABLE) {
        return true;
    } else if (picosatReturnValue==PICOSAT_UNSATISFIABLE) {
        return false;
    } else {
        throw std::string("Error: Picosat returned a value other than PICOSAT_SATISFIABLE and PICOSAT_UNSATISFIABLE!");
    }
}

void SatisfiabilityChecker::printSubformulaSATMapping() {

    std::cout << "=====================[Subformula to variable mapping]==============================\n";
    for (auto it = formulaFactory.getFormulaNrs().begin();it!=formulaFactory.getFormulaNrs().end();it++) {
        std::cout << "Formula:\n";
        std::cout.flush();
        std::cout << "FID: " << it->second << std::endl;
        formulaFactory.printFormula(it->second,2);
        std::cout << "[";
        for (int i=0;i<=wordLengthBoundSoFar;i++) {
            for (int j=0;j<=wordLengthBoundSoFar;j++) {
                auto finder = satInstanceSubformulaMapping.find(boost::make_tuple(it->second,i,j));
                if (finder == satInstanceSubformulaMapping.end()) {
                    std::cout << "\t";
                } else {
                    std::cout << finder->second << "\t";
                }
            }
            if (i!=wordLengthBoundSoFar) std::cout << "\n"; else std::cout << "]\n";
        }
    }
    std::cout.flush();
}

void SatisfiabilityChecker::printAPtoSATMapping() {
    std::cout.flush();
    std::cout << "=====================[Atomic Proposition to variable mapping]==============================\n";
    for (auto it = formulaFactory.getAPNrs().begin();it!=formulaFactory.getAPNrs().end();it++) {
        std::cerr << "AP: " << it->first << std::endl;
        std::cout << "[";
        for (int i=0;i<=wordLengthBoundSoFar;i++) {
            for (int j=0;j<=wordLengthBoundSoFar;j++) {
                auto finder = satInstanceSubformulaMapping.find(boost::make_tuple(it->second,i,j));
                if (finder == satInstanceSubformulaMapping.end()) {
                    std::cout << "\t";
                } else {
                    std::cout << finder->second << "\t";
                }
            }
            if (i!=wordLengthBoundSoFar) std::cout << "\n"; else std::cout << "]\n";
        }
    }
    std::cout.flush();
}

void SatisfiabilityChecker::addTemporalOperatorsNeededForTheEncoding() {
    // For every <A>... formula, we must also have a <B'>... formula
    std::set<boost::tuple<FormulaType,std::set<int> > > subformulasToBeAdded;
    for (auto it = formulaFactory.getFormulaNrs().begin();it!=formulaFactory.getFormulaNrs().end();it++) {
        if (it->first.get<0>()==TF_DIAMOND_A) {
            subformulasToBeAdded.insert(boost::make_tuple(TF_DIAMOND_B_BAR,it->first.get<1>()));
        } else if (it->first.get<0>()==TF_BOX_A) {
            subformulasToBeAdded.insert(boost::make_tuple(TF_BOX_B_BAR,it->first.get<1>()));
        }
    }
    for (auto it = subformulasToBeAdded.begin();it!=subformulasToBeAdded.end();it++) {
        formulaFactory.insertSubformula(*it);
    }
}

void SatisfiabilityChecker::run(int maxBound) {
    if ((maxBound<1) && (maxBound!=-1)) {
        throw std::string("Maximum bound must be at least 1.");
    }

    mainFormulaNumber = formulaFactory.makePositiveNormalForm(mainFormulaNumber);
    // The following three steps have to be performed in this order: first encode all derived temporal operators
    // as pure ones - this may leave some unused subformulas. Then remove them, as the SAT encoding encodes all
    // operators in the formula table. Finally, add helping subformulas for the encoding. These are technically
    // not connected to the rest of the formulas, so "removeUnreachableSubformulas" must not be called afterwards.
    // Also, removeUnreachableSubformulas must be called as the SAT encoder cannot encode derived operators, so this
    // would lead to an error otherwise.

    mainFormulaNumber = formulaFactory.encodeDerivedTemporalOperators(mainFormulaNumber);
    std::cout << "Formula size: " << formulaFactory.getFormulaNrs().size() << std::endl;
    mainFormulaNumber = formulaFactory.removeUnreachableSubformulas(mainFormulaNumber);
    addTemporalOperatorsNeededForTheEncoding();
    std::cout << "Formula size after encoding: " << formulaFactory.getFormulaNrs().size() << std::endl;

    // The loop
    while (true) {
        int newSize = wordLengthBoundSoFar+1;
        if (newSize==0) newSize+=1; // Words must have length at least 0, otherwise the semantics of the BOX operator is a bit unclear
        if (maxBound+1==newSize) {
            std::cerr << "Result: Aborting due to reaching the maximum bound of " << maxBound << std::endl;
            return;
        }
        std::cerr << "Trying a word of length " << newSize << std::endl;
        //printSubformulaSATMapping();

        extendWordLengthBound(newSize);

        bool isSAT = checkSatisfiabilityUnderBound();
        if (isSAT) {
            std::cerr << "Result: The temporal logic formula is satisfiable for a word of length " << newSize << std::endl;
            std::cerr << "We needed " << nofVariablesSoFar << " SAT Variables and " << nofClausesSoFar << " clauses\n";
            //printAPtoSATMapping();
            printSimplifiedSatisfiabilityCertificate();
            //printSatisfiabilityCertificate();
            //printAssignment();
            return;
        } else {
            // printAPtoSATMapping();
            // printSubformulaSATMapping();
        }
    }
}

void SatisfiabilityChecker::printAssignment() {
    int maxVar = picosat_inc_max_var(picosat);
    for (int i=1;i<maxVar;i++) {
        if (picosat_deref(picosat,i)==1) {
            std::cout << i << " ";
        } else {
            std::cout << -1*i << " ";
        }
    }
    std::cout << std::endl;
}

void SatisfiabilityChecker::printSatisfiabilityCertificate() {

    // Basic certificate information
    std::cout << "\n\n=====================[Satisfiability Certificate]==============================\n";
    /*std::cout << "Time line:\n";
    if (wordLengthBoundSoFar==1) {
        std::cout << "|";
    } else {
        std::cout << "|";
        for (int i=0;i<wordLengthBoundSoFar-2;i++) std::cout << "-";
        std::cout << "|";
    }
    std::cout << "\n\n";*/

    // Atomic propositions
    for (auto it = formulaFactory.getAPNrs().begin();it!=formulaFactory.getAPNrs().end();it++) {
        std::set<std::pair<int,int> > intervals;
        std::cout << "AP: " << it->first << std::endl;
        for (int i=0;i<=wordLengthBoundSoFar;i++) {
            for (int j=i;j<=wordLengthBoundSoFar;j++) {
                auto finder = satInstanceSubformulaMapping.find(boost::make_tuple(it->second,i,j));
                if (finder == satInstanceSubformulaMapping.end()) {
                    // Don't care
                } else {
                    // Holds on the interval?
                    if (picosat_deref(picosat,finder->second)==1) {
                        intervals.insert(std::pair<int,int>(i,j-i));
                    }
                }
            }
        }
        drawIntervals(intervals);
        std::cout << std::endl;
    }

    for (auto it = formulaFactory.getFormulaNrs().begin();it!=formulaFactory.getFormulaNrs().end();it++) {
        std::cout << "Subformula: " << it->second << std::endl;
        std::set<std::pair<int,int> > intervals;
        formulaFactory.printFormula(it->second,0);
        for (int i=0;i<=wordLengthBoundSoFar;i++) {
            for (int j=i;j<=wordLengthBoundSoFar;j++) {
                auto finder = satInstanceSubformulaMapping.find(boost::make_tuple(it->second,i,j));
                if (finder == satInstanceSubformulaMapping.end()) {
                    // Don't care
                } else {
                    // Holds on the interval?
                    if (picosat_deref(picosat,finder->second)==1) {
                        intervals.insert(std::pair<int,int>(i,j-i));
                    }
                }
            }
        }
        drawIntervals(intervals);
        std::cout << std::endl;
    }

    // Done!
    std::cout.flush();
}

void SatisfiabilityChecker::printSimplifiedSatisfiabilityCertificate() {

    // Basic certificate information
    std::cout << "\n\n=====================[Satisfiability Certificate]==============================\n";
    /*std::cout << "Time line:\n";
    if (wordLengthBoundSoFar==1) {
        std::cout << "|";
    } else {
        std::cout << "|";
        for (int i=0;i<wordLengthBoundSoFar-2;i++) std::cout << "-";
        std::cout << "|";
    }
    std::cout << "\n\n";*/

    // Atomic propositions
    for (auto it = formulaFactory.getAPNrs().begin();it!=formulaFactory.getAPNrs().end();it++) {
        std::set<std::pair<int,int> > intervals;
        std::cout << "AP: " << it->first << std::endl;
        for (int i=0;i<=wordLengthBoundSoFar;i++) {
            for (int j=i;j<=wordLengthBoundSoFar;j++) {
                auto finder = satInstanceSubformulaMapping.find(boost::make_tuple(it->second,i,j));
                if (finder == satInstanceSubformulaMapping.end()) {
                    // Don't care
                } else {
                    // Holds on the interval?
                    if (picosat_deref(picosat,finder->second)==1) {
                        intervals.insert(std::pair<int,int>(i,j-i));
                    }
                }
            }
        }
        drawIntervals(intervals);
        std::cout << std::endl;
    }


    // Done!
    std::cout.flush();
}


void SatisfiabilityChecker::drawIntervals(std::set<std::pair<int,int> > &intervals) {
    for (auto it = intervals.begin();it!=intervals.end();it++) {
        std::cout << "(" << it->first << "," << it->second+it->first << ") ";
    }
    std::cout << "\n";
}

