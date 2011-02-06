#!/usr/bin/python

import csv
import sys
import matplotlib

matplotlib.use('cairo.pdf')

import matplotlib.pyplot as plt
from matplotlib.pylab import *
import numpy as np

zoom = 2.5

print "fft_plot.py: FFT output plotting script\n"

if len(sys.argv) < 3:
	print "\tUsage: ./fft_plot.py source_file output_file"
	print "\tFor plotting difference:\n\t  ./fft_plot.py source_file_A source_file_B output_file"
	exit(1)

if len(sys.argv) == 3:
	sourceFile = sys.argv[1]
	outputFile = sys.argv[2]
	
	csvFile = open(sourceFile, 'r')
	csvReader = csv.reader(csvFile)
	plotData_r = map(float, csvReader.next())
	plotData_c = map(float, csvReader.next())
	
elif len(sys.argv) == 4:   # difference plotting
	sourceFileA = sys.argv[1]
	sourceFileB = sys.argv[2]
	outputFile = sys.argv[3]
	
	csvFileA = open(sourceFileA, 'r')
	csvFileB = open(sourceFileB, 'r')

	csvReader = csv.reader(csvFileA)
	A_r = map(float, csvReader.next())
	A_i = map(float, csvReader.next())

	csvReader = csv.reader(csvFileB)
	B_r = map(float, csvReader.next())
	B_i = map(float, csvReader.next())

	plotData_r = map((lambda x,y: (x-y)/y if y != 0 else 0), A_r, B_r)
	plotData_c = map((lambda x,y: (x-y)/y if y != 0 else 0), A_i, B_i)


print "Using matplotlib version:", matplotlib.__version__

matplotlib.rcParams['font.family'] = 'Consolas'
matplotlib.rcParams['font.size'] = 12*zoom

x = np.arange(2048)
y = np.arange(-1, 12)
z = [200*i for i in y]
width=0.1

realPlot = subplot(211)
realPlot.plot(x, plotData_r, 'k', antialiased=False)
realPlot.grid(True, 'major')
plt.xticks(z, z)

cmplxPlot = subplot(212)
cmplxPlot.plot(x, plotData_c, 'k--', antialiased=True)
cmplxPlot.grid(True, 'major')
plt.xticks(z, z)

fig = plt.gcf()
defaultSize = fig.get_size_inches()
fig.set_figwidth(defaultSize[0]*zoom)
fig.set_figheight(defaultSize[1]*zoom)

fig.savefig(outputFile)
