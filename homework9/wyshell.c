/*
 * wyshell.c
 * Author: Wyatt Emery
 * Date: NOV 07, 2016
 *
 * COSC 3750, Homework9
 *
 * Description
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

    printf("$> ");


    while((getline(&lineptr, &n, stdin)) != -1)
    {
        Node* startNode = newNode(NULL);
        Node* currentNode = startNode;
        BOOL firstScan = true;
        BOOL scanning = true;
        BOOL inputRedirect = false;
        BOOL outputRedirect = false;
        //BOOL newCommand = false;
        BOOL runBG = false;
        //BOOL pipeFlag = false;
        int token;

        //only ways to start new command on same line is | or ;
        while(scanning)
        {
            Node* nextNode = NULL;
            if(firstScan)
            {
                firstScan = false;
                token = parse_line(lineptr);
            }
            else token = parse_line(NULL);

            switch (token)
            {
                case QUOTE_ERROR:
                    fprintf(stderr, "Quote Error Encountered\n");
                    scanning = false;
                    deleteNodes(startNode);
                    startNode = NULL;
                    break;
                /*************************************************************/
                case ERROR_CHAR:
                    fprintf(stderr, "Invalid Character Encountered%c\n", error_char);
                    scanning = false;
                    deleteNodes(startNode);
                    startNode = NULL;
                    break;
                /*************************************************************/
                case SYSTEM_ERROR:
                    perror("System Error");
                    deleteNodes(startNode);
                    startNode = NULL;
                    return 1;
                /*************************************************************/
                case REDIR_OUT:
                    if(currentNode->command == NULL)
                    {
                        fprintf(stderr, "Ivalid null command\n");
                        deleteNodes(startNode);
                        startNode = NULL;
                        break;
                    }
                    if(currentNode->outRedir != 0)
                    {
                        fprintf(stderr, "Ambigous Output redirection\n");
                        deleteNodes(startNode);
                        startNode = NULL;
                        break;
                    }
                    /*Redirect output to file*/
                    currentNode->outRedir = REDIR_OUT;
                    outputRedirect = true;
                    break;
                /*************************************************************/
                case REDIR_IN:
                    if(currentNode->command == NULL)
                    {
                        fprintf(stderr, "Invalid null command\n");
                        deleteNodes(startNode);
                        startNode = NULL;
                        break;
                    }
                    if(currentNode->inRedir != 0)
                    {
                        fprintf(stderr, "Ambigous Input redirection\n");
                        deleteNodes(startNode);
                        startNode = NULL;
                        break;
                    }
                    /*Redirect input from file*/
                    currentNode->inRedir = REDIR_IN;
                    inputRedirect = true;
                    break;
                /*************************************************************/
                case APPEND_OUT:
                    break;
                /*************************************************************/
                case REDIR_ERR:
                    break;
                /*************************************************************/
                case APPEND_ERR:
                    break;
                /*************************************************************/
                case REDIR_ERR_OUT:
                    break;
                /*************************************************************/
                case SEMICOLON:
                    /*create next command*/
                    nextNode = newNode(currentNode);
                    currentNode->next = nextNode;
                    currentNode = nextNode;
                    //newCommand = true;
                    break;
                /*************************************************************/
                case PIPE:
                    if(currentNode->command == NULL) 
                    {
                        fprintf(stderr, "Invalid null command\n");
                        deleteNodes(startNode);
                        startNode = NULL;
                        break;
                    }
                    if(currentNode->outRedir != 0)
                    {
                        fprintf(stderr, "Ambigous output redirection\n");
                        deleteNodes(startNode);
                        startNode = NULL;
                        break;
                    }
                    /*create next command*/
                    nextNode = newNode(currentNode);
                    currentNode->next = nextNode;
                    /*redirect output of curr command to input of next command*/
                    currentNode->outRedir = PIPE;
                    nextNode->inRedir = PIPE;
                    //newCommand = true;

                    currentNode = nextNode;
                    break;
                /*************************************************************/
                case AMP:
                    runBG = true;
                    //ampersand must be at end of command
                    break;
                /*************************************************************/
                case WORD:
                    if(outputRedirect)
                    {
                        int len = strlen(lexeme) + 1;
                        currentNode->outFile = (char*) malloc(len);
                        if(currentNode->outFile == NULL)
                        {
                            perror("Malloc Error Occured");
                            return 1;
                        }
                        memset(currentNode->outFile, 0, len);
                        strncpy(currentNode->outFile, lexeme, len);
                        outputRedirect = false;
                        break;
                    }
                    if(inputRedirect)
                    {
                        int len = strlen(lexeme) + 1;
                        currentNode->inFile = (char*) malloc(len);
                        if(currentNode->inFile == NULL)
                        {
                            perror("Malloc Error Occured");
                            return 1;
                        }
                        memset(currentNode->inFile, 0, len);
                        strncpy(currentNode->inFile, lexeme, len);
                        inputRedirect = false;
                        break;
                    }
                    //everything should still work if I comment out this part and get rid of newcommand var alltogether
                    /*if(newCommand)
                    {
                        int len = strlen(lexeme) + 1;
                        currentNode->command = (char*) malloc(len);
                        if(currentNode->command == NULL)
                        {
                            perror("Malloc Error Occured");
                            return 1;
                        }
                        memset(currentNode->command, 0, len);
                        strncpy(currentNode->command, lexeme, len);
                        newCommand = false;
                        break;
                    }*/
                    if(currentNode->command == NULL)
                    {
                        int len = strlen(lexeme) + 1;
                        currentNode->command = (char*) malloc(len);
                        if(currentNode->command == NULL)
                        {
                            perror("Malloc Error Occured");
                            return 1;
                        }
                        memset(currentNode->command, 0, len);
                        strncpy(currentNode->command, lexeme, len);
                        break;
                    }
                    if(currentNode->command != NULL)
                    {
                        Word* newArg = NULL;
                        if(currentNode->arguments == NULL)
                        {
                            currentNode->arguments = newWord(NULL);
                            newArg = currentNode->arguments;
                        }
                        else
                        {
                            Word* lastArg = getLastWord(currentNode->arguments);
                            newArg = newWord(lastArg);
                            lastArg->next = newArg;
                        }
                        int len = strlen(lexeme) + 1;
                        newArg->data = (char*) malloc(len);
                        if(newArg->data == NULL)
                        {
                            perror("Malloc Error Occured");
                            return 1;
                        }
                        memset(newArg->data, 0, len);
                        strncpy(newArg->data, lexeme, len);

                        break;
                    }
                /*************************************************************/
                default:
                    if(currentNode->inRedir != 0)
                    {
                        //and infile === null error
                    }
                    scanning = false;
            }

        }
        printCommands(startNode, runBG);
        free(lineptr);
        lineptr = NULL;
        printf("$> ");
    }

    if(errno != 0)
    {
        perror("Error reading from stdin");
    }



    return 0;
}
