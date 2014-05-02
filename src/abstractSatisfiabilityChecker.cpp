#include "abstractSatisfiabilityChecker.hpp"
#include "formulaFactory.hpp"

ThreeValueBool ThreeValueBool::FALSE(3);
ThreeValueBool ThreeValueBool::TRUE(2);
ThreeValueBool ThreeValueBool::X(1);
ThreeValueBool ThreeValueBool::UNINITIALISED(0);

const int ThreeValueBool::threeValueBoolCombinationTableAND[] = {0,0,0,0, 0,1,1,3, 0,1,2,3, 0,3,3,3};
const int ThreeValueBool::threeValueBoolCombinationTableOR[] = {0,0,0,0, 0,1,2,1, 0,2,2,2, 0,1,2,3};
const int ThreeValueBool::threeValueBoolCombinationTableNOT[] = {0,1,3,2};

ThreeValueBool AbstractSatisfiabilityChecker::recurse(int subformula, int from, int to) {

    // Literal?
    if (subformula<0) return ThreeValueBool::X;

    //std::cerr << "Running: " << subformula << " as " << formulaFactory.getFormulas().at(subformula).get<0>() << "," << from << "," << to << std::endl;
    // Caching
    assert(from<=to);
    assert(from>=0);
    assert(to<wordLength);
    auto itOldValue = doneList.find(boost::make_tuple(subformula,from,to));
    if (itOldValue!=doneList.end()) return itOldValue->second;

    // Evaluate new sub-formula
    FormulaType formulaType = formulaFactory.getFormulas().at(subformula).get<0>();
    const std::set<int> &params = formulaFactory.getFormulas().at(subformula).get<1>();
    ThreeValueBool result;
    switch (formulaType) {
    case TF_AND:
        /* Special Case: Conjunction between the negation of an atomic proposition with itself
         * -> Detect the encoding of "FALSE"
         */
        {
            bool foundSpecialCase = false;
            result = true; // Initialize here already so that the compiler does not complain
            if (params.size()==2) {
                bool foundPos = false;
                bool foundNeg = false;
                int apNumber = 0;
                for (auto it = params.begin();it!=params.end();it++) {
                    if (*it < 0) {
                        if (apNumber!=0) {
                            apNumber = *it;
                            foundPos = true;
                        } else if (*it == apNumber) {
                            foundPos = true;
                        }
                    } else {
                        if (formulaFactory.getFormulas()[*it].get<0>()==TF_NOT) {
                            int negatedLiteral = formulaFactory.getSingleParameterOfTemporalSubformula(*it);
                            if (apNumber==0) {
                                apNumber = negatedLiteral;
                                foundNeg = true;
                            } else {
                                if (negatedLiteral==apNumber) {
                                    foundNeg = true;
                                }
                            }
                        }

                    }
                }
                if (foundPos && foundNeg) {
                    foundSpecialCase = true;
                    result = ThreeValueBool::FALSE;
                }
            }
            if (!foundSpecialCase) {
                for (auto it2 = params.begin();it2!=params.end();it2++) {
                    result &= recurse(*it2,from,to);
                }
            }
        }
        break;
    case TF_OR:
        result = ThreeValueBool::FALSE;
        for (auto it2 = params.begin();it2!=params.end();it2++) {
            result |= recurse(*it2,from,to);
        }
        break;
    case TF_NOT:
        {
            int negatedLiteral = formulaFactory.getSingleParameterOfTemporalSubformula(subformula);
            if (negatedLiteral>0) {
                throw std::string("TF_NOT may only be applied to atomic propositions!");
            } else {
                result = ThreeValueBool::X;
            }
        }
        break;
    case TF_DIAMOND_A:
        {
            auto it2 = formulaFactory.getFormulaNrs().find(boost::make_tuple(TF_DIAMOND_B_BAR,params));
            assert(it2!=formulaFactory.getFormulaNrs().end());
            int relevantBBarSubformula = it2->second;
            result = recurse(relevantBBarSubformula,to,to);
        }
        break;
    case TF_DIAMOND_B:
        result = ThreeValueBool::FALSE;
        for (int k=to-1;k>=from;k--) {
            result |= recurse(formulaFactory.getSingleParameterOfTemporalSubformula(subformula),from,k);
        }
        break;
    case TF_DIAMOND_E:
        result = ThreeValueBool::FALSE;
        for (int k=from+1;k<=to;k++) {
            result |= recurse(formulaFactory.getSingleParameterOfTemporalSubformula(subformula),k,to);
        }
        break;
    case TF_DIAMOND_A_BAR:
        result = ThreeValueBool::FALSE;
        for (int k=0;k<from;k++) {
            int intermediate = formulaFactory.getSingleParameterOfTemporalSubformula(subformula);
            result |= recurse(intermediate,k,from);
        }
        break;
    case TF_DIAMOND_E_BAR:
        result = ThreeValueBool::FALSE;
        for (int k=from-1;k>=0;k--) {
            result |= recurse(formulaFactory.getSingleParameterOfTemporalSubformula(subformula),k,to);
        }
        break;
    case TF_DIAMOND_B_BAR:
        if (to>=(wordLength-1)) {
            result = ThreeValueBool::FALSE;
        } else {
            result = recurse(subformula,from,to+1) | recurse(formulaFactory.getSingleParameterOfTemporalSubformula(subformula),from,to+1);
        }
        break;
    case TF_BOX_A:
        {
            auto it2 = formulaFactory.getFormulaNrs().find(boost::make_tuple(TF_BOX_B_BAR,params));
            assert(it2!=formulaFactory.getFormulaNrs().end());
            int relevantBBarSubformula = it2->second;
            result = recurse(relevantBBarSubformula,to,to);
        }
        break;
    case TF_BOX_B:
        result = recurse(formulaFactory.getSingleParameterOfTemporalSubformula(subformula),from,to);
        if (to==from) {
            result = ThreeValueBool::TRUE;
        } else {
            result &= recurse(subformula,from,to-1);
        }
        break;
    case TF_BOX_E:
        result = recurse(formulaFactory.getSingleParameterOfTemporalSubformula(subformula),from,to);
        if (to==from) {
            result = ThreeValueBool::TRUE;
        } else {
            result &= recurse(subformula,from+1,to);
        }
        break;
    case TF_BOX_A_BAR:
        result = ThreeValueBool::TRUE;
        for (int k=0;k<from;k++) {
            result &= recurse(formulaFactory.getSingleParameterOfTemporalSubformula(subformula),k,from);
        }
        break;
    case TF_BOX_B_BAR:
        result = ThreeValueBool::TRUE;
        if (to>=(wordLength-1)) {
            result = ThreeValueBool::TRUE;
        } else {
            result = recurse(subformula,from,to+1) & recurse(formulaFactory.getSingleParameterOfTemporalSubformula(subformula),from,to+1);
        }
        break;
    case TF_BOX_E_BAR:
        result = ThreeValueBool::TRUE;
        for (int k=from-1;k>=0;k--) {
            result &= recurse(formulaFactory.getSingleParameterOfTemporalSubformula(subformula),k,to);
        }
        break;
    default:
        std::cerr << "Code Found:" << formulaType << std::endl;
        std::cerr << "Code Example: " << TF_BOX_E << std::endl;
        throw std::string("Error: Illegal subformula or unimplemented type during abstract satisfiability checking");
    }

    // Store result
    assert(result!=ThreeValueBool::UNINITIALISED);
    doneList[boost::make_tuple(subformula,from,to)] = result;
    return result;
}
