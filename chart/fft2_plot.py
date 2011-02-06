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

if len(sys.argv) != 3:
	print "\tUsage: ./fft_plot.py source_file output_file"
	exit(1)

csvFile = open(sys.argv[1], 'r')
csvReader = csv.reader(csvFile)

outputFile = sys.argv[2]

print "Using matplotlib version:", matplotlib.__version__

matplotlib.rcParams['font.family'] = 'Consolas'
matplotlib.rcParams['font.size'] = 12*zoom

x = np.arange(512)
y = np.arange(-1, 12)
z = [50*i for i in y]
width=0.1

real = map(float, csvReader.next())
cmplx = map(float, csvReader.next())

#real = plt.bar(x, real, width, color='r')
#cmplxBars = plt.bar(x+width, cmplx, width, color='b')

realPlot = subplot(211)
realPlot.plot(x, real, 'k', antialiased=False)
realPlot.grid(True, 'major')
plt.xticks(z, z)

cmplxPlot = subplot(212)
cmplxPlot.plot(x, cmplx, 'k--', antialiased=True)
cmplxPlot.grid(True, 'major')
plt.xticks(z, z)

fig = plt.gcf()
defaultSize = fig.get_size_inches()
fig.set_figwidth(defaultSize[0]*zoom)
fig.set_figheight(defaultSize[1]*zoom)

fig.savefig(outputFile)
