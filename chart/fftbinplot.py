#!/usr/bin/python

import sys
import matplotlib
import array as ar

matplotlib.use('cairo.pdf')

import matplotlib.pyplot as plt
from matplotlib.pylab import *
import numpy as np

zoom = 2
epsilon = 0.00001

print "fft_plot.py: FFT output plotting script\n"

if len(sys.argv) < 3:
	print "\tUsage: ./fft_plot.py source_file output_file"
	print "\tFor plotting difference:\n\t  ./fft_plot.py source_file_A source_file_B output_file"
	exit(1)

if len(sys.argv) == 3:
	sourceFile = sys.argv[1]
	outputFile = sys.argv[2]
	
	fp = open(sourceFile, 'rb')

	plotData_r = ar.array('f')
	plotData_i = ar.array('f')

	plotData_r.fromfile(fp, 2048)
	plotData_i.fromfile(fp, 2048)
	
elif len(sys.argv) == 4:   # difference plotting
	sourceFileA = sys.argv[1]
	sourceFileB = sys.argv[2]
	outputFile = sys.argv[3]
	
	filea = open(sourceFileA, 'rb')
	fileb = open(sourceFileB, 'rb')

	A_r = ar.array('f')
	A_i = ar.array('f')

	A_r.fromfile(filea, 2048)
	A_i.fromfile(filea, 2048)

	B_r = ar.array('f')
	B_i = ar.array('f')

	B_r.fromfile(fileb, 2048)
	B_i.fromfile(fileb, 2048)

	plotData_r = map((lambda x,y: (x-y)/y if y != 0 else 0), A_r, B_r)
	plotData_i = map((lambda x,y: (x-y)/y if y != 0 else 0), A_i, B_i)



print "Using matplotlib version:", matplotlib.__version__

matplotlib.rcParams['font.family'] = 'Garamond Premier Pro'
matplotlib.rcParams['font.size'] = 15*zoom

plt.axis('tight')

x = np.arange(2048)
y = np.arange(0, 13)
z = [200*i for i in y]
width=0.1

realPlot = subplot(211)
realPlot.plot(x, plotData_r, 'k', antialiased=False)
realPlot.grid(True, 'major')
plt.xticks(z, z)
#title('')
ylabel('Real ')

cmplxPlot = subplot(212)
cmplxPlot.plot(x, plotData_i, 'k', antialiased=True)
cmplxPlot.grid(True, 'major')
plt.xticks(z, z)
ylabel('Complex')
xlabel('index')

fig = plt.gcf()
defaultSize = fig.get_size_inches()
fig.set_figwidth(defaultSize[0]*zoom)
fig.set_figheight(defaultSize[1]*zoom)

fig.savefig(outputFile)
