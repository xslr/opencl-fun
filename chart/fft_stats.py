#!/usr/bin/python

import csv
import sys

print "fft_stat.py: FFT error stats script\n"

if len(sys.argv) != 3:
	print "\tUsage: ./fft_plot.py file_A file_B"
	exit(1)

sourceFileA = sys.argv[1]
sourceFileB = sys.argv[2]

print "\nCalculating accuracy stats for \"" + sourceFileA + "\" and \"" + sourceFileB + "\"."

csvFileA = open(sourceFileA, 'r')
csvFileB = open(sourceFileB, 'r')

csvReader = csv.reader(csvFileA)
A_r = map(float, csvReader.next())
A_i = map(float, csvReader.next())

csvReader = csv.reader(csvFileB)
B_r = map(float, csvReader.next())
B_i = map(float, csvReader.next())

diff_r = map((lambda x,y: x-y), A_r, B_r)
diff_i = map((lambda x,y: x-y), A_i, B_i)

diffSQ_r = map((lambda x: x*x), diff_r)
diffSQ_i = map((lambda x: x*x), diff_i)

sumDiff_r = sum(diff_r)
sumDiff_i = sum(diff_i)

sumDiffAbs_r = reduce((lambda x,y: abs(x)+abs(y)), diff_r)
sumDiffAbs_i = reduce((lambda x,y: abs(x)+abs(y)), diff_i)

meanSQdiff_r = sum(diffSQ_r)
meanSQdiff_i = sum(diffSQ_i)

print "Sum of errors:" + str(sumDiff_r + sumDiff_i)
print "Sum of absolute values of errors:" + str(sumDiffAbs_r + sumDiffAbs_i)
print "Mean square error:" + str(meanSQdiff_r + meanSQdiff_i)
