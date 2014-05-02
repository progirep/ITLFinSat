ITLFinSat - Bounded Satisfiability Checker for Interval Temporal Logic
===================================================================

1. Introduction
---------------
ITLSat is a bounded Satisfiabiliy Checker for Interval Temporal logic, written by Ruediger Ehlers and Martin Lange. It interfaces with the picosat SAT solver by Armin Biere (http://fmv.jku.at/picosat/) for incremental SAT solving, which is included for your convenience.

2.1 Downloading and Compilation - Summary
-------------------------
On many Linux systems, ITLFinSat can be downloaded and built by the following sequence of commands:
...
...
...
...


2B. Compilation - Long version
-----------------------------

ITLFinSat comes in a form compilable under Linux. For other operating systems, it is typically possible to compile it by including the .c and .cpp files listed in the file "Project.pro" into a new project.

As a prerequisite, the GCC compiler chain, "qmake", "libboost", "bison", and "flex" must be installed. Under Ubuntu Linux, these can be installed by typing "sudo apt-get install build-essential qt4-qmake bison flex libboost-dev" on the command line.

ITLFinSat uses "picosat" as SAT solver. It can be downloaded from here: http://fmv.jku.at/picosat/ For a successful compilation of the ITLFinSat solver, version 957 must be placed in the folder picosat-957 in the directory in which ITLFinSat has been checked out. Later versions can be used, the file "src/Project.pro" must then be changed accordingly, however.

For compilation, run the following commands in the folder "src":

    qmake Project.pro
    make

Afterwards, if everything went smooth, an executable named "itlsc" (for ITL Satisfiability Checker) is produced. The compilation workflow is currently only tested on Ubuntu 12.04 (32 bit), but should work elsewhere as well. On systems with GCC < 4.7, it may be necessary to change the occurrence of "-std=c++11" in "src/Project.pro" to "-std=c++0x". 


3. Usage
--------
The tool "itlsc" reads the input formula from the command line and then performs a bounded satisfiability check. The input stream must be terminated before SAT checking starts. For example, after running "./itlsrc", one could type "[A]!p" and then press Enter and CRTL+D to close the input (under Linux). The resulting output would be:

    ==================[Satisfiability Checking Result]=================
    Formula size: 2
    Formula size after encoding: 3
    Trying a word of length 1
    Result: The temporal logic formula is satisfiable for a word of length 1
    We needed 12 SAT Variables and 7 clauses


    =====================[Satisfiability Certificate]==============================
    AP: p
    (0,0) 

The first few lines contain a bit of statistical information. Note that occurrences of atomic propositions do not count for the formula size. The important line is the line starting with "Result:", as it tells us that the formula has been found to be satisfiable. The satisfiability certificate lists all atomic propositions and the intervals on which they hold and thus describes an interval structure that is a model or the ITL formula. 

Note that for unsatisfiable formulas, "itlsc" diverges. To avoid this, the parameter "--max-bound [number]" can be added to the "itlsc" call. For example, running "itlsc --max-bound 10" on the formula "<A>p & [A]!p" yields:

    ==================[Satisfiability Checking Result]=================
    Formula size: 4
    Formula size after encoding: 6
    Trying a word of length 1
    Trying a word of length 2
    Trying a word of length 3
    Trying a word of length 4
    Trying a word of length 5
    Trying a word of length 6
    Trying a word of length 7
    Trying a word of length 8
    Trying a word of length 9
    Trying a word of length 10
    Result: Aborting due to reaching the maximum bound of 10

In ITL formulas, you can use all temporal operators of ITL ([A], [B], [E], [O], [L], [D], [A'], [B'], [E'], [O'], [L'], [D'], <A>, <B>, <E>, <O>, <D>, <L>, <A'>, <B'>, <E'>, <O'>, <D'>, <L'>), the negation operator "!", the disjunction operator "|", the conjunction operator "&", braces "(...)" and atomic proposition names. In terms of operator precedences, the unary operators bind strongest, then conjunction, and finally discjunction (as usual). The tool will issue a syntax error in case of illegal input. Line breaks and spaces are ignored, but mark the end of a part of the formula, so the input formula "[A](very long variable name)" is not an allowed input.

3. Examples and Tests
---------------------
The folder "examples" contains a couple of test instances ("simple...") and some more serious examples. The Python script "checkAll.py" can be used to test example specifications against the minimal length of their models, which is encoded as a number after the last "_" in the file name. For example, "simple_2_3.itl" is assumed to have a minimal model of length 3, and the "check_all.py" script will issue an error when this is not the case. The script sets the maximum bound to some number and treats all cases for which no model shorter than the maximum bound exists as unsatisfiable. The shortest lengths of the models for the simple examples have been determined by hand. It may be necessary to flag the script as executable before it can be used.

"itlsc" can be used on any specification file by running "../src/itlsc < filename.itl" from the "examples" directory. 

The specifications starting with "fischer" denote a benchmark on the Fischer mutual exclusion protocol. The value "n" given in the file name denotes the number of processes, and "c" describes how many processes need to be in the critical region at the same time in a model of the formula. In the system description paper, we always have n=c.

The specification "farmer.itl" represents the chicken crossing puzzle.

The specifications "setunset....itl" describe the binary counter from the system description paper. The smaller of these carry a "_" in their name, so that they are considered by the "check_all.py" script.

