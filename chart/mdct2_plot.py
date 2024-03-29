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

if len(sys.argv) != 3:
	print "\tUsage: ./mdct_plot.py source_file output_file"
	exit(1)

csvFile = open(sys.argv[1], 'r')
csvReader = csv.reader(csvFile)

outputFile = sys.argv[2]

print "Using matplotlib version:", matplotlib.__version__

matplotlib.rcParams['font.family'] = 'Consolas'
matplotlib.rcParams['font.size'] = 12*zoom

x = np.arange(2048)
y = np.arange(-1, 12)
z = [100*i for i in y]
width = 0.1

coeff = map(float, csvReader.next())

#real = plt.bar(x, real, width, color='r')
#cmplxBars = plt.bar(x+width, cmplx, width, color='b')

plt.plot(x, coeff, 'k', antialiased=False)
plt.grid(True, 'major')
plt.xticks(z, z)
fig = plt.gcf()
defaultSize = fig.get_size_inches()
fig.set_figwidth(defaultSize[0]*zoom)
fig.set_figheight(defaultSize[1]*zoom)

fig.savefig(outputFile)
