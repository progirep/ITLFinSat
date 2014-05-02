#include "formulaFactory.hpp"
#include <cassert>
#include <iostream>

/**
 * @brief Debugging function for simple printing of subformula types
 * @param lhs the output stream
 * @param e the subformula type to print
 * @return the output stream again
 */
std::ostream& operator<<(std::ostream& lhs, FormulaType e) {
    switch(e) {
        case TF_AND: lhs << "AND"; break;
        case TF_OR: lhs << "OR"; break;
        case TF_NOT: lhs << "NOT"; break;
        case TF_DIAMOND_A: lhs << "DIAMOND_A"; break;
        case TF_DIAMOND_B: lhs << "DIAMOND_B"; break;
        case TF_DIAMOND_E: lhs << "DIAMOND_E"; break;
        case TF_DIAMOND_O: lhs << "DIAMOND_O"; break;
        case TF_DIAMOND_L: lhs << "DIAMOND_L"; break;
        case TF_DIAMOND_D: lhs << "DIAMOND_D"; break;
        case TF_DIAMOND_A_BAR: lhs << "DIAMOND_A_BAR"; break;
        case TF_DIAMOND_B_BAR: lhs << "DIAMOND_B_BAR"; break;
        case TF_DIAMOND_E_BAR: lhs << "DIAMOND_E_BAR"; break;
        case TF_DIAMOND_O_BAR: lhs << "DIAMOND_O_BAR"; break;
        case TF_DIAMOND_L_BAR: lhs << "DIAMOND_L_BAR"; break;
        case TF_DIAMOND_D_BAR: lhs << "DIAMOND_D_BAR"; break;
        case TF_BOX_A: lhs << "BOX_A"; break;
        case TF_BOX_B: lhs << "BOX_B"; break;
        case TF_BOX_E: lhs << "BOX_E"; break;
        case TF_BOX_O: lhs << "BOX_O"; break;
        case TF_BOX_L: lhs << "BOX_L"; break;
        case TF_BOX_D: lhs << "BOX_D"; break;
        case TF_BOX_A_BAR: lhs << "BOX_A_BAR"; break;
        case TF_BOX_B_BAR: lhs << "BOX_B_BAR"; break;
        case TF_BOX_E_BAR: lhs << "BOX_E_BAR"; break;
        case TF_BOX_O_BAR: lhs << "BOX_O_BAR"; break;
        case TF_BOX_L_BAR: lhs << "BOX_L_BAR"; break;
        case TF_BOX_D_BAR: lhs << "BOX_D_BAR"; break;
    default:
        lhs << "UNKNOWN(" << (int)e << ")";
    }
    return lhs;
}


void FormulaFactory::printFormula(int nr, int level) {
    assert(nr<(int)(formulas.size()));
    for (int i=0;i<level;i++) std::cout << " ";
    if (nr<0) {
        std::cout << aps[-nr-1] << std::endl;
    } else {
        switch (formulas[nr].get<0>()) {
            case TF_AND:
                std::cout << "AND\n";
                for (std::set<int>::iterator it = formulas[nr].get<1>().begin();it!=formulas[nr].get<1>().end();it++)
                    printFormula(*it, level+2);
                break;
        case TF_OR:
            std::cout << "OR\n";
            for (std::set<int>::iterator it = formulas[nr].get<1>().begin();it!=formulas[nr].get<1>().end();it++)
                printFormula(*it, level+2);
            break;
        case TF_NOT:
            std::cout << "NOT\n";
            assert(formulas[nr].get<1>().size()==1);
            printFormula(*(formulas[nr].get<1>().begin()), level+2);
            break;
#define COVER_TEMPORAL_CASE(a,txt) case a: std::cout << txt << std::endl; assert(formulas[nr].get<1>().size()==1); printFormula(*(formulas[nr].get<1>().begin()), level+2); break;
        COVER_TEMPORAL_CASE(TF_DIAMOND_A,"<A>")
        COVER_TEMPORAL_CASE(TF_DIAMOND_B,"<B>")
        COVER_TEMPORAL_CASE(TF_DIAMOND_E,"<E>")
        COVER_TEMPORAL_CASE(TF_DIAMOND_L,"<L>")
        COVER_TEMPORAL_CASE(TF_DIAMOND_D,"<D>")
        COVER_TEMPORAL_CASE(TF_DIAMOND_O,"<O>")
        COVER_TEMPORAL_CASE(TF_DIAMOND_A_BAR,"<A'>")
        COVER_TEMPORAL_CASE(TF_DIAMOND_B_BAR,"<B'>")
        COVER_TEMPORAL_CASE(TF_DIAMOND_E_BAR,"<E'>")
        COVER_TEMPORAL_CASE(TF_DIAMOND_L_BAR,"<L'>")
        COVER_TEMPORAL_CASE(TF_DIAMOND_D_BAR,"<D'>")
        COVER_TEMPORAL_CASE(TF_DIAMOND_O_BAR,"<O'>")
        COVER_TEMPORAL_CASE(TF_BOX_A,"[A]")
        COVER_TEMPORAL_CASE(TF_BOX_B,"[B]")
        COVER_TEMPORAL_CASE(TF_BOX_E,"[E]")
        COVER_TEMPORAL_CASE(TF_BOX_L,"[L]")
        COVER_TEMPORAL_CASE(TF_BOX_D,"[D]")
        COVER_TEMPORAL_CASE(TF_BOX_O,"[O]")
        COVER_TEMPORAL_CASE(TF_BOX_A_BAR,"[A']")
        COVER_TEMPORAL_CASE(TF_BOX_B_BAR,"[B']")
        COVER_TEMPORAL_CASE(TF_BOX_E_BAR,"[E']")
        COVER_TEMPORAL_CASE(TF_BOX_L_BAR,"[L']")
        COVER_TEMPORAL_CASE(TF_BOX_D_BAR,"[D']")
        COVER_TEMPORAL_CASE(TF_BOX_O_BAR,"[O']")
        default:
            throw std::string("Error: Known subformula type encountered during printing a subformula");
        }
    }
}

int FormulaFactory::makePositiveNormalForm(int startingFormula, bool negated) {

    // Atomic proposition
    if (startingFormula<0) {
        if (!negated) {
            return startingFormula;
        } else {
            std::set<int> res;
            res.insert(startingFormula);
            return insertSubformula(boost::make_tuple(TF_NOT,res));
        }
    }

    // Negation
    if (formulas[startingFormula].get<0>() == TF_NOT) {
        if (getSingleParameterOfTemporalSubformula(startingFormula)<0) {
            if (negated) {

                return getSingleParameterOfTemporalSubformula(startingFormula);
            } else {
                return startingFormula;
            }
        } else {
            return makePositiveNormalForm(getSingleParameterOfTemporalSubformula(startingFormula),!negated);
        }
    }

    // All other cases
    std::set<int> res;
    std::set<int> orig = formulas[startingFormula].get<1>();
    for (auto it = orig.begin();it!=orig.end();it++) {
        int thisResult = makePositiveNormalForm(*it,negated);
        assert(thisResult!=startingFormula);
        res.insert(thisResult);
    }

    FormulaType newType;
    if (negated) {
        switch (formulas[startingFormula].get<0>()) {
        case TF_AND:
            newType = TF_OR; break;
        case TF_OR:
            newType = TF_AND; break;
        case TF_DIAMOND_A:
            newType = TF_BOX_A; break;
        case TF_DIAMOND_B:
            newType = TF_BOX_B; break;
        case TF_DIAMOND_E:
            newType = TF_BOX_E; break;
        case TF_DIAMOND_A_BAR:
            newType = TF_BOX_A_BAR; break;
        case TF_DIAMOND_B_BAR:
            newType = TF_BOX_B_BAR; break;
        case TF_DIAMOND_E_BAR:
            newType = TF_BOX_E_BAR; break;
        case TF_DIAMOND_L:
            newType = TF_BOX_L; break;
        case TF_DIAMOND_D:
            newType = TF_BOX_D; break;
        case TF_DIAMOND_O:
            newType = TF_BOX_O; break;
        case TF_DIAMOND_L_BAR:
            newType = TF_BOX_L_BAR; break;
        case TF_DIAMOND_D_BAR:
            newType = TF_BOX_D_BAR; break;
        case TF_DIAMOND_O_BAR:
            newType = TF_BOX_O_BAR; break;
        case TF_BOX_A:
            newType = TF_DIAMOND_A; break;
        case TF_BOX_B:
             newType = TF_DIAMOND_B; break;
        case TF_BOX_E:
             newType = TF_DIAMOND_E; break;
        case TF_BOX_A_BAR:
             newType = TF_DIAMOND_A_BAR; break;
        case TF_BOX_B_BAR:
             newType = TF_DIAMOND_B_BAR; break;
        case TF_BOX_E_BAR:
             newType = TF_DIAMOND_E_BAR; break;
        case TF_BOX_L:
             newType = TF_DIAMOND_L; break;
        case TF_BOX_D:
             newType = TF_DIAMOND_D; break;
        case TF_BOX_O:
             newType = TF_DIAMOND_O; break;
        case TF_BOX_L_BAR:
             newType = TF_DIAMOND_L_BAR; break;
        case TF_BOX_D_BAR:
             newType = TF_DIAMOND_D_BAR; break;
        case TF_BOX_O_BAR:
             newType = TF_DIAMOND_O_BAR; break;
        default:
            std::cerr << "Offending type: " << formulas[startingFormula].get<0>() << std::endl;
            throw std::string("Did not find a formula type during translation into positive normal form.");
        }
    } else {
        newType = formulas[startingFormula].get<0>();
    }
    return insertSubformula(boost::make_tuple(newType,res));
}

void FormulaFactory::printFormulaTable() {
    std::cerr << "Formula Table: \n";
    for (uint i=0;i<formulas.size();i++) {
        std::cerr << i << "\t" << formulas[i].get<0>() << "\t{";
        for (auto it = formulas[i].get<1>().begin();it!=formulas[i].get<1>().end();it++) {
            std::cerr << *it << " ";
        }
        std::cerr << "}\t" << formulaNrs[formulas[i]];
        std::cerr << "\n";
    }

}

/**
 * @brief Replaces derived temporal operators by non-derived ones (A,B,E,A',B',E')
 * @param startingFormula
 * @return the new handle of the starting formula, which may change
 */
int FormulaFactory::encodeDerivedTemporalOperators(int startingFormula) {

    // Atomic proposition
    if (startingFormula<0) return startingFormula;

    // If not, then recurse
    std::set<int> res;
    std::set<int> from = formulas[startingFormula].get<1>();
    for (auto it = from.begin();it!=from.end();it++) {
        res.insert(encodeDerivedTemporalOperators(*it));
    }


    switch (formulas[startingFormula].get<0>()) {
    case TF_DIAMOND_L:
        {
            int aSubFormula = insertSubformula(boost::make_tuple(TF_DIAMOND_A,res));
            std::set<int> params;
            params.insert(aSubFormula);
            return insertSubformula(boost::make_tuple(TF_DIAMOND_A,params));
        }
        // No 'break' necessary, since the previous command is a return statement
    case TF_BOX_L:
        {
            int aSubFormula = insertSubformula(boost::make_tuple(TF_BOX_A,res));
            std::set<int> params;
            params.insert(aSubFormula);
            return insertSubformula(boost::make_tuple(TF_BOX_A,params));
        }
        // No 'break' necessary, since the previous command is a return statement
    case TF_DIAMOND_O:
        {
            int aSubFormula = insertSubformula(boost::make_tuple(TF_DIAMOND_B_BAR,res));
            std::set<int> params;
            params.insert(aSubFormula);
            return insertSubformula(boost::make_tuple(TF_DIAMOND_E,params));
        }
        // No 'break' necessary, since the previous command is a return statement
    case TF_BOX_O:
        {
            int aSubFormula = insertSubformula(boost::make_tuple(TF_BOX_B_BAR,res));
            std::set<int> params;
            params.insert(aSubFormula);
            return insertSubformula(boost::make_tuple(TF_BOX_E,params));
        }
        // No 'break' necessary, since the previous command is a return statement
    case TF_DIAMOND_D:
    {
        // To things a bit smarter: as we have two possible encoding (<B><E> and <E><B>), check if we already have
        // one present and use that one then.
        auto it = formulaNrs.find(boost::make_tuple(TF_DIAMOND_E,res));
        if (it==formulaNrs.end()) {
            // Let's do <E><B> then
            int aSubFormula = insertSubformula(boost::make_tuple(TF_DIAMOND_B,res));
            std::set<int> params;
            params.insert(aSubFormula);
            return insertSubformula(boost::make_tuple(TF_DIAMOND_E,params));
        } else {
            // Let's do <B><E> -- the latter is already present
            std::set<int> params;
            params.insert(it->second);
            return insertSubformula(boost::make_tuple(TF_DIAMOND_B,params));
        }
        // No 'break' necessary, since the previous command is a return statement
    }
    case TF_BOX_D:
    {
        // To things a bit smarter: as we have two possible encoding ([B][E] and [E][B]), check if we already have
        // one present and use that one then.
        auto it = formulaNrs.find(boost::make_tuple(TF_BOX_E,res));
        if (it==formulaNrs.end()) {
            // Let's do [E][B] then
            int aSubFormula = insertSubformula(boost::make_tuple(TF_BOX_B,res));
            std::set<int> params;
            params.insert(aSubFormula);
            return insertSubformula(boost::make_tuple(TF_BOX_E,params));
        } else {
            // Let's do [B][E] -- the latter is already present
            std::set<int> params;
            params.insert(it->second);
            return insertSubformula(boost::make_tuple(TF_BOX_B,params));
        }
        // No 'break' necessary, since the previous command is a return statement
    }
    case TF_DIAMOND_O_BAR:
        {
            int aSubFormula = insertSubformula(boost::make_tuple(TF_DIAMOND_E_BAR,res));
            std::set<int> params;
            params.insert(aSubFormula);
            return insertSubformula(boost::make_tuple(TF_DIAMOND_B,params));
        }
        // No 'break' necessary, since the previous command is a return statement
    case TF_BOX_O_BAR:
        {
            int aSubFormula = insertSubformula(boost::make_tuple(TF_BOX_E_BAR,res));
            std::set<int> params;
            params.insert(aSubFormula);
            return insertSubformula(boost::make_tuple(TF_BOX_B,params));
        }
        // No 'break' necessary, since the previous command is a return statement
    case TF_DIAMOND_L_BAR:
        {
            int aSubFormula = insertSubformula(boost::make_tuple(TF_DIAMOND_A_BAR,res));
            std::set<int> params;
            params.insert(aSubFormula);
            return insertSubformula(boost::make_tuple(TF_DIAMOND_A_BAR,params));
        }
        // No 'break' necessary, since the previous command is a return statement
    case TF_BOX_L_BAR:
        {
            int aSubFormula = insertSubformula(boost::make_tuple(TF_BOX_A_BAR,res));
            std::set<int> params;
            params.insert(aSubFormula);
            return insertSubformula(boost::make_tuple(TF_BOX_A_BAR,params));
        }
        // No 'break' necessary, since the previous command is a return statement
    case TF_DIAMOND_D_BAR:
    {
        // See above
        auto it = formulaNrs.find(boost::make_tuple(TF_DIAMOND_E_BAR,res));
        if (it==formulaNrs.end()) {
            int aSubFormula = insertSubformula(boost::make_tuple(TF_DIAMOND_B_BAR,res));
            std::set<int> params;
            params.insert(aSubFormula);
            return insertSubformula(boost::make_tuple(TF_DIAMOND_E_BAR,params));
        } else {
            std::set<int> params;
            params.insert(it->second);
            return insertSubformula(boost::make_tuple(TF_DIAMOND_B_BAR,params));
        }
        // No 'break' necessary, since the previous command is a return statement
    }
    case TF_BOX_D_BAR:
    {
        // See above
        auto it = formulaNrs.find(boost::make_tuple(TF_BOX_E_BAR,res));
        if (it==formulaNrs.end()) {
            int aSubFormula = insertSubformula(boost::make_tuple(TF_BOX_B_BAR,res));
            std::set<int> params;
            params.insert(aSubFormula);
            return insertSubformula(boost::make_tuple(TF_BOX_E_BAR,params));
        } else {
            std::set<int> params;
            params.insert(it->second);
            return insertSubformula(boost::make_tuple(TF_BOX_B_BAR,params));
        }
        // No 'break' necessary, since the previous command is a return statement
    }
    default:
        return insertSubformula(boost::make_tuple(formulas[startingFormula].get<0>(),res));
    }
}

/**
 * @brief Removes all subformulas that are unreachable from the given starting formula
 * @param startingFormula the only formula that is considered to be reachable by default
 * @return the new handle of the starting formula, which may change
 */
int FormulaFactory::removeUnreachableSubformulas(int startingFormula) {

    // Obtain list of reachable subformulas
    std::set<int> todo;
    std::set<int> reachable;
    todo.insert(startingFormula);
    reachable.insert(startingFormula);
    while (todo.size()>0) {
        int thisOne = *(todo.begin());
        todo.erase(*(todo.begin()));
        if (thisOne>=0) {
            std::set<int> currentOne = formulas[thisOne].get<1>();
            for (auto it = currentOne.begin();it!=currentOne.end();it++) {
                if (reachable.count(*it)==0) {
                    reachable.insert(*it);
                    todo.insert(*it);
                }
            }
        }
    }

    // Compute mapping
    std::map<int,int> mapping;
    for (auto it = reachable.begin();it!=reachable.end();it++) {
        if (*it>=0) {
            int newValue = mapping.size();
            mapping[*it] = newValue;
        }
    }

    // Rebuild formulas. Totally rebuild the "formulaNrs" map, and only shorten the vector
    std::map<boost::tuple<FormulaType,std::set<int> >,int,FormulaNrsComparator> formulaNrsNew;
    for (auto it = mapping.begin();it!=mapping.end();it++) {
        std::set<int> &old = formulas[it->first].get<1>();
        std::set<int> newOne;
        for (auto it2 = old.begin();it2!=old.end();it2++) {
            // Handle atomic propositions (those with a value < 0) differently
            if (*it2<0) {
                newOne.insert(*it2);
            } else {
                newOne.insert(mapping[*it2]);
            }
        }
        auto newTuple = boost::make_tuple(formulas[it->first].get<0>(),newOne);
        formulas[it->second] = newTuple;
        formulaNrsNew[newTuple] = it->second;
        //std::cout << "formulaNrsNew Add: "  << it->second << std::endl;
    }

    formulaNrs = formulaNrsNew;
    formulas.resize(mapping.size());

    if (startingFormula<0) return startingFormula;
    return mapping.at(startingFormula);

}

int FormulaFactory::insertSubformula(const boost::tuple<FormulaType,std::set<int> > &searchingFor) {
    std::map<boost::tuple<FormulaType,std::set<int> >,int>::iterator it = formulaNrs.find(searchingFor);
    if (it==formulaNrs.end()) {
        assert(formulaNrs.count(searchingFor)==0);
        int newNr = formulas.size();
        formulas.push_back(searchingFor);
        formulaNrs[searchingFor] = newNr;
        return newNr;
    } else {
        return it->second;
    }
}

int FormulaFactory::getOr(int a, int b) {
    std::set<int> res;
    res.insert(a);
    res.insert(b);
    boost::tuple<FormulaType,std::set<int> > searchingFor = boost::make_tuple(TF_OR,res);
    std::map<boost::tuple<FormulaType,std::set<int> >,int>::iterator it = formulaNrs.find(searchingFor);
    return insertSubformula(searchingFor);
}

int FormulaFactory::getAnd(int a, int b) {
    std::set<int> res;
    res.insert(a);
    res.insert(b);
    boost::tuple<FormulaType,std::set<int> > searchingFor = boost::make_tuple(TF_AND,res);
    return insertSubformula(searchingFor);
}

int FormulaFactory::getNot(int a) {
    std::set<int> res;
    res.insert(a);
    boost::tuple<FormulaType,std::set<int> > searchingFor = boost::make_tuple(TF_NOT,res);
    std::map<boost::tuple<FormulaType,std::set<int> >,int>::iterator it = formulaNrs.find(searchingFor);
    return insertSubformula(searchingFor);
}

int FormulaFactory::getDiamondTemporalFormula(std::string *op, int a) {
    std::set<int> res;
    FormulaType ft;
    if (*op=="a" || *op=="A") {
        ft = TF_DIAMOND_A;
    } else if (*op=="b" || *op=="B") {
        ft = TF_DIAMOND_B;
    } else if (*op=="e" || *op=="E") {
        ft = TF_DIAMOND_E;
    } else if (*op=="l" || *op=="L") {
        ft = TF_DIAMOND_L;
    } else if (*op=="d" || *op=="D") {
        ft = TF_DIAMOND_D;
    } else if (*op=="o" || *op=="O") {
        ft = TF_DIAMOND_O;
    } else if (*op=="a'" || *op=="A'") {
        ft = TF_DIAMOND_A_BAR;
    } else if (*op=="b'" || *op=="B'") {
        ft = TF_DIAMOND_B_BAR;
    } else if (*op=="e'" || *op=="E'") {
        ft = TF_DIAMOND_E_BAR;
    } else if (*op=="l'" || *op=="L'") {
        ft = TF_DIAMOND_L_BAR;
    } else if (*op=="d'" || *op=="D'") {
        ft = TF_DIAMOND_D_BAR;
    } else if (*op=="o'" || *op=="O'") {
        ft = TF_DIAMOND_O_BAR;
    } else {
        throw std::string("Did not understand the temporal operator <")+*op+">";
    }
    res.insert(a);
    boost::tuple<FormulaType,std::set<int> > searchingFor = boost::make_tuple(ft,res);
    return insertSubformula(searchingFor);
}

int FormulaFactory::getBoxTemporalFormula(std::string *op, int a) {
    std::set<int> res;
    FormulaType ft;
    if (*op=="a" || *op=="A") {
        ft = TF_BOX_A;
    } else if (*op=="b" || *op=="B") {
        ft = TF_BOX_B;
    } else if (*op=="e" || *op=="E") {
        ft = TF_BOX_E;
    } else if (*op=="o" || *op=="O") {
        ft = TF_BOX_O;
    } else if (*op=="l" || *op=="L") {
        ft = TF_BOX_L;
    } else if (*op=="d" || *op=="D") {
        ft = TF_BOX_D;
    } else if (*op=="a'" || *op=="A'") {
        ft = TF_BOX_A_BAR;
    } else if (*op=="b'" || *op=="B'") {
        ft = TF_BOX_B_BAR;
    } else if (*op=="e'" || *op=="E'") {
        ft = TF_BOX_E_BAR;
    } else if (*op=="o'" || *op=="O'") {
        ft = TF_BOX_O_BAR;
    } else if (*op=="l'" || *op=="L'") {
        ft = TF_BOX_L_BAR;
    } else if (*op=="d'" || *op=="D'") {
        ft = TF_BOX_D_BAR;
    } else if (*op=="e'" || *op=="E'") {
        ft = TF_BOX_E_BAR;
    } else {
        throw std::string("Did not understand the temporal operator [")+*op+"]";
    }
    res.insert(a);
    boost::tuple<FormulaType,std::set<int> > searchingFor = boost::make_tuple(ft,res);
    return insertSubformula(searchingFor);
}

int FormulaFactory::getPropositionalFormula(std::string *name) {
    std::map<std::string,int>::iterator finder = apNrs.find(*name);
    if (finder==apNrs.end()) {
        int apNumber = -aps.size()-1;
        aps.push_back(*name);
        apNrs[*name] = apNumber;
        return apNumber;
    }
    return finder->second;
}

int FormulaFactory::getSingleParameterOfTemporalSubformula(int number) {
    std::set<int> &params = formulas[number].get<1>();
    assert(params.size()==1);
    int result = *params.begin();
    return result;
}
