#!/usr/bin/python

import csv
import sys
import matplotlib

matplotlib.use('cairo.pdf')

import matplotlib.pyplot as plt
from matplotlib.pylab import *
import numpy as np

zoom = 2

print "mdctperfplot.py: MDCT performance plotting script\n"

if len(sys.argv) != 2:
	print "\tUsage: ./mdctperfplot.py perfdata"
	exit(1)

sourceFile = sys.argv[1]
outputFileTotal = "plot/mdct_perf_total.pdf"
outputFileExec = "plot/mdct_perf_exec.pdf"
outputFileGPU = "plot/mdct_perf_gpu.pdf"
	
csvFile = open(sourceFile, 'r')
csvReader = csv.reader(csvFile)
count, gpu_total, gpu_ul, gpu_exec, gpu_dl, vorbis_total, vorbis_init, vorbis_exec, brute = zip(*csvReader)

# convert microsecond timings to milliseconds
gpu_total = [float(i)/1000 for i in gpu_total]
gpu_exec = [float(i)/1000 for i in gpu_exec]
gpu_ul = [float(i)/1000 for i in gpu_ul]
gpu_dl = [float(i)/1000 for i in gpu_dl]
vorbis_total = [float(i)/1000 for i in vorbis_total]
vorbis_init = [float(i)/1000 for i in vorbis_init]
vorbis_exec = [float(i)/1000 for i in vorbis_exec]

print "Using matplotlib version:", matplotlib.__version__

rc('text', usetex=False)
rc('font', family='serif')
rc('legend', fontsize=12)
rc('font', family='Garamond Premier Pro')
rc('font', size  = 12)

#### Total graph ####
fig = plt.figure()

x = np.arange(100, 100100, 100)
y = np.arange(0, 11)
z = [4000*i for i in y]

ax = fig.add_subplot(1, 1, 1)

ax.plot(x, gpu_total, 'r', linewidth=0.5, label='Total GPU time')
ax.plot(x, vorbis_total, 'b', linewidth=0.5, label='Total Vorbis time')
ax.grid(True, 'major')
plt.xlabel('number of blocks')
plt.ylabel(u'time (ms)')

ax.legend(bbox_to_anchor=(0, 0, 0, 1), loc='upper left', ncol=2)

fig_width_pt = 455.24411  # Get this from LaTeX using \showthe\columnwidth
inches_per_pt = 1.0/72.27               # Convert pt to inch
golden_mean = (sqrt(5)-1.0)/2.0         # Aesthetic ratio
fig_width = fig_width_pt*inches_per_pt  # width in inches
fig_height = fig_width*golden_mean      # height in inches
fig_size =  [fig_width,fig_height]

fig.savefig(outputFileTotal)

#### GPU timing breakdown ####
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

#### Execution times ####
fig = plt.figure()

ax = fig.add_subplot(1, 1, 1)

ax.plot(x, gpu_exec, 'r', linewidth=0.5, label='GPU execution time')
ax.plot(x, vorbis_exec, 'b', linewidth=0.5, label='Vorbis execution time')
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
