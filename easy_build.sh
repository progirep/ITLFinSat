cd src || exit
bison -d -t -y parser.y && mv y.tab.c y.tab.cpp
gcc -fPIE -O -I../picosat-957 ../picosat-957/picosat.c -c -o picosat.o
g++ -O -std=c++11 -I../picosat-957 picosat.o abstractSatisfiabilityChecker.cpp formulaFactory.cpp satChecker.cpp main.cpp y.tab.cpp lex.yy.cc -o itlsc
