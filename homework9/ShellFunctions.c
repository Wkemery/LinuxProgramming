/*
 * ShellFunctions.c
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


#include<stdlib.h>
#include<stdio.h>
#include"ShellFunctions.h"
#include"wyscanner.h"

Node* newNode(Node* prevNode)
{
    Node* ret = (Node*)malloc(sizeof(Node));
    if(ret == NULL)
    {
        perror("Malloc Error Occured");
        return NULL;
    }
    ret->next = NULL;
    ret->prev = prevNode;
    ret->command = NULL;
    ret->arguments = NULL;
    ret->inRedir = 0;
    ret->outRedir = 0;
    ret->errRedir = 0;
    ret->inFile = NULL;
    ret->outFile = NULL;
    ret->errFile = NULL;

    return ret;
}

Word* newWord(Word* prevWord)
{
    Word* ret = (Word*) malloc(sizeof(Word));
    if(ret == NULL)
    {
        perror("Malloc Error Occured");
        return NULL;
    }
    ret->next = NULL;
    ret->prev = prevWord;
    ret->data = NULL;

    return ret;
}

void deleteNodes(Node* startNode)
{
    if(startNode != NULL)
    {
        deleteNodes(startNode->next);
        free(startNode);
    }
}

void printCommands(Node* startNode, short BGFlag)
{
    Node* currentNode = startNode;
    while((currentNode != NULL) && (currentNode->command != NULL))
    {
        printf(":--: %s\n", currentNode->command);
        printArgs(currentNode->arguments);
        printRedirSyms(currentNode);
        if(BGFlag) printf("&\n");
        printf("EOL\n");
        currentNode = currentNode->next;
    }
}

void printArgs(Word* startWord)
{
    Word* currentWord = startWord;
    while(currentWord != NULL)
    {
        printf(" --: %s\n", currentWord->data);
        currentWord = currentWord->next;
    }
}

void printRedirSyms(Node* currentNode)
{
    if(currentNode->errRedir != 0)
    {
        //do later
    }

    if(currentNode->inRedir != 0)
    {
        if(currentNode->inRedir == REDIR_IN)
        {
            printf("<\n");
            printf(" --: %s\n", currentNode->inFile);
        }
    }
    if(currentNode->outRedir != 0)
    {
        if(currentNode->outRedir == REDIR_OUT)
        {
            printf(">\n");
            printf(" --: %s\n", currentNode->outFile);
        }
        else
        {
            printf("|\n");
            return;
        }
    }
}


Word* getLastWord(Word* startWord)
{
    Word* currentWord = startWord;
    Word* ret = NULL;
    while(currentWord != NULL)
    {
        ret = currentWord;
        currentWord = currentWord->next;
    }
    return ret;
}


