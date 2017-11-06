/*
 * ShellFunctions.h
 * Author: Wyatt Emery
 * Date: NOV 07, 2016
 *
 * COSC 3750, Homework9
 *
 * Description
 */


#define STDBUFF 4096
typedef struct node Node;
typedef struct word Word;


#include<stdlib.h>


#ifndef SHELL_FUNCTIONS
#define SHELL_FUNCTIONS

struct node
{
    Node* next;
    Node* prev;
    char* command;
    Word* arguments;
    int inRedir;
    int outRedir;
    int errRedir;
    char* inFile;
    char* outFile;
    char* errFile;
};

struct word
{
    char* data;
    Word* next;
    Word* prev;
};

extern Node* newNode(Node* prevNode);

extern Word* newWord(Word* prevWord);

extern void deleteNodes(Node* startNode);

extern void printCommands(Node* startNode, short BGFlag);

extern void printArgs(Word* startWord);

extern void printRedirSyms(Node* currentNode);

extern Word* getLastWord(Word* startWord);

#endif