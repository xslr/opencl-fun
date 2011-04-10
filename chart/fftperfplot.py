#!/usr/bin/python
# -*- coding: utf-8 -*-

import csv
import sys
import matplotlib

matplotlib.use('cairo.png')

import matplotlib.pyplot as plt
from matplotlib.pylab import *
import numpy as np

print "fftperfplot.py: FFT performance plotting\n"

if len(sys.argv) != 2:
	print "\tUsage: ./fftperfplot.py perfdata"
	exit(1)

sourceFile = sys.argv[1]
outputFileTotal = "plot/fft_perf_total.pdf"
outputFileExec = "plot/fft_perf_exec.pdf"
outputFileGPU = "plot/fft_perf_gpu.pdf"
	
csvFile = open(sourceFile, 'r')
csvReader = csv.reader(csvFile)
count, gpu_total, gpu_ul, gpu_exec, gpu_dl, fftw_total, fftw_plan, fftw_exec = zip(*csvReader)

# convert microsecond timings to milliseconds
gpu_total = [float(i)/1000 for i in gpu_total]
gpu_exec = [float(i)/1000 for i in gpu_exec]
gpu_ul = [float(i)/1000 for i in gpu_ul]
gpu_dl = [float(i)/1000 for i in gpu_dl]
fftw_total = [float(i)/1000 for i in fftw_total]
fftw_plan = [float(i)/1000 for i in fftw_plan]
fftw_exec = [float(i)/1000 for i in fftw_exec]

print "Using matplotlib version:", matplotlib.__version__

rc('text', usetex=False)
rc('font', family='serif')
rc('legend', fontsize=12)
rc('font', family='Garamond Premier Pro')
rc('font', size  = 12)

#### Total Graph ####
fig = plt.figure()

x = np.arange(100, 40100, 100)
y = np.arange(0, 11)
z = [4000*i for i in y]

ax = fig.add_subplot(1, 1, 1)

ax.plot(x, gpu_total, 'r', linewidth=0.5, label='Total GPU time')
ax.plot(x, fftw_total, 'b', linewidth=0.5, label='Total FFTW time')
ax.grid(True, 'major')
plt.xlabel('number of blocks')
plt.ylabel('time (ms)')

ax.legend(bbox_to_anchor=(0, 0, 0, 1), loc='upper left', ncol=2)

defaultSize = fig.get_size_inches()

fig_width_pt = 455.24411  # Get this from LaTeX using \showthe\columnwidth
inches_per_pt = 1.0/72.27               # Convert pt to inch
golden_mean = (sqrt(5)-1.0)/2.0         # Aesthetic ratio
fig_width = fig_width_pt*inches_per_pt  # width in inches
fig_height = fig_width*golden_mean      # height in inches
fig_size =  [fig_width,fig_height]

fig.savefig(outputFileTotal)

#### GPU BreakDown ####
fig = plt.figure()

ax = fig.add_subplot(1, 1, 1)

ax.plot(x, gpu_ul, 'b', linewidth=0.5, label='GPU upload time')
ax.plot(x, gpu_exec, 'r', linewidth=0.5, label='GPU execution time')
ax.plot(x, gpu_dl, 'g', linewidth=0.5, label='GPU download time')
ax.grid(True, 'major')
plt.xlabel('number of blocks')
plt.ylabel('time (ms)')

ax.legend(bbox_to_anchor=(0, 0, 0, 1), loc='upper left', ncol=2)

fig_width_pt = 455.24411  # Get this from LaTeX using \showthe\columnwidth
inches_per_pt = 1.0/72.27               # Convert pt to inch
golden_mean = (sqrt(5)-1.0)/2.0         # Aesthetic ratio
fig_width = fig_width_pt*inches_per_pt  # width in inches
fig_height = fig_width*golden_mean      # height in inches
fig_size =  [fig_width,fig_height]

fig.savefig(outputFileGPU)

#### Execution Times ####
fig = plt.figure()

ax = fig.add_subplot(1, 1, 1)

ax.plot(x, gpu_exec, 'r', linewidth=0.5, label='GPU execution time')
ax.plot(x, fftw_exec, 'b', linewidth=0.5, label='FFTW execution time')
ax.grid(True, 'major')
plt.xlabel('number of blocks')
plt.ylabel('time (ms)')

ax.legend(bbox_to_anchor=(0, 0, 0, 1), loc='upper left', ncol=2)
fig_width_pt = 455.24411  # Get this from LaTeX using \showthe\columnwidth
inches_per_pt = 1.0/72.27               # Convert pt to inch
golden_mean = (sqrt(5)-1.0)/2.0         # Aesthetic ratio
fig_width = fig_width_pt*inches_per_pt  # width in inches
fig_height = fig_width*golden_mean      # height in inches
fig_size =  [fig_width,fig_height]

fig.savefig(outputFileExec)

