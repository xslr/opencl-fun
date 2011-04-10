#!/usr/bin/python

import sys
import matplotlib
import array as ar

matplotlib.use('cairo.pdf')

import matplotlib.pyplot as plt
from matplotlib.pylab import *
import numpy as np

epsilon = 0

print "mdct_plot.py: MDCT output plotting script\n"

if len(sys.argv) < 3:
	print "\tUsage: ./mdct_plot.py source_file output_image"
	print "\tFor plotting difference in waveforms:\n\t  ./mdct_plot.py source_file_A source_file_B output_file"
	exit(1)

if len(sys.argv) == 3:
	sourceFile = sys.argv[1]
	outputFile = sys.argv[2]
	
	fp = open(sourceFile, 'rb')
	plotData = ar.array('f')
	plotData.fromfile(fp, 1024)
	
elif len(sys.argv) == 4:    # difference plotting
	sourceFileA = sys.argv[1]
	sourceFileB = sys.argv[2]
	outputFile = sys.argv[3]
	
	filea = open(sourceFileA, 'rb')
	fileb = open(sourceFileB, 'rb')

	coeffA = ar.array('f')
	coeffA.fromfile(filea, 1024)

	coeffB = ar.array('f')
	coeffB.fromfile(fileb, 1024)

	plotData = map((lambda x,y: (x-y)/y if abs(y) > epsilon else 0), coeffA, coeffB)

print "Using matplotlib version:", matplotlib.__version__

matplotlib.rcParams['font.family'] = 'Garamond Premier Pro'
matplotlib.rcParams['font.size'] = 14

x = np.arange(1024)
y = np.arange(0, 8)
z = [150*i for i in y]
width = 0.1

plt.plot(x, plotData, 'k', linewidth=0.3, antialiased=False)
plt.grid(True, 'major')
plt.xticks(z, z)

plt.ylabel('coefficient value')
plt.xlabel('coefficient index')

fig = plt.gcf()

fig_width_pt = 455.24411  # Get this from LaTeX using \showthe\columnwidth
inches_per_pt = 1.0/72.27               # Convert pt to inch
golden_mean = (sqrt(5)-1.0)/2.0         # Aesthetic ratio
fig_width = fig_width_pt*inches_per_pt  # width in inches
fig_height = fig_width*golden_mean      # height in inches
fig_size =  [fig_width,fig_height]

fig.savefig(outputFile)
