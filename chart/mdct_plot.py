#!/usr/bin/python

import csv
import sys
import matplotlib

matplotlib.use('cairo.pdf')

import matplotlib.pyplot as plt
from matplotlib.pylab import *
import numpy as np

zoom = 2.5

print "mdct_plot.py: MDCT output plotting script\n"

if len(sys.argv) < 3:
	print "\tUsage: ./mdct_plot.py source_file output_image"
	print "\tFor plotting difference in waveforms:\n\t  ./mdct_plot.py source_file_A source_file_B output_file"
	exit(1)

if len(sys.argv) == 3:
	sourceFile = sys.argv[1]
	outputFile = sys.argv[2]
	
	csvFile = open(sourceFile, 'r')
	csvReader = csv.reader(csvFile)
	plotData = map(float, csvReader.next())
	
elif len(sys.argv) == 4:    # difference plotting
	sourceFileA = sys.argv[1]
	sourceFileB = sys.argv[2]
	outputFile = sys.argv[3]
	
	csvFileA = open(sourceFileA, 'r')
	csvFileB = open(sourceFileB, 'r')

	csvReader = csv.reader(csvFileA)
	coeffA = map(float, csvReader.next())

	csvReader = csv.reader(csvFileB)
	coeffB = map(float, csvReader.next())

	plotData = map((lambda x,y: (x-y)/y if y != 0 else 0), coeffA, coeffB)

print "Using matplotlib version:", matplotlib.__version__

matplotlib.rcParams['font.family'] = 'Consolas'
matplotlib.rcParams['font.size'] = 12*zoom

x = np.arange(1024)
y = np.arange(-1, 12)
z = [100*i for i in y]
width = 0.1

plt.plot(x, plotData, 'k', antialiased=False)
plt.grid(True, 'major')
plt.xticks(z, z)

fig = plt.gcf()
defaultSize = fig.get_size_inches()
fig.set_figwidth(defaultSize[0]*zoom)
fig.set_figheight(defaultSize[1]*zoom)

fig.savefig(outputFile)
