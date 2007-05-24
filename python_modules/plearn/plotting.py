# plotting.py
# Copyright (C) 2005 Pascal Vincent
#
#  Redistribution and use in source and binary forms, with or without
#  modification, are permitted provided that the following conditions are met:
#
#   1. Redistributions of source code must retain the above copyright
#      notice, this list of conditions and the following disclaimer.
#
#   2. Redistributions in binary form must reproduce the above copyright
#      notice, this list of conditions and the following disclaimer in the
#      documentation and/or other materials provided with the distribution.
#
#   3. The name of the authors may not be used to endorse or promote
#      products derived from this software without specific prior written
#      permission.
#
#  THIS SOFTWARE IS PROVIDED BY THE AUTHORS ``AS IS'' AND ANY EXPRESS OR
#  IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
#  OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN
#  NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
#  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
#  TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
#  PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
#  LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
#  NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
#  SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#
#  This file is part of the PLearn library. For more information on the PLearn
#  library, go to the PLearn Web site at www.plearn.org


# Author: Pascal Vincent

# from array import *
import numarray

import string
import matplotlib
# matplotlib.interactive(True)
#matplotlib.use('TkAgg')
#matplotlib.use('GTK')
  
from pylab import *
from numarray import *
from mayavi.tools import imv

from plearn.vmat.PMat import *


threshold = 0

def margin(scorevec):
    if len(scorevec)==1:
        return abs(scorevec[0]-threshold)
    else:
        sscores = sort(scorevec)    
        return sscores[-1]-sscores[-2]

def winner(scorevec):
    if len(scorevec)==1:
        if scorevec[0]>threshold:
            return 1
        else:
            return 0
    else:
        return argmax(scorevec)

def xyscores_to_winner_and_magnitude(xyscores):
    return array([ (v[0], v[1], winner(v[2:]),max(v[2:])) for v in xyscores ])

def xyscores_to_winner_and_margin(xyscores):
    return array([ (v[0], v[1], winner(v[2:]),margin(v[2:])) for v in xyscores ])

def regular_xyval_to_2d_grid_values(xyval):
    """Returns (grid_values, x0, y0, deltax, deltay)"""
    xyval = numarray.array(xyval)
    n = len(xyval)
    x = xyval[:,0]
    y = xyval[:,1]
    values = xyval[:,2:].copy()
    # print "type(values)",type(values)
    valsize = numarray.size(values,1)
    x0 = x[0]
    y0 = y[0]

    k = 1
    if x[1]==x0:
        deltay = y[1]-y[0]
        while x[k]==x0:
            k = k+1
        deltax = x[k]-x0
        ny = k
        nx = n // ny
        # print 'A) nx,ny:',nx,ny
        values.shape = (nx,ny,valsize)
        # print "A type(values)",type(values)
        values = numarray.transpose(values,(1,0,2))
        # print "B type(values)",type(values)
    elif y[1]==y0:
        deltax = x[1]-x[0]
        while y[k]==y0:
            k = k+1
        deltay = y[k]-y0
        nx = k
        ny = n // nx
        # print 'B) nx,ny:',nx,ny
        values.shape = (ny,nx,valsize)
        # print "C type(values)",type(values)
        values = numarray.transpose(values,(1,0,2))
        # print "D type(values)",type(values)
    else:
        raise ValueError("Strange: x[1]!=x0 and y[1]!=y0 this doesn't look like a regular grid...")

    print 'In regular_xyval_to_2d_grid_values: ', type(xyval), type(values)
    return values, x0, y0, deltax, deltay


def divide_by_mean_magnitude(xymagnitude):
    mag = xymagnitude[:,2]
    meanval = mag.mean()
    mag *= 1./meanval
    return meanval

def divide_by_max_magnitude(xymagnitude):
    mag = xymagnitude[:,2]
    maxval = mag.max()
    mag *= 1./maxval
    return maxval

def transform_magnitude_into_covered_percentage(xymagnitude):
    magnitudes = []
    l = len(xymagnitude)
    for i in xrange(l):
        row = xymagnitude[i]
        magnitudes.append([row[-1],i])
    magnitudes.sort()
    # magnitude.reverse()
    cum = 0
    for row in magnitudes:
        mag, i = row
        cum += mag
        row[0] = cum
    for mag,i in magnitudes:
        xymagnitude[i][-1] = mag/cum
    return cum
        
def imshow_xymagnitude(regular_xymagnitude, interpolation='nearest', cmap = cm.jet):
    grid_values, x0, y0, deltax, deltay = regular_xyval_to_2d_grid_values(regular_xymagnitude)
    # print 'In imshow_xymagnitude: ', type(regular_xymagnitude), type(grid_values)
    imshow_2d_grid_values(grid_values, x0, y0, deltax, deltay, interpolation, cm.jet)
    
def imshow_xyrgb(regular_xyrgb, interpolation='nearest'):
    grid_rgb, x0, y0, deltax, deltay = regular_xyval_to_2d_grid_values(regular_xyrgb)
    # print 'grid_rgb shape=',grid_rgb.shape
    imshow_2d_grid_rgb(grid_rgb, x0, y0, deltax, deltay, interpolation, cm.jet)

def classcolor(winner,margin=0):
    colors = { 0: [0.5, 0.5, 1.0],
               1: [1.0, 0.5, 0.5],
               2: [0.5, 1.0, 0.5],
             }
    return colors[winner]
    
def xy_winner_magnitude_to_xyrgb(xy_winner_margin):
    res = []
    for x,y,w,m in xy_winner_margin:
        res.append([x,y]+classcolor(w,m))
    return res

def xymagnitude_to_x_y_grid(regular_xymagnitude):
    gridvalues, x0, y0, deltax, deltay = regular_xyval_to_2d_grid_values(regular_xymagnitude)
    nx = numarray.size(gridvalues,0)
    ny = numarray.size(gridvalues,1)
    gridvalues = numarray.reshape(gridvalues,(nx,ny))
    x = numarray.arange(x0,x0+nx*deltax-1e-6,deltax)
    y = numarray.arange(y0,y0+ny*deltay-1e-6,deltay)
    # print "x = ",x
    # print "y = ",y
    # print "z = ",gridvalues
    # print "type(x) = ",type(x)
    # print "type(y) = ",type(y)
    # print "type(z) = ",type(gridvalues)
    # imv.view(gridvalues)
    return x, y, gridvalues
    
def surfplot_xymagnitude(regular_xymagnitude):
    x,y,gridvalues = xymagnitude_to_x_y_grid(regular_xymagnitude)
    imv.surf(x, y, gridvalues)

def contour_xymagnitude(regular_xymagnitude):
    x,y,gridvalues = xymagnitude_to_x_y_grid(regular_xymagnitude)
    clabel(contour(x, y, gridvalues))
    
def contourf_xymagnitude(regular_xymagnitude):
    x,y,gridvalues = xymagnitude_to_x_y_grid(regular_xymagnitude)
    contourf(x, y, gridvalues)
    colorbar()

def imshow_2d_grid_rgb(gridrgb, x0, y0, deltax, deltay, interpolation='nearest', cmap = cm.jet):
    nx = numarray.size(gridrgb,0)
    ny = numarray.size(gridrgb,1)
    extent = (x0-.5*deltax, x0+nx*deltax, y0-.5*deltay, y0+ny*deltay)
    # gridrgb = numarray.reshape(gridrgb,(nx,ny))
    # print 'SHAPE:',gridrgb.shape
    # print 'gridrgb:', gridrgb
    imshow(gridrgb, cmap=cmap, origin='lower', extent=extent, interpolation=interpolation)
    colorbar()


def imshow_2d_grid_values(gridvalues, x0, y0, deltax, deltay, interpolation='nearest', cmap = cm.jet):
    nx = numarray.size(gridvalues,0)
    ny = numarray.size(gridvalues,1)
    extent = (x0-.5*deltax, x0+nx*deltax, y0-.5*deltay, y0+ny*deltay)
    print 'gridval type', type(gridvalues)
    gridvalues = numarray.reshape(gridvalues,(nx,ny))
    # print 'SHAPE:',gridvalues.shape
    # print 'gridvalues:', gridvalues
    imshow(gridvalues, cmap=cmap, origin='lower', extent=extent, interpolation=interpolation)
    colorbar()

def plot_2d_points(pointlist, style='bo'):
    x, y = zip(*pointlist)
    plot(x, y, style)

def plot_2d_class_points(pointlist, styles):
    classnum = 0
    for style in styles:
        points_c = [ [x,y] for x,y,c in pointlist if c==classnum]
        if len(points_c)==0:
            break
        # print 'points',classnum,':',points_c
        plot_2d_points(points_c, style)
        classnum += 1


# def generate_2D_color_plot(x_y_color):

# def plot_2D_decision_surface(training_points

def main():
    print "Still under development. Do not use!!!"
    extent = (1, 25, -5, 25)

    x = arange(7)
    y = arange(5)
    X, Y = meshgrid(x,y)
    Z = rand( len(x), len(y))
    # pcolor_classic(X, Y, transpose(Z))
    #show()
    #print 'pcolor'

    for interpol in ['bicubic',
                     'bilinear',
                     'blackman100',
                     'blackman256',
                     'blackman64',
                     'nearest',
                     'sinc144',
                     'sinc256',
                     'sinc64',
                     'spline16',
                     'spline36']:

        raw_input()
        print interpol
        clf()
        imshow(Z, cmap=cm.jet, origin='upper', extent=extent, interpolation=interpol)
        markers = [(15.9, 14.5), (16.8, 15), (20,20)]
        x,y = zip(*markers)
        plot(x, y, 'o')
        plot(rand(20)*25, rand(20)*25, 'o') 
        show()
        # draw()

    show()


if __name__ == "__main__":
    main()



#t = arange(0.0, 5.2, 0.2)

# red dashes, blue squares and green triangles
#plot(t, t, 'r--', t, t**2, 'bs', t, t**3, 'g^')
#show()

