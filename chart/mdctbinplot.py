#!/usr/bin/python

import sys
import matplotlib
import array as ar

matplotlib.use('cairo.pdf')

import matplotlib.pyplot as plt
from matplotlib.pylab import *
import numpy as np

zoom = 3
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
matplotlib.rcParams['font.size'] = 15*zoom

#rc('text', usetex=True)
#rc('font', family='serif')

x = np.arange(1024)
y = np.arange(0, 8)
z = [150*i for i in y]
width = 0.1

plt.plot(x, plotData, 'k', antialiased=False)
plt.grid(True, 'major')
plt.xticks(z, z)

fig = plt.gcf()
defaultSize = fig.get_size_inches()
fig.set_figwidth(defaultSize[0]*zoom)
fig.set_figheight(defaultSize[1]*zoom)

fig.savefig(outputFile)
