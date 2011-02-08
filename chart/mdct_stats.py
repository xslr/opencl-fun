#!/usr/bin/python

import csv
import sys

print "mdct_plot.py: MDCT error statistics calculation\n"

if len(sys.argv) != 3:
	print "\tUsage: ./mdct_plot.py file_A file_B"
	exit(1)

sourceFileA = sys.argv[1]
sourceFileB = sys.argv[2]

print "\nCalculating accuracy stats for \"" + sourceFileA + "\" and \"" + sourceFileB + "\"."

csvFileA = open(sourceFileA, 'r')
csvFileB = open(sourceFileB, 'r')

csvReader = csv.reader(csvFileA)
coeffA = map(float, csvReader.next())

csvReader = csv.reader(csvFileB)
coeffB = map(float, csvReader.next())

diff = map((lambda x,y: x-y), coeffA, coeffB)
diffSQ = map((lambda x: x*x), diff)

sumDiff = sum(diff)
sumDiffAbs = reduce((lambda x,y: abs(x)+abs(y)), diff)
meanSQdiff = sum(diffSQ)

print "Sum of errors:" + str(sumDiff)
print "Sum of absolute values of errors:" + str(sumDiffAbs)
print "Mean square error:" + str(meanSQdiff)
