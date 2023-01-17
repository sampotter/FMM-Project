// testing the marcher with hermite interpolation for T(xLambda)

#include "eik_grid.h"
#include "updates_2D.h"
#include "opti_method.h"

#include <math.h>
#include <stdio.h>
#include <time.h>


int main()
{
    // TEST GEOMETRY FOR DIFFERENT INDICES OF REFRACTION - SOME PATHS GO AROUND, NOT INTO REG3



  const char *pathPoints, *pathNeighbors, *pathIncidentFaces, *pathFaces, *pathIndexRegions, *pathToSaveTr_, *pathSaveGradientsTr_, *path_BoundaryTan, *path_BoundaryChain;
    const char *pathSavePath, *pathSaveLambdas, *pathTimes;
    pathPoints = "/Users/marianamartinez/Documents/NYU-Courant/FMM-Project/TestBaseSnow/H0/H0_MeshPoints.txt";
    pathNeighbors = "/Users/marianamartinez/Documents/NYU-Courant/FMM-Project/TestBaseSnow/H0/H0_Neigh.txt";
    pathIncidentFaces = "/Users/marianamartinez/Documents/NYU-Courant/FMM-Project/TestBaseSnow/H0/H0_IncidentFaces.txt";
    pathFaces = "/Users/marianamartinez/Documents/NYU-Courant/FMM-Project/TestBaseSnow/H0/H0_Faces.txt";
    pathIndexRegions = "/Users/marianamartinez/Documents/NYU-Courant/FMM-Project/TestBaseSnow/H0/H0_FacesLabel.txt";

    pathToSaveTr_ = "/Users/marianamartinez/Documents/NYU-Courant/FMM-Project/TestBaseSnow/H0/H0_ComputedValuesCubic.bin";
    pathSaveGradientsTr_ = "/Users/marianamartinez/Documents/NYU-Courant/FMM-Project/TestBaseSnow/H0/H0_ComputedGradientsCubic.bin";
    path_BoundaryTan = "/Users/marianamartinez/Documents/NYU-Courant/FMM-Project/TestBaseSnow/H0/H0_Boundary_tan.txt";
    path_BoundaryChain = "/Users/marianamartinez/Documents/NYU-Courant/FMM-Project/TestBaseSnow/H0/H0_Boundary_chain.txt";
    pathSavePath = "/Users/marianamartinez/Documents/NYU-Courant/FMM-Project/TestBaseSnow/H0/H0_ParentsCubic.bin";
    pathSaveLambdas = "/Users/marianamartinez/Documents/NYU-Courant/FMM-Project/TestBaseSnow/H0/H0_LambdasOptCubic.bin";
    pathTimes = "/Users/marianamartinez/Documents/NYU-Courant/FMM-Project/TestBaseSnow/H0/H0_TimesCubic.bin";

    int *start;
    int nstart, s;

    s = 0;
    start = &s;
    nstart = 1;
    // now we test the init with just the path to the files

    printf("\n------------------------------------");
    printf("\n------------------------------------");
    printf("\n------------------------------------");
    printf("\n\n\n TESTING THE UPDATES WITH CURVY TRIANGLES \n\n\n\n");
    eik_gridS *eik_g1;
    eik_grid_alloc(&eik_g1);
    eik_grid_initFromFile(eik_g1, start, nstart, pathPoints, pathNeighbors, pathIncidentFaces, pathFaces, pathIndexRegions, path_BoundaryTan, path_BoundaryChain);

    printAllInfoMesh(eik_g1);


    info_updateS *info_update;
    info_update_alloc(&info_update);

    /* int indexAccepted, x1_ind, x2_ind, xHat_ind; */
    /* double T0, T1, indexRef_01, indexRef_02; */

    /* indexAccepted = 48; */
    /* xHat_ind = 49; */
    /* T0 = 14.8753; */
    /* indexRef_01 = regionBetweenTwoPoints(eik_g1->triM_2D, indexAccepted, x1_ind); */

    /* printf("\n\nTESTING CREEPING UPDATE\n\n"); */

    /* info_update_initCr(info_update, indexAccepted, xHat_ind, T0, indexRef_01); */
    
    /* print_info_update(info_update); */

    /* creepingUpdate(eik_g1->triM_2D, info_update); */

    /* print_info_update(info_update); */

    /* printf("\n\nTESTING FREE SPACE TWO POINT UPDATE\n\n"); */

    /* int indexAccepted, x1_ind, xHat_ind; */
    /* double T0, T1, grad0[2], grad1[2], indexRef; */

    /* grad0[0] = -0.228191; */
    /* grad0[1] = -0.973616; */
    /* grad1[0] = -0.409301; */
    /* grad1[1] = -0.9124; */
    /* indexAccepted = 3128; */
    /* x1_ind = 2942; */
    /* xHat_ind = 127; */
    /* T0 = 8.21679; */
    /* T1 = 7.32957; */
    /* indexRef = 1.0; */

    /* printf("T1: %lf\n", T1); */
    /* info_update_initTwo(info_update, indexAccepted, x1_ind, xHat_ind, T0, grad0, T1, grad1, indexRef); */

    /* print_info_update(info_update); */

    /* simple_TwoPointUpdate(eik_g1->triM_2D, info_update); */

    /* print_info_update(info_update); */


    printf("\n\nTESTING UPDATE FROM EDGE\n\n");

    double lambda0, T0, grad0[2], B0[2], T1, grad1[2], B1[2], x0[2], x1[2], xHat[2], indexRef, lam_opti, fObj;

    lambda0 = 0.5;
    T0 = 0.70710678;
    T1 = 0.70710678;
    x0[0] = -1;
    x0[1] = 1;
    grad0[0] = -0.70710678;
    grad0[1] = 0.70710678;
    x1[0] = 1;
    x1[1] = 1;
    grad1[0] = 0.70710678;
    grad1[1] = 0.70710678;
    B0[0] = 1;
    B0[1] = 0;
    B1[0] = 1;
    B1[1] = 0;
    xHat[0] = 0;
    xHat[1] = 5;
    indexRef = 1;

    fObj = fobjective_fromEdge(0.5, T0, grad0, B0, T1, grad1, B1, x0, x1, xHat, indexRef);
    der_fromEdge(0.5, T0, grad0, B0, T1, grad1, B1, x0, x1, xHat, indexRef);

    printf("Starting with objetive value %lf    and derivative  %lf\n", fObj, fObj);

    lam_opti = projectedGradient_fromEdge(lambda0, T0, grad0, B0, T1, grad1, B1, x0, x1, xHat, 0.0001, 50, indexRef);

    printf("Optimum lambda %lf", lam_opti);
    
    
    


    eik_grid_dealloc(&eik_g1);



}