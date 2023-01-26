

/*
Optimization methods for the 2D FMM
*/

#include "opti_method.h"
#include "linAlg.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>


double eikApproxLin(double T0, double T1, double lambda, double x0[2], double x1[2], double xHat[2], double indexRef) {
    // this is the approximation to the eikonal solution using local approximations everywhere

    double xlambda[2], firstPartxLambda[2], secondPartxLambda[2], xLamMinxHat[2], normAux, min1Lambda, gApprox;

    xlambda[0] = 0;
    xlambda[1] = 0;
    firstPartxLambda[0] = 0;
    firstPartxLambda[1] = 0;
    secondPartxLambda[0] = 0;
    secondPartxLambda[1] = 0;
    xLamMinxHat[0] = 0;
    xLamMinxHat[1] = 0;
    min1Lambda = 1-lambda;
    // printf("   aux printing eikApproxLin\n");
    // printf("            1-lambda = %fl\n", min1Lambda);
    scalar_times_2vec( min1Lambda, x0, firstPartxLambda ); // (1-lambda)*x0
    // printf("            (1-lambda)*x0 = (%fl,%fl)\n", firstPartxLambda[0], firstPartxLambda[1]);
    scalar_times_2vec(lambda, x1, secondPartxLambda); // lambda*x1
    // printf("            lambda*x1 = (%fl,%fl)\n", secondPartxLambda[0], secondPartxLambda[1]);
    vec2_addition( firstPartxLambda, secondPartxLambda, xlambda ); // (1-lambda)*x0 + lambda*x1
    // printf("            xlambda = (%fl,%fl)\n", xlambda[0], xlambda[1]);
    vec2_subtraction( xlambda, xHat, xLamMinxHat ); // xlambda - xHat
    // printf("            xlambda-xHat = (%fl, %fl)\n", xLamMinxHat[0], xLamMinxHat[1]);
    normAux = l2norm( xLamMinxHat ); //   || xlambda - xHat ||
    // printf("            norm(xlambda-xHat) = %fl\n", normAux);

    gApprox = min1Lambda*T0 + lambda*T1 + indexRef*normAux;

    return gApprox;
}

double gPrime(double T0, double T1, double lambda, double x0[2], double x1[2], double xHat[2], double indexRef){
    // auxiliary function to compute the function gPrime
    double x1Minx0[2], dotProduct, normAux, gPrim;
    double xlambda[2], firstPartxLambda[2], secondPartxLambda[2], xLamMinxHat[2], min1Lambda;

    xlambda[0] = 0;
    xlambda[1] = 0;
    firstPartxLambda[0] = 0;
    firstPartxLambda[1] = 0;
    secondPartxLambda[0] = 0;
    secondPartxLambda[1] = 0;
    xLamMinxHat[0] = 0;
    xLamMinxHat[1] = 0;
    min1Lambda = 1-lambda;

    scalar_times_2vec( min1Lambda, x0, firstPartxLambda ); // (1-lambda)*x0
    scalar_times_2vec(lambda, x1, secondPartxLambda); // lambda*x1
    vec2_addition( firstPartxLambda, secondPartxLambda, xlambda ); // (1-lambda)*x0 + lambda*x1
    vec2_subtraction( xlambda, xHat, xLamMinxHat ); // xlambda - xHat
    normAux = l2norm( xLamMinxHat ); //   || xlambda - xHat ||

    x1Minx0[0] = 0;
    x1Minx0[1] = 0;

    dotProduct = 0;
    normAux= 0;

    vec2_subtraction(x1, x0, x1Minx0);

    dotProduct = dotProd(x1Minx0, xLamMinxHat);
    normAux = l2norm(xLamMinxHat);

    gPrim = T1 - T0 + indexRef*dotProduct/normAux;

    return gPrim;

}

double secant_2D(double lambda0, double lambda1, double T0, double T1, double x0[2], double x1[2], double xHat[2], double tol, int maxIter, double indexRef){
    // This method is the implementation of the secant method for the 2D fmm using the
    // function defined in SoSFunction.h as the speed of sound
    int k = 1;
    double lam, gPrime0, gPrime1;

    gPrime1 = gPrime(T0, T1, lambda1, x0, x1, xHat, indexRef);
    // printf("Initial, with lambda = %fl   the value of gPrime is: %fl\n", lambda0, gPrime(T0, T1, lambda0, x0, x1, xHat, indexRef));
    // printf("Initial, with lambda = %fl   the value of gPrime is: %fl\n", lambda1, gPrime1);
    
    while(k < maxIter & fabs(gPrime1)>tol){
        // u
        gPrime0 = gPrime(T0, T1, lambda0, x0, x1, xHat, indexRef);
        gPrime1 = gPrime(T0, T1, lambda1, x0, x1, xHat, indexRef);
        lam = lambda1 - gPrime1*(lambda1 - lambda0)/( gPrime1 - gPrime0 );
        // printf("In this iteration lambda_i has value of %fl", lam);
        // printf("    |  value of T(xhat): %fl   with %fl as index of regraction\n", eikApproxLin(T0, T1, lam, x0, x1, xHat, indexRef), indexRef );
        // printf("    |   In this iteration the value of gPrime is: %fl \n", gPrime(T0, T1, lam, x0, x1, xHat, indexRef));
        lambda0 = lambda1;
        lambda1 = lam;
        k ++;
    }
    // check that lambda1 stays in the interval [0, 1]
    if(lambda1<0){
        lambda1 = 0;
    }
    if(lambda1 > 1){
        lambda1 = 1;
    }
    // printf("Optimum lambda found %fl\n", lambda1);
    // printf("With gPrime value of: %fl\n", gPrime(T0, T1, lambda1, x0, x1, xHat, indexRef));
    // printf("With the secant method, the optimal lambda found (for a simple update) is: %fl\n", lambda1);
    return lambda1;
}


// for the "two step" optimization part (i.e. the piece wise linear path goes through two different regions with different indices of refraction)

double eikApproxLin_2Regions(double T0, double T1, double lambda, double mu, double x0[2], double x1[2], double x2[2], double xHat[2], double indexRef_01, double indexRef_02) {
    // piece wise linear approximation of a ray from x0x1 passing through a change of region defined at x0x2 to get to xHat

    double xlambda[2], firstPartxLambda[2], secondPartxLambda[2], xMuMinxLam[2], normAux1, min1Lambda, gApprox;
    double xmu[2], firstPartxMu[2], seconPartxMu[2], xHatMinxMu[2], normAux2, min1Mu;

    min1Lambda = 1 - lambda;
    min1Mu = 1 - mu;

    scalar_times_2vec( min1Lambda, x0, firstPartxLambda ); // (1-lambda)*x0
    scalar_times_2vec(lambda, x1, secondPartxLambda); // lambda*x1

    scalar_times_2vec( min1Mu, x0, firstPartxMu ); // (1-mu)*x0
    scalar_times_2vec(mu, x2, seconPartxMu); // mu*x2

    vec2_addition( firstPartxLambda, secondPartxLambda, xlambda ); // (1-lambda)*x0 + lambda*x1

    vec2_addition( firstPartxMu, seconPartxMu, xmu ); // (1-mu)*x0 + mu*x2

    vec2_subtraction( xmu, xlambda, xMuMinxLam ); // xmu - xlambda

    vec2_subtraction( xHat, xmu, xHatMinxMu ); // xhat - xmu

    normAux1 = l2norm( xMuMinxLam ); //   || xmu - xlambda ||

    normAux2 = l2norm( xHatMinxMu ); //   || xhat - xmu ||

    gApprox = min1Lambda*T0 + lambda*T1 + indexRef_01*normAux1 + indexRef_02*normAux2;

    return gApprox;
}

void gradient_2Regions(double grad[2], double T0, double T1, double lambda, double mu, double x0[2], double x1[2], double x2[2], double xHat[2], double indexRef_01, double indexRef_02) {
    // auxiliary function to calculate the gradient of eikApproxLin_2Regions, the first entry corresponds to the partial with respect to lambda
    // and the second entry to the partial with respect to mu
    double xlambda[2], firstPartxLambda[2], secondPartxLambda[2], xMuMinxLam[2], normAux1, min1Lambda;
    double xmu[2], firstPartxMu[2], seconPartxMu[2], xHatMinxMu[2], normAux2, min1Mu;
    double x1Minx0[2], x2Minx0[2], dotProd1, dotProd2, dotProd3;

    min1Lambda = 1 - lambda;
    min1Mu = 1 - mu;

    scalar_times_2vec( min1Lambda, x0, firstPartxLambda ); // (1-lambda)*x0
    scalar_times_2vec(lambda, x1, secondPartxLambda); // lambda*x1

    scalar_times_2vec( min1Mu, x0, firstPartxMu ); // (1-mu)*x0
    scalar_times_2vec(mu, x2, seconPartxMu); // mu*x2

    vec2_addition( firstPartxLambda, secondPartxLambda, xlambda ); // (1-lambda)*x0 + lambda*x1

    vec2_addition( firstPartxMu, seconPartxMu, xmu ); // (1-mu)*x0 + mu*x2

    vec2_subtraction( xmu, xlambda, xMuMinxLam ); // xmu - xlambda

    vec2_subtraction( xHat, xmu, xHatMinxMu ); // xhat - xmu

    normAux1 = l2norm( xMuMinxLam ); //   || xmu - xlambda ||

    normAux2 = l2norm( xHatMinxMu ); //   || xhat - xmu ||

    vec2_subtraction(x1, x0, x1Minx0); // x1 - x0

    vec2_subtraction(x2, x0, x2Minx0); // x2 - x0

    dotProd1 = dotProd( xMuMinxLam, x1Minx0 ); // (xmu-xlam)T(x1-x0)
    dotProd2 = dotProd( xMuMinxLam, x2Minx0 ); // (xmu-xlam)T(x2-x0)
    dotProd3 = dotProd(xHatMinxMu, x2Minx0); // (xhat - xmu)T(x2-x0)
    // assemble
    grad[0] = T1 - T0 - indexRef_01*dotProd1/normAux1;
    grad[1] = indexRef_01*dotProd2/normAux1 - indexRef_02*dotProd3/normAux2;
}

void projectedGradientDescent(double optimizers[2], double T0, double T1, double x0[2], double x1[2], double x2[2], double xHat[2], double tol, int maxIter, double indexRef_01, double indexRef_02) {
    // this is the optimization method to find lambda and mu such that xlam is on the segment x0x1, xmu is on the segment x0x2, where
    // x0x2 defined the change in region from index of refraction indexRef_01 to index of refraction indexRef_02
    // optimizers[0] = lambda, optimizers[1] = mu
    // this is the projected gradient descent method with step sizes equal to 0.01
    double grad[2], step[2], yk[2], projectedGradient[2], t;
    int iter;
    iter = 0;
    optimizers[0] = 1.0;
    optimizers[1] = 1.0; // initial value of the optimizers
    // printf("\n\nIteration %d\n", iter);
    // printf("Optimizers at this step: %fl    , %fl\n", optimizers[0], optimizers[1]);
    // we have to calculate the current gradient
    gradient_2Regions(grad, T0, T1, optimizers[0], optimizers[1], x0, x1, x2, xHat, indexRef_01, indexRef_02);
    // printf("Gradient: (%fl, %fl)\n", grad[0], grad[1]);
    // steepest descent and calculation of the projected gradient
    projection01Cube(grad, projectedGradient);
    // printf("Projected gradient: (%fl, %fl)\n", projectedGradient[0], projectedGradient[1]);

    // iteration part
    while( l2norm(projectedGradient) > tol & iter < maxIter  ){
        iter ++;
        // printf("\n\nIteration %d\n", iter);
        // we have to calculate the current gradient
        // printf("Projected gradient: (%fl, %fl)  with norm %fl\n", projectedGradient[0], projectedGradient[1], l2norm(projectedGradient));
        t = backtracking(optimizers, grad, T0, T1, x0, x1, x2, xHat, indexRef_01, indexRef_02);
        scalar_times_2vec(t, grad, step);
        // printf("This steps %fl with direction: (%fl, %fl)\n", t, step[0], step[1]);
        vec2_subtraction( optimizers, step, yk );
        // printf("Without projecting back to the domain, the next optimizers would be   %fl   %fl\n", yk[0], yk[1]);
        // then we need to project it back to the feasible set
        projection01Cube(yk, optimizers);
        // printf("Optimizers at this step: %fl    , %fl\n", optimizers[0], optimizers[1]);
        // Tcurrent = eikApproxLin_2Regions(T0, T1, optimizers[0], optimizers[1], x0, x1, x2, xHat, indexRef_01, indexRef_02);
        // printf("Current value of T: %fl\n", Tcurrent);
        gradient_2Regions(grad, T0, T1, optimizers[0], optimizers[1], x0, x1, x2, xHat, indexRef_01, indexRef_02);
        // printf("Gradient: (%fl, %fl)  with norm %fl\n", grad[0], grad[1], l2norm(grad));
        // steepest descent and calculation of the projected gradient
        projection01Cube(grad, projectedGradient);
        if(optimizers[0] == 0 & optimizers[1] == 0){
            break;
        }
    }

    // printf("With the projected gradient method the value of lambda: %fl    , the value of mu: %fl\n", optimizers[0], optimizers[1]);

}

double backtracking(double optimizers[2], double gradient[2], double T0, double T1, double x0[2], double x1[2], double x2[2], double xHat[2], double indexRef_01, double indexRef_02){
    // backtracking algorithm to find the step size of the projected gradient descent method so that the
    // Armijo Wolfe conditions are met
    double t, alpha, beta, Tval_t, Tval, dotGradDirection, grad[2], direction[2];
    direction[0] = -gradient[0];
    direction[1] = -gradient[1];
    t = 0.05;
    alpha = 0.05;
    beta = 0.1;
    Tval_t = eikApproxLin_2Regions(T0, T1, optimizers[0] + t*direction[0], optimizers[1] + t*direction[1], x0, x1, x2, xHat, indexRef_01, indexRef_02);
    Tval = eikApproxLin_2Regions(T0, T1, optimizers[0], optimizers[1], x0, x1, x2, xHat, indexRef_01, indexRef_02);
    gradient_2Regions(grad, T0, T1, optimizers[0], optimizers[1], x0, x1, x2, xHat, indexRef_01, indexRef_02);
    dotGradDirection = dotProd( grad, direction );
    // backtracking part
    while(  Tval_t > Tval + alpha*t*dotGradDirection  ){
        t = beta*t;
        Tval_t = eikApproxLin_2Regions(T0, T1, optimizers[0] + t*direction[0], optimizers[1] + t*direction[1], x0, x1, x2, xHat, indexRef_01, indexRef_02);
    }
    return t;
}

//////////////////////////////////////////////////////////////////////////////////////////////////
///// OPTIMIZATION METHODS - INTERPOLATING T(LAMBDA) WITH CUBIC HERMITE INTERPOLATION

double eikApprox_freeSpace(double T0, double T1, double grad0[2], double grad1[2],
		    double lambda, double x0[2], double x1[2], double xHat[2], double indexRef) {
    // this is the approximation to the eikonal solution using Hermite interpolation (because
   // we also approximate the gradient so we use it

    double xlambda[2], firstPartxLambda[2], secondPartxLambda[2], xLamMinxHat[2], normAux, min1Lambda, gApprox;
    double lambdaSq, lambdaCub, hermPol, dotXGrad0, dotXGrad1, x1Minx0[2];
      
    xlambda[0] = 0;
    xlambda[1] = 0;
    firstPartxLambda[0] = 0;
    firstPartxLambda[1] = 0;
    secondPartxLambda[0] = 0;
    secondPartxLambda[1] = 0;
    xLamMinxHat[0] = 0;
    xLamMinxHat[1] = 0;
    min1Lambda = 1-lambda;
    scalar_times_2vec( min1Lambda, x0, firstPartxLambda ); // (1-lambda)*x0
    scalar_times_2vec(lambda, x1, secondPartxLambda); // lambda*x1
    vec2_addition( firstPartxLambda, secondPartxLambda, xlambda ); // (1-lambda)*x0 + lambda*x1
    vec2_subtraction( xlambda, xHat, xLamMinxHat ); // xlambda - xHat
    normAux = l2norm( xLamMinxHat ); //   || xlambda - xHat ||
    // For the part of the Hermite interpolation we need the following auxiliary variables
    lambdaSq = pow(lambda, 2); // lambda^2
    lambdaCub = pow(lambda, 3); // lambda^3
    vec2_subtraction(x1, x0, x1Minx0); // x1-x0
    dotXGrad0 = dotProd(x1Minx0, grad0); // (x1-x0)T(grad0)
    dotXGrad1 = dotProd(x1Minx0, grad1); // (x1-x0)T(grad1)
    hermPol = T0*(2*lambdaCub - 3*lambdaSq + 1) + T1*(-2*lambdaCub + 3*lambdaSq);
    hermPol += dotXGrad0*(lambdaCub - 2*lambdaSq + lambda) + dotXGrad1*(lambdaCub - lambdaSq);
    // now we have the approximation gApprox = HermiteInterpolation + nu*distance
    gApprox = hermPol + indexRef*normAux;

    return gApprox;
}

double gPrimeCubic(double T0, double T1, double grad0[2], double grad1[2], double lambda,
		   double x0[2], double x1[2], double xHat[2], double indexRef){
    // auxiliary function to compute the function gPrime when we are approximation T(xLambda)
   // using Hermite interpolation
    double x1Minx0[2], dotProduct, normAux, gPrim;
    double xlambda[2], firstPartxLambda[2], secondPartxLambda[2], xLamMinxHat[2], min1Lambda;
    double dotXGrad0, dotXGrad1, lambdaSq, devHerm;

    xlambda[0] = 0;
    xlambda[1] = 0;
    firstPartxLambda[0] = 0;
    firstPartxLambda[1] = 0;
    secondPartxLambda[0] = 0;
    secondPartxLambda[1] = 0;
    xLamMinxHat[0] = 0;
    xLamMinxHat[1] = 0;
    min1Lambda = 1-lambda;

    scalar_times_2vec( min1Lambda, x0, firstPartxLambda ); // (1-lambda)*x0
    scalar_times_2vec(lambda, x1, secondPartxLambda); // lambda*x1
    vec2_addition( firstPartxLambda, secondPartxLambda, xlambda ); // (1-lambda)*x0 + lambda*x1
    vec2_subtraction( xlambda, xHat, xLamMinxHat ); // xlambda - xHat
    normAux = l2norm( xLamMinxHat ); //   || xlambda - xHat ||

    x1Minx0[0] = 0;
    x1Minx0[1] = 0;

    dotProduct = 0;
    normAux= 0;

    vec2_subtraction(x1, x0, x1Minx0);

    dotProduct = dotProd(x1Minx0, xLamMinxHat);
    normAux = l2norm(xLamMinxHat);

    // For the part of the derivative that corresponds to the hermite polynomial:
    lambdaSq = pow(lambda, 2); // lambda^2
    vec2_subtraction(x1, x0, x1Minx0); // x1-x0
    dotXGrad0 = dotProd(x1Minx0, grad0); // (x1-x0)T(grad0)
    dotXGrad1 = dotProd(x1Minx0, grad1); // (x1-x0)T(grad1)
    devHerm = T0*(6*lambdaSq -6*lambda) + T1*(-6*lambdaSq + 6*lambda);
    devHerm += dotXGrad0*(3*lambdaSq - 4*lambda + 1) + dotXGrad1*(3*lambdaSq - 2*lambda);

    gPrim = devHerm + indexRef*dotProduct/normAux;

    return gPrim;

}

double secant_freeSpace(double lambda0, double lambda1, double T0, double T1, double grad0[2], double grad1[2],
		     double x0[2], double x1[2], double xHat[2], double tol, int maxIter, double indexRef){
  // optimization on free space with Hermite interpolation for T
    // This method is the implementation of the secant method to optimize the path from xlambda to xhat
   // T(xlambda) is approximated with a cubic Hermite polynomial 
    int k = 1;
    double lam, gPrime0, gPrime1;

    gPrime1 = gPrimeCubic(T0, T1, grad0, grad1, lambda1, x0, x1, xHat, indexRef);
    // printf("Initial, with lambda = %fl   the value of gPrime is: %fl\n", lambda0, gPrime(T0, T1, lambda0, x0, x1, xHat, indexRef));
    // printf("Initial, with lambda = %fl   the value of gPrime is: %fl\n", lambda1, gPrime1);
    
    while(k < maxIter & fabs(gPrime1)>tol){
      gPrime0 = gPrimeCubic(T0, T1, grad0, grad1, lambda0, x0, x1, xHat, indexRef);
      gPrime1 = gPrimeCubic(T0, T1, grad0, grad1, lambda1, x0, x1, xHat, indexRef);
      lam = lambda1 - gPrime1*(lambda1 - lambda0)/( gPrime1 - gPrime0 );
       lambda0 = lambda1;
       lambda1 = lam;
       k ++;
    }
    if(lambda1<0){
        lambda1 = 0;
    }
    if(lambda1 > 1){
        lambda1 = 1;
    }
    return lambda1;
}

double eikApproxCubic_2Regions(double T0, double T1, double grad0[2], double grad1[2],
			       double lambda, double mu, double x0[2], double x1[2],
			       double x2[2], double xHat[2], double indexRef_01, double indexRef_02) {
    // approximation of a ray from x0x1 passing through a change of region defined at x0x2 to get to xHat
    // the approximation of T(xlambda) is computed using cubic hermite polynomial

    double xlambda[2], firstPartxLambda[2], secondPartxLambda[2], xMuMinxLam[2], normAux1, min1Lambda, gApprox;
    double xmu[2], firstPartxMu[2], seconPartxMu[2], xHatMinxMu[2], normAux2, min1Mu;
    double lambdaSq, lambdaCub, hermPol, dotXGrad0, dotXGrad1, x1Minx0[2];

    min1Lambda = 1 - lambda;
    min1Mu = 1 - mu;

    scalar_times_2vec( min1Lambda, x0, firstPartxLambda ); // (1-lambda)*x0
    scalar_times_2vec(lambda, x1, secondPartxLambda); // lambda*x1

    scalar_times_2vec( min1Mu, x0, firstPartxMu ); // (1-mu)*x0
    scalar_times_2vec(mu, x2, seconPartxMu); // mu*x2

    vec2_addition( firstPartxLambda, secondPartxLambda, xlambda ); // (1-lambda)*x0 + lambda*x1

    vec2_addition( firstPartxMu, seconPartxMu, xmu ); // (1-mu)*x0 + mu*x2

    vec2_subtraction( xmu, xlambda, xMuMinxLam ); // xmu - xlambda

    vec2_subtraction( xHat, xmu, xHatMinxMu ); // xhat - xmu

    normAux1 = l2norm( xMuMinxLam ); //   || xmu - xlambda ||

    normAux2 = l2norm( xHatMinxMu ); //   || xhat - xmu ||

    // For the part of the Hermite interpolation we need the following auxiliary variables
    lambdaSq = pow(lambda, 2); // lambda^2
    lambdaCub = pow(lambda, 3); // lambda^3
    vec2_subtraction(x1, x0, x1Minx0); // x1-x0
    dotXGrad0 = dotProd(x1Minx0, grad0); // (x1-x0)T(grad0)
    dotXGrad1 = dotProd(x1Minx0, grad1); // (x1-x0)T(grad1)
    hermPol = T0*(2*lambdaCub - 3*lambdaSq + 1) + T1*(-2*lambdaCub + 3*lambdaSq);
    hermPol += dotXGrad0*(lambdaCub - 2*lambdaSq + lambda) + dotXGrad1*(lambdaCub - lambdaSq);

    gApprox = hermPol + indexRef_01*normAux1 + indexRef_02*normAux2;

    return gApprox;
}

void gradientCubic_2Regions(double grad[2], double T0, double T1, double grad0[2], double grad1[2],
			    double lambda, double mu, double x0[2], double x1[2], double x2[2],
			    double xHat[2], double indexRef_01, double indexRef_02) {
    // auxiliary function to calculate the gradient of eikApproxCubic_2Regions,
    //the first entry corresponds to the partial with respect to lambda
    // and the second entry to the partial with respect to mu
    double xlambda[2], firstPartxLambda[2], secondPartxLambda[2], xMuMinxLam[2], normAux1, min1Lambda;
    double xmu[2], firstPartxMu[2], seconPartxMu[2], xHatMinxMu[2], normAux2, min1Mu;
    double x1Minx0[2], x2Minx0[2], dotProd1, dotProd2, dotProd3;
    double dotXGrad0, dotXGrad1, lambdaSq, devHerm;

    min1Lambda = 1 - lambda;
    min1Mu = 1 - mu;

    scalar_times_2vec( min1Lambda, x0, firstPartxLambda ); // (1-lambda)*x0
    scalar_times_2vec(lambda, x1, secondPartxLambda); // lambda*x1

    scalar_times_2vec( min1Mu, x0, firstPartxMu ); // (1-mu)*x0
    scalar_times_2vec(mu, x2, seconPartxMu); // mu*x2

    vec2_addition( firstPartxLambda, secondPartxLambda, xlambda ); // (1-lambda)*x0 + lambda*x1

    vec2_addition( firstPartxMu, seconPartxMu, xmu ); // (1-mu)*x0 + mu*x2

    vec2_subtraction( xmu, xlambda, xMuMinxLam ); // xmu - xlambda

    vec2_subtraction( xHat, xmu, xHatMinxMu ); // xhat - xmu

    normAux1 = l2norm( xMuMinxLam ); //   || xmu - xlambda ||

    normAux2 = l2norm( xHatMinxMu ); //   || xhat - xmu ||

    vec2_subtraction(x1, x0, x1Minx0); // x1 - x0

    vec2_subtraction(x2, x0, x2Minx0); // x2 - x0

    dotProd1 = dotProd( xMuMinxLam, x1Minx0 ); // (xmu-xlam)T(x1-x0)
    dotProd2 = dotProd( xMuMinxLam, x2Minx0 ); // (xmu-xlam)T(x2-x0)
    dotProd3 = dotProd(xHatMinxMu, x2Minx0); // (xhat - xmu)T(x2-x0)

    // For the part of the derivative that corresponds to the hermite polynomial in grad[0]:
    lambdaSq = pow(lambda, 2); // lambda^2
    vec2_subtraction(x1, x0, x1Minx0); // x1-x0
    dotXGrad0 = dotProd(x1Minx0, grad0); // (x1-x0)T(grad0)
    dotXGrad1 = dotProd(x1Minx0, grad1); // (x1-x0)T(grad1)
    devHerm = T0*(6*lambdaSq -6*lambda) + T1*(-6*lambdaSq + 6*lambda);
    devHerm += dotXGrad0*(3*lambdaSq - 4*lambda + 1) + dotXGrad1*(3*lambdaSq - 2*lambda);
    
    // assemble
    grad[0] = devHerm - indexRef_01*dotProd1/normAux1;
    grad[1] = indexRef_01*dotProd2/normAux1 - indexRef_02*dotProd3/normAux2;
}

void projectedGradientDescentCubic(double optimizers[2], double T0, double T1, double grad0[2], double grad1[2],
			      double x0[2], double x1[2], double x2[2], double xHat[2], double tol,
			      int maxIter, double indexRef_01, double indexRef_02) {
    // this is the optimization method to find lambda and mu such that xlam is on the segment x0x1,
    //xmu is on the segment x0x2, where
    // x0x2 defined the change in region from index of refraction indexRef_01 to index of refraction indexRef_02
    // optimizers[0] = lambda, optimizers[1] = mu
    // this is the projected gradient descent method with step sizes equal to 0.01
    double grad[2], step[2], yk[2], projectedGradient[2], t;
    int iter;
    iter = 0;
    optimizers[0] = 1.0;
    optimizers[1] = 1.0; // initial value of the optimizers
    // we have to calculate the current gradient
    gradientCubic_2Regions(grad, T0, T1, grad0,grad1,optimizers[0], optimizers[1], x0, x1, x2, xHat, indexRef_01, indexRef_02);
    // steepest descent and calculation of the projected gradient
    projection01Cube(grad, projectedGradient);

    // iteration part
    while( l2norm(projectedGradient) > tol & iter < maxIter  ){
        iter ++;
        // we have to calculate the current gradient
        t = backtracking(optimizers, grad, T0, T1, x0, x1, x2, xHat, indexRef_01, indexRef_02);
        scalar_times_2vec(t, grad, step);
        vec2_subtraction( optimizers, step, yk );
        // then we need to project it back to the feasible set
        projection01Cube(yk, optimizers);
        gradientCubic_2Regions(grad,T0,T1,grad0,grad1,optimizers[0],optimizers[1], x0, x1, x2, xHat, indexRef_01, indexRef_02);
        // steepest descent and calculation of the projected gradient
        projection01Cube(grad, projectedGradient);
        if(optimizers[0] == 0 & optimizers[1] == 0){
            break;
        }
    }
}

double der_fromEdge(double lambda, double T0, double grad0[2], double B0[2], double T1, double grad1[2], double B1[2], double x0[2], double x1[2], double xHat[2], double indexRef) {
  // derivative with respect of lambda from the function to minimize for the update in which the
  // segment x0x1 is on the boundary but xHat if fully contained in a region with index indexRef
  double lambda3, lambda2;
  lambda2 = lambda*lambda;
  lambda3 = lambda2*lambda;
  double x1Minx0[2], B0PlusB1[2], B1Plus2B0[2], twoB0[2], grad0Plusgrad1[2], twoGrad0PlusGrad1[2], twoGrad0[2];
  double lamB0[2], x0MinxHat[2];
  // first time we gather terms
  vec2_subtraction(x1, x0, x1Minx0);
  vec2_addition(B0, B1, B0PlusB1);
  scalar_times_2vec(2, B0, twoB0);
  vec2_addition(B1, twoB0, B1Plus2B0);
  vec2_addition(grad0, grad1, grad0Plusgrad1);
  scalar_times_2vec(2, grad0, twoGrad0);
  vec2_addition(twoGrad0, grad1, twoGrad0PlusGrad1);
  scalar_times_2vec(lambda, B0, lamB0);
  vec2_subtraction(x0, xHat, x0MinxHat);
  // second time gathering terms
  double dotProd1, dotProd2, sixX0minx1[2], threeB0plusB1[2], twoB1Plus2B0[2], twox0Minx1[2], threex0Minx1[2];
  dotProd1 = dotProd(x1Minx0, grad0Plusgrad1);
  dotProd2 = dotProd(x1Minx0, twoGrad0PlusGrad1);
  scalar_times_2vec(-6, x1Minx0, sixX0minx1);
  scalar_times_2vec(3, B0PlusB1, threeB0plusB1);
  scalar_times_2vec(2, B1Plus2B0, twoB1Plus2B0);
  scalar_times_2vec(-2, x1Minx0, twox0Minx1);
  scalar_times_2vec(-3, x1Minx0, threex0Minx1);
  // third time gathering terms
  double coeflam2_1[2], coeflam_1[2], coef_lam3_2[2], coef_lam2_2[2], coef_lam_2[2];
  vec2_addition(sixX0minx1, threeB0plusB1, coeflam2_1);
  vec2_addition(sixX0minx1,  twoB1Plus2B0, coeflam_1);
  vec2_addition(twox0Minx1, B0PlusB1, coef_lam3_2);
  vec2_addition(threex0Minx1, B1Plus2B0, coef_lam2_2);
  scalar_times_2vec(lambda, B0, coef_lam_2);
  // fourth time gathering terms
  double lam2_1[2], lam1_1[2], rest1_1[2], lam3_2[2], lam2_2[2], rest1_2[2], rest2_2[2];
  scalar_times_2vec(lambda2, coeflam2_1, lam2_1);
  scalar_times_2vec(lambda, coeflam_1, lam1_1);
  vec2_subtraction(lam2_1, lam1_1, rest1_1);
  scalar_times_2vec(lambda3, coef_lam3_2, lam3_2);
  scalar_times_2vec(lambda2, coef_lam2_2, lam2_2);
  vec2_addition(lamB0, x0MinxHat, rest2_2);
  vec2_subtraction(lam3_2, lam2_2, rest1_2);
  // fifth time gathering terms
  double derInside[2], xHatMinxLam[2], dotProd3, norm1, boundaryPart, tLamPart, dotProd4;
  vec2_addition(rest1_1, B0, derInside);
  vec2_addition(rest1_2, rest2_2, xHatMinxLam);
  dotProd3 = dotProd(derInside, xHatMinxLam);
  dotProd4 = dotProd(x1Minx0, grad0);
  norm1 = l2norm(xHatMinxLam);
  boundaryPart = indexRef*dotProd3/norm1;
  tLamPart = 6*T0*lambda2 - 6*T1*lambda2 + 3*lambda2*dotProd1 -6*T0*lambda + 6*T1*lambda - 2*lambda*dotProd2 + dotProd4;
  // Putting everything together
  return tLamPart + boundaryPart;
}

double backTr_fromEdge(double alpha0, double d, double lambda, double T0, double grad0[2], double B0[2], double T1, double grad1[2], double B1[2], double x0[2], double x1[2], double xHat[2], double indexRef){
  // backtracking method for projected gradient descent from updates from the edge of the domain
  double f_prev, f_cur, alpha;
  int i = 0;
  alpha = alpha0;
  // EVALUATING THE OBJECTIVE FUNCTION
  f_prev = fobjective_fromEdge(lambda, T0, grad0, B0, T1, grad1, B1, x0, x1, xHat, indexRef);
  f_cur = fobjective_fromEdge(lambda - alpha*d, T0, grad0, B0, T1, grad1, B1, x0, x1, xHat, indexRef);
  while(f_prev <= f_cur & i < 10){
    alpha = alpha*0.5;
    f_cur = fobjective_fromEdge(lambda - alpha*d, T0, grad0, B0, T1, grad1, B1, x0, x1, xHat, indexRef);
    i ++;
  }
  return alpha;
}

double fobjective_fromEdge(double lambda, double T0, double grad0[2], double B0[2], double T1, double grad1[2], double B1[2], double x0[2], double x1[2], double xHat[2], double indexRef){
  double lambda3, lambda2;
  lambda2 = lambda*lambda;
  lambda3 = lambda2*lambda;
  double x1Minx0[2], B0PlusB1[2], B1Plus2B0[2], twoB0[2], grad0Plusgrad1[2], twoGrad0PlusGrad1[2], twoGrad0[2];
  double lamB0[2], x0MinxHat[2];
  // first time we gather terms
  vec2_subtraction(x1, x0, x1Minx0);
  vec2_addition(B0, B1, B0PlusB1);
  scalar_times_2vec(2, B0, twoB0);
  vec2_addition(B1, twoB0, B1Plus2B0);
  vec2_addition(grad0, grad1, grad0Plusgrad1);
  scalar_times_2vec(2, grad0, twoGrad0);
  vec2_addition(twoGrad0, grad1, twoGrad0PlusGrad1);
  scalar_times_2vec(lambda, B0, lamB0);
  vec2_subtraction(x0, xHat, x0MinxHat);
  // second time gathering terms
  double dotProd1, dotProd2, twox0Minx1[2], threex0Minx1[2];
  dotProd1 = dotProd(x1Minx0, grad0Plusgrad1);
  dotProd2 = dotProd(x1Minx0, twoGrad0PlusGrad1);
  scalar_times_2vec(-2, x1Minx0, twox0Minx1);
  scalar_times_2vec(-3, x1Minx0, threex0Minx1);
  // third time gathering terms
  double coef_lam3_2[2], coef_lam2_2[2], coef_lam_2[2];
  vec2_addition(twox0Minx1, B0PlusB1, coef_lam3_2);
  vec2_addition(threex0Minx1, B1Plus2B0, coef_lam2_2);
  scalar_times_2vec(lambda, B0, coef_lam_2);
  // fourth time gathering terms
  double lam3_2[2], lam2_2[2], rest1_2[2], rest2_2[2];
  scalar_times_2vec(lambda3, coef_lam3_2, lam3_2);
  scalar_times_2vec(lambda2, coef_lam2_2, lam2_2);
  vec2_addition(lamB0, x0MinxHat, rest2_2);
  vec2_subtraction(lam3_2, lam2_2, rest1_2);
  // fifth time gathering terms
  double xHatMinxLam[2], norm1, boundaryPart, tLamPart, dotProd4;
  vec2_addition(rest1_2, rest2_2, xHatMinxLam);
  dotProd4 = dotProd(x1Minx0, grad0);
  norm1 = l2norm(xHatMinxLam);
  tLamPart = 2*T0*lambda3 - 2*T1*lambda3 + lambda3*dotProd1 -3*T0*lambda2 + 3*T1*lambda2 - lambda2*dotProd2 + lambda*dotProd4 + T0;
  return tLamPart + indexRef*norm1;
}


double projectedGradient_fromEdge(double lambda0, double T0, double grad0[2], double B0[2], double T1, double grad1[2], double B1[2], double x0[2], double x1[2], double xHat[2], double tol, double maxIter, double indexRef){
  // projected gradient descent for an update in which the segment x0x1 is on the boundary but xHat
  // if fully contained in a region with index indexRef
  double grad_cur, grad_prev, step, alpha, lam_prev, lam_cur, test;
  int i;
  i = 1;
  alpha = 0.25;
  lam_prev = lambda0;
  grad_cur = der_fromEdge(lambda0, T0, grad0, B0, T1, grad1, B1, x0, x1, xHat, indexRef);
  grad_prev = der_fromEdge(lambda0, T0, grad0, B0, T1, grad1, B1, x0, x1, xHat, indexRef);
  if(fabs(grad_cur) > tol){
    test = lam_prev - alpha*grad_cur;
  }
  else{
    test = lam_prev;
  }
  if(test>1){
    lam_cur = 1;
  }
  else if(test<0){
    lam_cur = 0;
  }
  else{
    lam_cur = test;
  }
  grad_cur = der_fromEdge(lam_cur, T0, grad0, B0, T1, grad1, B1, x0, x1, xHat, indexRef);

  while(i<maxIter & fabs(grad_cur)>tol & fabs(lam_cur - lam_prev)>0) {
    alpha = backTr_fromEdge(0.25, grad_cur, lam_cur, T0, grad0, B0, T1, grad1, B1, x0, x1, xHat, indexRef);
    test = lam_cur - alpha*grad_cur;
    if(test<0){
      test = 0;
    }
    if(test>1){
      test = 1;
    }
    grad_prev = grad_cur;
    lam_prev = lam_cur;
    lam_cur = test;
    grad_cur = der_fromEdge(lam_cur, T0, grad0, B0, T1, grad1, B1, x0, x1, xHat, indexRef);
    i ++;
  }
  
  return lam_cur;
}

double der_freeSpace(double lambda, double TA, double gradA[2], double TB, double gradB[2], double xA[2], double xB[2], double xHat[2], double indexRef){
  double lambda2;
  lambda2 = lambda*lambda;
  // first time we gather terms
  double  xBminxA[2], gradAplusgradB[2], twogradA[2], twogradAplusgradB[2], xHatMinxA[2];
  vec2_subtraction(xB, xA, xBminxA);
  vec2_addition(gradA, gradB, gradAplusgradB);
  scalar_times_2vec(2, gradA, twogradA);
  vec2_addition(twogradA, gradB, twogradAplusgradB);
  vec2_subtraction(xHat, xA, xHatMinxA);
  // second time we gather terms
  double dotProd1, dotProd2, dotProd3, lamxBminxA[2], disxlam[2], dotProd4;
  dotProd1 = dotProd(xBminxA, gradAplusgradB);
  dotProd2 = dotProd(xBminxA, twogradAplusgradB);
  dotProd3 = dotProd(xBminxA, gradA);
  scalar_times_2vec(lambda, xBminxA, lamxBminxA);
  vec2_subtraction(xHatMinxA, lamxBminxA, disxlam);
  dotProd4 = dotProd(xBminxA, disxlam);
  // third time we gather terms
  double tLamPart, rayPart;
  tLamPart = (6*TA - 6*TB + 3*dotProd1)*lambda2 + (-6*TA + 6*TB - 2*dotProd2)*lambda + dotProd3;
  rayPart = indexRef*(dotProd4)/l2norm(disxlam);
  return tLamPart - rayPart;
}

double backTr_freeSpace(double alpha0, double d, double lambda, double TA, double gradA[2], double TB, double gradB[2], double xA[2], double xB[2], double xHat[2], double indexRef){
  double f_prev, f_cur, alpha;
  int i = 0;
  alpha = alpha0;
  // EVALUATING THE OBJECTIVE FUNCTION
  f_prev = fobjective_freeSpace(lambda, TA, gradA, TB, gradB, xA, xB, xHat, indexRef);
  f_cur = fobjective_freeSpace(lambda - alpha*d, TA, gradA, TB, gradB, xA, xB, xHat, indexRef);
  while(f_prev <= f_cur & i < 10){
    alpha = alpha*0.5;
    f_cur = fobjective_freeSpace(lambda - alpha*d, TA, gradA, TB, gradB, xA, xB, xHat, indexRef);
    i ++;
  }
  return alpha;
}

double fobjective_freeSpace(double lambda, double TA, double gradA[2], double TB, double gradB[2], double xA[2], double xB[2], double xHat[2], double indexRef){
  double lambda2, lambda3;
  lambda2 = lambda*lambda;
  lambda3 = lambda2*lambda;
  // first time we gather terms
  double xBminxA[2], gradAplusgradB[2], twogradA[2], twogradAplusgradB[2], lamxBminxA[2], xHatminxA[2];
  vec2_subtraction(xB, xA, xBminxA);
  vec2_addition(gradA, gradB, gradAplusgradB);
  scalar_times_2vec(2, gradA, twogradA);
  vec2_addition(twogradA, gradB, twogradAplusgradB);
  scalar_times_2vec(lambda, xBminxA, lamxBminxA);
  vec2_subtraction(xHat, xA, xHatminxA);
  // second time we gather terms
  double dotProd1, dotProd2, dotProd3, disxlam[2], norm1;
  dotProd1 = dotProd(xBminxA, gradAplusgradB);
  dotProd2 = dotProd(xBminxA, twogradAplusgradB);
  dotProd3 = dotProd(xBminxA, gradA);
  vec2_subtraction(xHatminxA, lamxBminxA, disxlam);
  norm1 = l2norm(disxlam);
  return (2*TA - 2*TB + dotProd1)*lambda3 + (-3*TA + 3*TB - dotProd2)*lambda2 + dotProd3*lambda + TA + indexRef*norm1;
}

double projectedGradient_freeSpace(double lambda0, double lambdaMin, double lambdaMax, double TA, double gradA[2], double TB, double gradB[2], double xA[2], double xB[2], double xHat[2], double tol, double maxIter, double indexRef){
  // two point optimization problem. xA and xHat are on the boundary and xB is fully contained in one region.
  // With usual notation xA could be either x0 or x1 and xB could be either x0 or x1 (this is more abstract)
  // THIS IS A PROJECTED GRADIENT DESCENT METHOD
  double grad_cur, grad_prev, step, alpha, lam_prev, lam_cur, test;
  int i =1;
  alpha = 1;
  lam_prev = lambda0;
  grad_cur = der_freeSpace(lambda0, TA, gradA, TB, gradB, xA, xB, xHat, indexRef);
  grad_prev = grad_cur;
  if(fabs(grad_cur) > tol){
    test = lam_prev - alpha*grad_cur;
  }
  else{
    test = lam_prev;
  }
  if(test>lambdaMax){
    test = lambdaMax;
  }
  if(test<lambdaMin){
    test = lambdaMin;
  }
  lam_cur = test;
  grad_cur = der_freeSpace(lam_cur, TA, gradA, TB, gradB, xA, xB, xHat, indexRef);

  while( i<maxIter & fabs(grad_cur)>tol & fabs(lam_cur - lam_prev)>0){
    alpha = backTr_freeSpace(1, grad_cur, lam_cur, TA, gradA, TB, gradB, xA, xB, xHat, indexRef);
    printf("\n\nIteration %d\n", i);
    printf("Step size %lf   with direction  %lf, hence change is %lf\n", alpha, -grad_cur, -alpha*grad_cur);
    test = lam_cur - alpha*grad_cur;
    if(test > lambdaMax){
      test = lambdaMax;
    }
    if(test < lambdaMin){
      test = lambdaMin;
    }

    grad_prev = grad_cur;
    lam_prev = lam_cur;
    lam_cur = test;
    grad_cur = der_freeSpace(lam_cur, TA, gradA, TB, gradB, xA, xB, xHat, indexRef);
    printf("Iteration %d   with lam_prev %lf  and lam_cur %lf   with objective value: %lf   and derivative  %lf \n\n\n", i, lam_prev, lam_cur, fobjective_freeSpace(lam_cur, TA, gradA, TB, gradB, xA, xB, xHat, indexRef), grad_cur);
    
    i++;
  }

  return lam_cur;
}

void grad_twoStep(double gradient[2], double lambda, double mu, double T0, double grad0[2], double T1, double grad1[2], double x0[2], double x1[2], double x2[2], double xHat[2], double B0[2], double B2[2], double indexRef_01, double indexRef_02) {
  // gradient for the function to minimize for a two step update (when there is a change in region)
  double mu2, mu3, lambda2;
  mu2 = mu*mu;
  mu3 = mu2*mu;
  lambda2 = lambda*lambda;
  /////// partial with respect to lambda
  // first time gathering terms
  double x1Minx0[2], x0InxLamMinxMu[2], x1inxLamMinxMu[2], B0inxLamMinxMu[2], x2InxLamMinxMu[2], B2inxLamMinxMu[2];
  vec2_subtraction(x1, x0, x1Minx0);
  scalar_times_2vec(-lambda-2*mu3 + 3*mu2, x0, x0InxLamMinxMu);
  scalar_times_2vec(lambda, x1, x1inxLamMinxMu);
  scalar_times_2vec(mu3 - 2*mu2 + mu, B0, B0inxLamMinxMu);
  scalar_times_2vec(-2*mu3 + 3*mu2, x2, x2InxLamMinxMu);
  scalar_times_2vec(mu3 - mu2, B2, B2inxLamMinxMu);
  // second time gatherieng terms
  double sum1[2], sum2[2], sum3[2], xLamMinxMu[2], t, normxLamMinxMu, coefgrad0[2], coefgrad1[2], sumgrads[2], dot1;
  vec2_addition(x0InxLamMinxMu, x1inxLamMinxMu, sum1);
  vec2_addition(B0inxLamMinxMu, x2InxLamMinxMu, sum2);
  vec2_addition(sum2, B2inxLamMinxMu, sum3);
  vec2_subtraction(sum1, sum3, xLamMinxMu);
  t = dotProd(x1Minx0, xLamMinxMu);
  normxLamMinxMu = l2norm(xLamMinxMu);
  scalar_times_2vec(3*lambda2 - 4*lambda + 1, grad0, coefgrad0);
  scalar_times_2vec(3*lambda2 - 2*lambda, grad1, coefgrad1);
  vec2_addition(coefgrad0, coefgrad1, sumgrads);
  dot1 = dotProd(x1Minx0, sumgrads);
  // then the partial is
  gradient[0] =(6*lambda2 - 6*lambda)*T0 + (-6*lambda2 + 6*lambda)*T1 + dot1 + indexRef_01*t/normxLamMinxMu;
  /////// partial with respect to mu
  // first time gathering terms
  double der_coefx0[2], der_coefB0[2], der_coefx2[2], der_coefB2[2];
  scalar_times_2vec(-6*mu2 + 6*mu, x0, der_coefx0);
  scalar_times_2vec(-3*mu2 + 4*mu - 1, B0, der_coefB0);
  scalar_times_2vec(6*mu2 - 6*mu, x2, der_coefx2);
  scalar_times_2vec(-3*mu2 + 2*mu, B2,  der_coefB2);
  // second time gathering terms
  double coefx0[2], coefB0[2], coefx2[2], coefB2[2];
  scalar_times_2vec(-2*mu3 + 3*mu2 - 1, x0, coefx0);
  scalar_times_2vec(-mu3 + 2*mu2 - mu, B0, coefB0);
  scalar_times_2vec(2*mu3 - 3*mu2, x2, coefx2);
  scalar_times_2vec(-mu3 + mu2, B2, coefB2);
  // third time gathering terms
  double der_sum1[2], der_sum2[2], derMuxMu[2], sum4[2], sum5[2], sum6[2], xLamMinxHat[2], normxLamMinxHat;
  vec2_addition(der_coefx0, der_coefB0, der_sum1);
  vec2_addition(der_coefx2,  der_coefB2, der_sum2);
  vec2_addition(der_sum1, der_sum2, derMuxMu);
  vec2_addition(xHat, coefx0, sum4);
  vec2_addition(coefB0, coefx2, sum5);
  vec2_addition(sum5, coefB2, sum6);
  vec2_addition(sum4, sum6, xLamMinxHat);
  normxLamMinxHat = l2norm(xLamMinxHat);
  // fourth time gathering terms
  double t1, t2;
  t1 = dotProd( derMuxMu, xLamMinxMu);
  t2 = dotProd( derMuxMu, xLamMinxHat);
  // everything together
  gradient[1] = indexRef_01*t1/normxLamMinxMu + indexRef_02*t2/normxLamMinxHat;
}


double backTr_TwoStep(double alpha0, double d[2], double lambda, double mu, double T0, double grad0[2], double T1, double grad1[2], double x0[2], double x1[2], double x2[2], double xHat[2], double B0[2], double B2[2], double indexRef_01, double indexRef_02) {
  // backtracking to compute a step length in the two step update
  double f_prev, f_cur, alpha;
  int i = 0;
  alpha = alpha0;
  // EVALUATING THE OBJECTIVE FUNCTION
  f_prev = fobjective_TwoStep(lambda, mu, T0, grad0, T1, grad1, x0, x1, x2, xHat, B0, B2, indexRef_01, indexRef_02);
  printf("Objective function before %lf  with lambda %lf  and mu  %lf\n", f_prev, lambda, mu);
  f_cur = fobjective_TwoStep(lambda - alpha*d[0], mu - alpha*d[1], T0, grad0, T1, grad1, x0, x1, x2, xHat, B0, B2, indexRef_01, indexRef_02);
  while(f_prev <= f_cur & i < 10 ){
    alpha = alpha*0.5;
    f_cur = fobjective_TwoStep(lambda - alpha*d[0], mu - alpha*d[1], T0, grad0, T1, grad1, x0, x1, x2, xHat, B0, B2, indexRef_01, indexRef_02);
    i ++;
  }
  if (f_prev <= f_cur){
    alpha = 0;
  }
  printf("Objective function adter back tracking  %lf\n", fobjective_TwoStep(lambda - alpha*d[0], mu - alpha*d[1], T0, grad0, T1, grad1, x0, x1, x2, xHat, B0, B2, indexRef_01, indexRef_02) );
  printf("with lambda %lf  and mu %lf\n", lambda-alpha*d[0], lambda-alpha*d[1]);
  return alpha;
}

double fobjective_TwoStep(double lambda, double mu, double T0, double grad0[2], double T1, double grad1[2], double x0[2], double x1[2], double x2[2], double xHat[2], double B0[2], double B2[2], double indexRef_01, double indexRef_02){
  // objective function for a two step update
  double lambda2, lambda3, mu2, mu3;
  lambda2 = lambda*lambda;
  lambda3 = lambda2*lambda;
  mu2 = mu*mu;
  mu3 = mu2*mu;
  // firt time gathering terms
  double x1Minx0[2], coefgrad0[2], coefgrad1[2], sumgrads[2], dotProd1;
  vec2_subtraction(x1, x0, x1Minx0);
  scalar_times_2vec(lambda3 - 2*lambda2 + lambda, grad0, coefgrad0);
  scalar_times_2vec(lambda3 - lambda2, grad1, coefgrad1);
  vec2_addition(coefgrad0, coefgrad1, sumgrads);
  dotProd1 = dotProd(x1Minx0, sumgrads);
  // second time gathering terms
  double x0InxLamMinxMu[2], x1inxLamMinxMu[2], B0inxLamMinxMu[2], x2InxLamMinxMu[2], B2inxLamMinxMu[2];
  scalar_times_2vec(-lambda-2*mu3 + 3*mu2, x0, x0InxLamMinxMu);
  scalar_times_2vec(lambda, x1, x1inxLamMinxMu);
  scalar_times_2vec(-mu3 + 2*mu2 - mu, B0, B0inxLamMinxMu);
  scalar_times_2vec(2*mu3 - 3*mu2, x2, x2InxLamMinxMu);
  scalar_times_2vec(-mu3 + mu2, B2, B2inxLamMinxMu);
  // third time gatherieng terms
  double sum1[2], sum2[2], sum3[2], xLamMinxMu[2], t, normxLamMinxMu, dot1;
  vec2_addition(x0InxLamMinxMu, x1inxLamMinxMu, sum1);
  vec2_addition(B0inxLamMinxMu, x2InxLamMinxMu, sum2);
  vec2_addition(sum2, B2inxLamMinxMu, sum3);
  vec2_addition(sum1, sum3, xLamMinxMu);
  normxLamMinxMu = l2norm(xLamMinxMu);
  // fourth time gathering terms
  double coefx0[2], coefB0[2], coefx2[2], coefB2[2];
  scalar_times_2vec(-2*mu3 + 3*mu2 - 1, x0, coefx0);
  scalar_times_2vec(-mu3 + 2*mu2 - mu, B0, coefB0);
  scalar_times_2vec(2*mu3 - 3*mu2, x2, coefx2);
  scalar_times_2vec(-mu3 + mu2, B2, coefB2);
  // fifth time gathering terms
  double sum4[2], sum5[2], sum6[2], xLamMinxHat[2], normxLamMinxHat;
  vec2_addition(xHat, coefx0, sum4);
  vec2_addition(coefB0, coefx2, sum5);
  vec2_addition(sum5, coefB2, sum6);
  vec2_addition(sum4, sum6, xLamMinxHat);
  normxLamMinxHat = l2norm(xLamMinxHat);
  
  return (2*lambda3 - 3*lambda2 + 1)*T0 + (-2*lambda3 + 3*lambda2)*T1 + dotProd1 + indexRef_01*normxLamMinxMu + indexRef_02*normxLamMinxHat;
  
}


void projectedGradient_TwoStep(double optimizers[2], double lambdaMin, double lambdaMax, double muMin, double muMax, double T0, double grad0[2], double T1, double grad1[2], double x0[2], double x1[2], double x2[2], double xHat[2], double B0[2], double B2[2], double indexRef_01, double indexRef_02, double tol, int maxIter) {
  // projected gradient descent for a two step update (when the index of refraction changes)
  double grad_cur[2], grad_prev[2], step, alpha, optimizers_prev[2], optimizers_cur[2], test[2], difStep[2];
  int i = 1;
  alpha = 0.1;
  optimizers[0] = lambdaMax;
  optimizers[1] = muMax;
  // compute the gradient
  grad_twoStep(grad_cur, optimizers[0], optimizers[1], T0, grad0, T1, grad1, x0, x1, x2, xHat, B0, B2, indexRef_01, indexRef_02);
  grad_prev[0] = grad_cur[0];
  grad_prev[1] = grad_cur[1];
  if (l2norm(grad_cur) > tol){
    test[0] = optimizers[0] - alpha*grad_cur[0];
    test[1] = optimizers[1] - alpha*grad_cur[1];
  }
  else{
    test[0] = optimizers[0];
    test[1] = optimizers[1];
  }
  if( test[0] > lambdaMax){
    test[0] = lambdaMax;
  }
  if(test[1] > muMax){
    test[1] = muMax;
  }
  if(test[0] < lambdaMin){
    test[0] = lambdaMin;
  }
  if(test[1] < muMin){
    test[1] = muMin;
  }
  // start the iteration
  optimizers_cur[0] = test[0];
  optimizers_cur[1] = test[1];
  grad_twoStep(grad_cur, optimizers_cur[0], optimizers_cur[1], T0, grad0, T1, grad1, x0, x1, x2, xHat, B0, B2, indexRef_01, indexRef_02);
  vec2_subtraction(optimizers_prev, optimizers_cur, difStep);
  while(i < maxIter & l2norm(grad_cur) > tol & l2norm(difStep) > 0){
    printf("\n\nIteration %d\n", i);
    alpha = backTr_TwoStep(0.1, grad_cur, optimizers_cur[0], optimizers_cur[1], T0, grad0, T1, grad1, x0, x1, x2, xHat, B0, B2, indexRef_01, indexRef_02);
    printf("Step size %lf  with direction  %lf  %lf, hence the change is  %lf   %lf\n", alpha, -grad_cur[0], -grad_cur[1], -alpha*grad_cur[0], - alpha*grad_cur[1]);
    test[0] = optimizers_cur[0] - alpha*grad_cur[0];
    test[1] = optimizers_cur[1] - alpha*grad_cur[1];
    printf("Values before projecting back  %lf   %lf\n", test[0], test[1]);
    // project back if neccesary
    if( test[0] > lambdaMax){
      test[0] = lambdaMax;
    }
    if(test[1] > muMax){
      test[1] = muMax;
    }
    if(test[0] < lambdaMin){
      test[0] = lambdaMin;
    }
    if(test[1] < muMin){
      test[1] = muMin;
    }
    // if there is no better function value don't update
    if( fobjective_TwoStep(optimizers_cur[0], optimizers_cur[1], T0, grad0, T1, grad1, x0, x1, x2, xHat, B0, B2, indexRef_01, indexRef_02) < fobjective_TwoStep(test[0], test[1], T0, grad0, T1, grad1, x0, x1, x2, xHat, B0, B2, indexRef_01, indexRef_02) ) {
      test[0] = optimizers_cur[0];
      test[1] = optimizers_cur[1];
    }
    // update
    grad_prev[0] = grad_cur[0];
    grad_prev[1] = grad_cur[1];
    optimizers_prev[0] = optimizers_cur[0];
    optimizers_prev[1] = optimizers_cur[1];
    optimizers_cur[0] = test[0];
    optimizers_cur[1] = test[1];
    vec2_subtraction(optimizers_prev, optimizers_cur, difStep);
    grad_twoStep(grad_cur, optimizers_cur[0], optimizers_cur[1], T0, grad0, T1, grad1, x0, x1, x2, xHat, B0, B2, indexRef_01, indexRef_02);
    printf("Iteration %d  with lambda_prev   %lf   and lambda_cur  %lf\n", i, optimizers_prev[0], optimizers_cur[0]);
    printf("Iteration %d  with mu_prev   %lf   and mu_cur  %lf\n", i, optimizers_prev[1], optimizers_cur[1]);
    printf("Objective value   %lf    and gradient %lf  %lf \n", fobjective_TwoStep(optimizers_cur[0], optimizers_cur[1], T0, grad0, T1, grad1, x0, x1, x2, xHat, B0, B2, indexRef_01, indexRef_02), grad_cur[0], grad_cur[1]);
    i ++;
  }
  optimizers[0] = optimizers_cur[0];
  optimizers[1] = optimizers_cur[1];
}


