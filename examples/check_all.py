#!/usr/bin/python
#
# Tests the interval temporal logic satisfiability checker on all .ilp files in the current directory. 

import os, subprocess
import sys, time

# os.system("killall kpdf")
allfiles = os.popen("ls *.itl")

for line in allfiles:
    p = subprocess.Popen(["../src/itlsc","--max-bound","17"], bufsize=1048768, stdin=subprocess.PIPE, stderr=subprocess.STDOUT, stdout=subprocess.PIPE)
    line = line.strip()
    print >>sys.stderr, line+":",
    if line.find("_")>=0:    
        with open(line) as inFile:
            for e in inFile:
                p.stdin.write(e)
        p.stdin.close()
        satLength = None
        for a in p.stdout:
            if a.startswith("Result: The temporal logic formula is satisfiable for a word of length "):
                satLength = a[71:].strip()
            if a.startswith("Result: Aborting due to reaching the maximum bound"):
                satLength = "unsat"
        p.stdout.close()    
        if satLength==None:
            print >>sys.stderr, "Error: Could not find satisfiability result when running 'itlsc' for input file "+line
            sys.exit(1)
        lineparts = line[0:len(line)-4].split("_")
        if lineparts[len(lineparts)-1]!=satLength:
            print >>sys.stderr, "Error: Expected shortest satisfied word of length "+lineparts[len(lineparts)-1]+" but found length "+satLength+" instead."
            print >>sys.stderr, "Input formula: "+line
            sys.exit(1)
        print >>sys.stderr, "OK"
    else:
        print >>sys.stderr, "Skipped"
