// Subformula class
#ifndef __SUBFORMULA_HPP___
#define __SUBFORMULA_HPP___

#include <string>
#include <vector>
#include <map>
#include <set>
#include <algorithm>
#include <iostream>
#include <boost/smart_ptr.hpp>
#include <boost/tuple/tuple.hpp>
#include <boost/tuple/tuple_comparison.hpp>

/**
  * @brief The possible types of subformulas in a temporal formula.
  */
#define FIRST_BOX_FORMULA_TYPE 100
typedef enum { TF_AND, TF_OR, TF_NOT, TF_DIAMOND_A, TF_DIAMOND_B, TF_DIAMOND_E,
               TF_DIAMOND_A_BAR, TF_DIAMOND_B_BAR, TF_DIAMOND_E_BAR, TF_DIAMOND_L,
               TF_DIAMOND_D, TF_DIAMOND_O, TF_DIAMOND_L_BAR, TF_DIAMOND_D_BAR,
               TF_DIAMOND_O_BAR, TF_BOX_A=FIRST_BOX_FORMULA_TYPE, TF_BOX_B, TF_BOX_E, TF_BOX_A_BAR,
               TF_BOX_B_BAR, TF_BOX_E_BAR, TF_BOX_L, TF_BOX_D, TF_BOX_O,
               TF_BOX_L_BAR, TF_BOX_D_BAR, TF_BOX_O_BAR} FormulaType;
std::ostream& operator<<(std::ostream& lhs, FormulaType e);


/**
 * @brief FormulaFactory class that represents formulas as integers. Atomic propositions have negative numbers, all numbers 0 and
 * above represent proper formulas.
 */
class FormulaFactory {
private:
    // Two-way lookup structures for APs
    std::vector<std::string> aps;
    std::map<std::string,int> apNrs;

    // Two-way lookup structures for formulas
    class FormulaNrsComparator {
    public:
        bool operator()(const boost::tuple<FormulaType,std::set<int> >& left, const boost::tuple<FormulaType,std::set<int> >& right) const {
            if (left.get<0>() < right.get<0>()) return true;
            if (left.get<0>() > right.get<0>()) return false;
            return std::lexicographical_compare(left.get<1>().begin(),left.get<1>().end(),right.get<1>().begin(),right.get<1>().end());
        }
    };

    std::vector<boost::tuple<FormulaType,std::set<int> > > formulas;
    std::map<boost::tuple<FormulaType,std::set<int> >,int,FormulaNrsComparator> formulaNrs;

public:
    FormulaFactory() {}
    int insertSubformula(const boost::tuple<FormulaType,std::set<int> > &searchingFor);
    int getOr(int a, int b);
    int getAnd(int a, int b);
    int getNot(int a);
    int getDiamondTemporalFormula(std::string *type, int subformula);
    int getBoxTemporalFormula(std::string *type, int subformula);
    int getPropositionalFormula(std::string *name);
    void printFormula(int nr, int level=0);
    int getSingleParameterOfTemporalSubformula(int number);
    int encodeDerivedTemporalOperators(int startingFormula);
    int removeUnreachableSubformulas(int startingFormula);
    int makePositiveNormalForm(int startingFormula, bool negated = false);
    void printFormulaTable();

    const std::map<std::string,int> &getAPNrs() { return apNrs; }
    const std::map<boost::tuple<FormulaType,std::set<int> >,int,FormulaNrsComparator> &getFormulaNrs() { return formulaNrs; }
    const std::vector<boost::tuple<FormulaType,std::set<int> > > &getFormulas() { return formulas; }
};

extern FormulaFactory formulaFactory;

#endif

