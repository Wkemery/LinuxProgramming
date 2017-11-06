/*
 * wyshell.c
 * Author: Wyatt Emery
 * Date: NOV 29, 2016
 *
 * COSC 3750, Homework11
 *
 * This is the final iteration of the wyshell program. 
 * It parses command line input and detects errors. It adds redirection 
 * functionality. if an & occurs at the end of a string of commands, all 
 * previous commands are run in the background.
 * All memory is freed by use of deleteNodes and waitOnChildren.
 * If an error is encountered, the program discards the line of
 * input. All commands are executed in the order that they are entered in.
 * Zombies are purged after execution of all commands and just after/before the
 * next $> is printed out to stdout. Pipes are fully functional. the order of 
 * redirection is taken into accout for redirecting stderr to stdout. 
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
            execAllCommands(startNode, &childPIDs);
        }

        free(lineptr);
        lineptr = NULL;
        deleteNodes(startNode);
        printf("$> ");
        if(childPIDs != NULL)
            childPIDs = waitOnChildren(childPIDs);
    }

    if(errno != 0)
    {
        perror("Error reading from stdin");
    }

    printf("\n");

    return 0;
}
