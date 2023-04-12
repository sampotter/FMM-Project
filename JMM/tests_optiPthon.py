# Before going to C with the optimization ruting
# we test optiPython and intermediateTests here


import matplotlib.pyplot as plt
import numpy as np
from numpy.linalg import norm
from math import sqrt, pi, cos, sin
import intermediateTests as itt
from optiPython import blockCoordinateGradient, plotResults, fObj_noTops, gradient_TY
from analyticSol_circle import trueSolution # So that we can test all of this in an actual "true geometry
import colorcet as cc
import matplotlib.colors as clr
from mpl_toolkits.mplot3d import axes3d


colormap2  = clr.LinearSegmentedColormap.from_list('Retro',
                                                   [(0,    '#000000'),
                                                    (0.1, '#2c3454'),
                                                    (0.25, '#0033ff'),
                                                    (0.60, '#00f3ff'),
                                                    (1,    '#e800ff')], N=256)
maxIter = 30
tol = 1e-8
      
###########################################
## NAIVELY TEST OPTIPYTHON
## TESTS FOR THESE FUNCTIONS

x0 = np.array([0.0,0.0])
x1 = np.array([2, -0.2])
x2 = np.array([1.5, 0.8])
x3 = np.array([0.2, 1.2])
xHat = np.array([-0.8, 0.7])
B01 = np.array([2.2, 1])
B01 = B01/norm(B01)
B02 = np.array([1, 1.5])
B02 = B02/norm(B02)
B03 = np.array([0.2, 2])
B03 = B03/norm(B03)
B0Hat = np.array([-1, 0.4])
B0Hat = B0Hat/norm(B0Hat)
B1 = np.array([1, -0.6])
B1 = B1/norm(B1)
B2 = np.array([2, -0.2])
B2 = B2/norm(B2)
B3 = np.array([1, 2])
B3 = B3/norm(B3)
BHat = np.array([-1, 1])
BHat = BHat/norm(BHat)


mu1 = 0.15
lam2 = 0.15
mu2 = 0.13
lam3 = 0.17
mu3 = 0.15
lam4 = 1
params0 = [mu1, lam2, mu2, lam3, mu3, lam4]

xSource = np.array([ 1, -0.5 ])
T0 = norm(xSource - x0)
grad0 = (x0 - xSource)/T0
T1 = norm(xSource - x1)
grad1 = (x1 - xSource)/T1

THat_true = norm(xHat - xSource)

listIndices = [1.0, 1.0, 1.0, 1.0]
listxk = [x0, x1, x2, x3, xHat]
listB0k = [B01, B02, B03, B0Hat]
listBk = [B1, B2, B3, BHat]


# # Compute the projected gradient descent






# # Another example

print("Start test foward pass update \n\n")


mu1 = 0.5
lam2 = 0.65
mu2 = 0.5
lam3 = 0.75
mu3 = 0.5
lam4 = 0.45
params0 = [mu1, lam2, mu2, lam3, mu3, lam4]
listIndices = [1.0, 1.0, 1.0, 1.0]


paramsOpt, listObjVals, listGrads, listChangefObj = blockCoordinateGradient(params0, x0, T0, grad0, x1, T1, grad1, xHat, listIndices, listxk, listB0k, listBk, maxIter, tol, plotSteps = False)

plotResults(x0, T0, grad0, x1, T1, grad1, xHat, listIndices, listB0k, listxk, listBk, params0, paramsOpt, listObjVals, listGrads, listChangefObj, trueSol = None)



mu1 = 0.5
lam2 = 0.65
mu2 = 0.5
lam3 = 0.75
mu3 = 0.5
lam4 = 0.45
params = [mu1, lam2, mu2, lam3, mu3, lam4]
listIndices = [3.5, 1, 0.5, 1.0]


paramsOpt, listObjVals, listGrads, listChangefObj = blockCoordinateGradient(params, x0, T0, grad0, x1, T1, grad1, xHat, listIndices, listxk, listB0k, listBk, maxIter, tol, plotSteps = False)

plotResults(x0, T0, grad0, x1, T1, grad1, xHat, listIndices, listB0k, listxk, listBk, params0, paramsOpt, listObjVals, listGrads, listChangefObj, trueSol = None)

