/*
* mmult.c
* Author: Wyatt Emery
* Date: DEC 3, 2016
*
* COSC 3750, Homework 12
*
* Description
*/

#define SIZE 4096
#define true 1
#define false 0
typedef short BOOL;

#include<stdio.h>
#include<errno.h>

int main (int argc, char** argv)
{
    if(argc == 4)
    {
        char* matrixAName = argv[1];
        char* matrixBName = argv[2];
        char* outMatrixName = argv[3];
        int ret;

        /**Perform Matrix multiply without threads*/

        /*Open matrix files*/
        FILE* matrixA = fopen(matrixAName, "r");
        if(matrixA == NULL)
        {
            perror(matrixAName);
            return 1;
        }

        FILE* matrixB = fopen(matrixBName, "r");
        if(matrixB == NULL)
        {
            perror(matrixBName);
            return 1;
        }

        FILE* outMatrix = fopen(outMatrixName, "w");
        if(outMatrix == NULL)
        {
            perror(outMatrixName);
            return 1;
        }


        /*Read in matrix A and matrix B sizes*/
        int Arows = 0;
        int Acolumns = 0;
        int Brows = 0;
        int Bcolumns = 0;

        ret = fread(&Arows, 1, 4, matrixA);
        if(ret == -1)
        {
            perror("fread");
            return 1;
        }
        printf("%d\n", Arows);

        ret = fread(&Acolumns, 1, 4, matrixA);
        if(ret == -1)
        {
            perror("fread");
            return 1;
        }
        printf("%d\n", Acolumns);

        ret = fread(&Brows, 1, 4, matrixB);
        if(ret == -1)
        {
            perror("fread");
            return 1;
        }
        printf("%d\n", Brows);

        ret = fread(&Bcolumns, 1, 4, matrixB);
        if(ret == -1)
        {
            perror("fread");
            return 1;
        }
        printf("%d\n", Bcolumns);

        /*calculate size of final matrix*/






        /*Validate multiply*/
        if(Acolumns != Brows)
        {
            fprintf(stderr, "Error: columns in %s must be equal to rows in "
                    "%s\n", matrixAName, matrixBName);
            return 1;
        }

    }
    else if(argc == 5)
    {

    }
    else
    {
        fprintf(stderr, "Usage: [numThreads] matrix1 matrix2 outputFile\n");
    }


    return 0;
}
