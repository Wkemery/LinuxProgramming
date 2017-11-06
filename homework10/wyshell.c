/*
 * wyshell.c
 * Author: Wyatt Emery
 * Date: NOV 15, 2016
 *
 * COSC 3750, Homework9
 *
 * Part 1 of the program parses command line input and detects errors.
 * If an error is encountered, the program discards the line of input.
 * The parsed commands are printed out after processing them all. As a result,
 * if there is an error, nothing is printed out except the error before
 * discarding the line of input.
 */



#define STDBUFF 4096
#define true 1
#define false 0
typedef short BOOL;
typedef struct node Node;
typedef struct word Word;
/*****************************************************************************/

#include<stdio.h>
#include<errno.h>
#include"wyscanner.h"
#include"ShellFunctions.h"
#include<stdlib.h>
#include<string.h>

int main (int argc, char** argv)
{
    char* lineptr = NULL;
    size_t n = 0;
    Node* startNode = NULL;
    PIDNode* childPIDs = NULL;
    printf("$> ");


    while((getline(&lineptr, &n, stdin)) != -1)
    {


        startNode = parseInput(lineptr);

        if(startNode!= NULL)
        {
            //printCommands(startNode, runBG);
            execAllCommands(startNode, &childPIDs);
        }

        free(lineptr);
        lineptr = NULL;
        deleteNodes(startNode);
        printf("$> ");
        if(childPIDs != NULL)
            waitOnChildren(childPIDs);
    }

    if(errno != 0)
    {
        perror("Error reading from stdin");
    }

    printf("\n");

    return 0;
}
