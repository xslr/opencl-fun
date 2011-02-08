#!/usr/bin/python

import array as ar
import sys

print "mdctbinstats.py: MDCT error statistics calculation\n"

if len(sys.argv) != 3:
	print "\tUsage: ./mdct_plot.py file_A file_B"
	exit(1)

sourceFileA = sys.argv[1]
sourceFileB = sys.argv[2]

print "\nCalculating accuracy stats for \"" + sourceFileA + "\" and \"" + sourceFileB + "\"."

filea = open(sourceFileA, 'rb')
fileb = open(sourceFileB, 'rb')

coeffA = ar.array('f')
coeffA.fromfile(filea, 1024)

coeffB = ar.array('f')
coeffB.fromfile(fileb, 1024)

diff = map((lambda x,y: x-y), coeffA, coeffB)
diffSQ = map((lambda x: x*x), diff)

sumDiff = sum(diff)
sumDiffAbs = reduce((lambda x,y: abs(x)+abs(y)), diff)
meanSQdiff = sum(diffSQ)

print "Sum of errors:" + str(sumDiff)
print "Sum of absolute values of errors:" + str(sumDiffAbs)
print "Mean square error:" + str(meanSQdiff)
