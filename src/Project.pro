TEMPLATE = app
TARGET = itlsc
DEPENDPATH += .
INCLUDEPATH += ../picosat-957
CONFIG = console release

QMAKE_CXXFLAGS += -Wall -std=c++11

# Input
FLEXSOURCES = lexer.flex
BISONSOURCES = parser.y
 
flex.commands = flex ${QMAKE_FILE_IN} && mv lex.yy.c lex.yy.cc
flex.input = FLEXSOURCES
flex.output = lex.yy.cc
flex.variable_out = SOURCES
flex.depends = y.tab.h
flex.name = flex
QMAKE_EXTRA_COMPILERS += flex
 
bison.commands = bison -d -t -y ${QMAKE_FILE_IN} && mv y.tab.c y.tab.cpp
bison.input = BISONSOURCES
bison.output = y.tab.cpp
bison.variable_out = SOURCES
bison.name = bison
QMAKE_EXTRA_COMPILERS += bison
 
bisonheader.commands = @true
bisonheader.input = BISONSOURCES
bisonheader.output = y.tab.h
bisonheader.variable_out = HEADERS
bisonheader.name = bison header
bisonheader.depends = y.tab.cpp
QMAKE_EXTRA_COMPILERS += bisonheader

HEADERS += y.tab.h formulaFactory.hpp satChecker.hpp ../picosat-957/picosat.h abstractSatisfiabilityChecker.hpp
SOURCES += main.cpp satChecker.cpp formulaFactory.cpp ../picosat-957/picosat.c abstractSatisfiabilityChecker.cpp
