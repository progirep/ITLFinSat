#ifndef __ABSTRACT_SATISFIABILITY_CHECKER_HPP__
#define __ABSTRACT_SATISFIABILITY_CHECKER_HPP__

#include <map>
#include <boost/tuple/tuple.hpp>
#include <boost/tuple/tuple_comparison.hpp>


/**
  * Three-Valued abstraction class
  */
class ThreeValueBool {
private:
    int value;
    static const int threeValueBoolCombinationTableAND[];
    static const int threeValueBoolCombinationTableOR[];
    static const int threeValueBoolCombinationTableNOT[];
public:
    ThreeValueBool(int _internalValue) : value(_internalValue) {}
    ThreeValueBool() : value(0) {} // Uninitialised
    ThreeValueBool operator&(const ThreeValueBool &other) const {
        return ThreeValueBool(threeValueBoolCombinationTableAND[value*4+other.value]);
    }
    ThreeValueBool operator|(const ThreeValueBool &other) const {
        return ThreeValueBool(threeValueBoolCombinationTableOR[value*4+other.value]);
    }
    ThreeValueBool& operator|=(const ThreeValueBool &other) {
        value = threeValueBoolCombinationTableOR[value*4+other.value];
        return *this;
    }
    ThreeValueBool operator&=(const ThreeValueBool &other) {
        value = threeValueBoolCombinationTableAND[value*4+other.value];
        return *this;
    }
    ThreeValueBool& operator=(const ThreeValueBool &other) {
        value = other.value;
        return *this;
    }
    ThreeValueBool operator!() const {
        return threeValueBoolCombinationTableNOT[value];
    }
    bool operator==(const ThreeValueBool &other) const {
        return value==other.value;
    }
    bool operator!=(const ThreeValueBool &other) const {
        return value!=other.value;
    }

    // Constant Values
    static ThreeValueBool FALSE;
    static ThreeValueBool TRUE;
    static ThreeValueBool X;
    static ThreeValueBool UNINITIALISED;
};




/**
 * @brief This class is used for preprocessing: for the given ITL formula in positive normal
 * form, we test if even replacing all atomic propositions and their negation by TRUE makes
 * a subformula satisfiabile on some interval and if it is reachable. In all other cases, we
 * can just set the corresponding atomic proposition in the SAT encoding to FALSE.
 */
class AbstractSatisfiabilityChecker {
    std::map<boost::tuple<int,int,int>,ThreeValueBool> doneList;

    ThreeValueBool recurse(int subformula, int from, int to);
    int wordLength;
public:
    AbstractSatisfiabilityChecker(int mainFormula, int _wordLength) : wordLength(_wordLength) { recurse(mainFormula,0,0); }
    const std::map<boost::tuple<int,int,int>,ThreeValueBool> &getStorage() const { return doneList; }

    class NonXResultIterator {
        std::map<boost::tuple<int,int,int>,ThreeValueBool> &reference;
        std::map<boost::tuple<int,int,int>,ThreeValueBool>::const_iterator it;
        bool readCurrent;
    public:
        void searchNext() {
            readCurrent = false;
            while ((it!=reference.end()) && (it->second!=ThreeValueBool::FALSE) && (it->second!=ThreeValueBool::TRUE)) it++;
        }
        NonXResultIterator(std::map<boost::tuple<int,int,int>,ThreeValueBool> &doneList) : reference(doneList), it(doneList.begin()){
            searchNext();
        }
        bool hasNext() {
            return it!=reference.end();
        }
        bool isTrue() {
            return it->second==ThreeValueBool::TRUE;
        }
        bool isFalse() {
            return it->second==ThreeValueBool::FALSE;
        }
        int getSubformula() {
            return it->first.get<0>();
        }
        int getFrom() {
            return it->first.get<1>();
        }
        int getTo() {
            return it->first.get<2>();
        }
    };
    NonXResultIterator getNonXResultIterator() { return NonXResultIterator(doneList); }





};





#endif
