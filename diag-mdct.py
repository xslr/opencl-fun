#!/usr/bin/python

import csv
import sys
import matplotlib

#matplotlib.use('cairo.pdf')

import matplotlib.pyplot as plt
from matplotlib.pylab import *
import numpy as np

zoom = 2.5

csvFile = open('rot-ml.csv', 'r')
csvReader = csv.reader(csvFile)
rot_ml = map(float, csvReader.next())

csvFile = open('rot-c.csv', 'r')
csvReader = csv.reader(csvFile)
rot_c = map(float, csvReader.next())

rot_diff = map((lambda x,y: x-y), rot_ml, rot_c)

csvFile = open('c-ml-r.csv', 'r')
csvReader = csv.reader(csvFile)
real = map(float, csvReader.next())

csvFile = open('c-ml-i.csv', 'r')
csvReader = csv.reader(csvFile)
imag = map(float, csvReader.next())

c_ml = map(complex, real, imag)

csvFile = open('w-ml-r.csv', 'r')
csvReader = csv.reader(csvFile)
real = map(float, csvReader.next())

csvFile = open('w-ml-i.csv', 'r')
csvReader = csv.reader(csvFile)
imag = map(float, csvReader.next())

w_ml = map(complex, real, imag)


csvFile = open('fkprefft.csv', 'r')
csvReader = csv.reader(csvFile)
real = map(float, csvReader.next())
imag = map(float, csvReader.next())

c_c = map(complex, real, imag)

csvFile = open('wprefft.csv', 'r')
csvReader = csv.reader(csvFile)
real = map(float, csvReader.next())
imag = map(float, csvReader.next())

w_c = map(complex, real, imag)

c_diff = map((lambda x,y: x-y), c_c, c_ml)
w_diff = map((lambda x,y: x-y), w_ml, w_c)
wdr = map((lambda x: x.real), w_diff)
wdi = map((lambda x: x.imag), w_diff)
cdr = map((lambda x: x.real), c_diff)
cdi = map((lambda x: x.imag), c_diff)

mul_ml = map((lambda x,y: x*y), c_ml, w_ml)
mul_c = map((lambda x,y: x*y), c_c, w_c)

def plot2(a, b):
	plt.subplot(211)
	plt.plot(a, 'b')
	plt.subplot(212)
	plt.plot(b, 'r')
	plt.show()
	return

def plot1(a):
	plt.plot(a)
	plt.show()
	return
