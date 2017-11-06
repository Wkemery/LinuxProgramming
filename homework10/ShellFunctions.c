/*
 * ShellFunctions.c
 * Author: Wyatt Emery
 * Date: NOV 15, 2016
 *
 * COSC 3750, Homework9
 *
 * Functions to accompany wyshell.c
 */



#define STDBUFF 4096
#define true 1
#define false 0
typedef short BOOL;

typedef struct node Node;
typedef struct word Word;
typedef struct pidNode PIDNode;

#include<stdlib.h>
#include<stdio.h>
#include<string.h>
#include<sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include<errno.h>


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
    ret->runBG = 0;
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

PIDNode* newPIDNode(PIDNode* prevPIDNode)
{
    PIDNode* ret = (PIDNode*) malloc(sizeof(PIDNode));
    if(ret == NULL)
    {
        perror("Malloc Error Occured");
        return NULL;
    }
    ret->next = NULL;
    ret->prev = prevPIDNode;
    ret->pid = 0;

    return ret;
}


Node* parseInput(char* lineptr)
{
    Node* startNode = newNode(NULL);
    Node* currentNode = startNode;
    
    BOOL scanning = true;
    BOOL inputRedirect = false;
    BOOL outputRedirect = false;
    BOOL errorRedirect = false;
    BOOL firstScan = true;
    
    int lastToken;
    int token;

    /*only ways to start new command on same line is | or ;*/
    while(scanning)
    {
        Node* nextNode = NULL;
        if(firstScan)
        {
            firstScan = false;
            token = parse_line(lineptr);
        }
        else token = parse_line(NULL);
        
        
        /*ampersand must be at end of command*/
        if((token == EOL) && (lastToken == AMP))
        {
            setAllBG(startNode);
            break;
        }
        
        switch (token)
        {
            case QUOTE_ERROR:
                fprintf(stderr, "Quote Error Encountered\n");
                scanning = false;
                deleteNodes(startNode);
                startNode = NULL;
                return NULL;
                /*************************************************************/
            case ERROR_CHAR:
                fprintf(stderr, "Invalid Character Encountered%c\n",
                        error_char);
                scanning = false;
                deleteNodes(startNode);
                startNode = NULL;
                return NULL;
                /*************************************************************/
            case SYSTEM_ERROR:
                perror("System Error");
                deleteNodes(startNode);
                startNode = NULL;
                exit(1);
                /*************************************************************/
            case REDIR_OUT:
                if(currentNode->command == NULL)
                {
                    fprintf(stderr, "Ivalid null command\n");
                    deleteNodes(startNode);
                    startNode = NULL;
                    return NULL;
                }
                if(currentNode->outRedir != 0)
                {
                    fprintf(stderr, "Ambigous Output redirection\n");
                    deleteNodes(startNode);
                    startNode = NULL;
                    return NULL;
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
                    return NULL;
                }
                if(currentNode->inRedir != 0)
                {
                    fprintf(stderr, "Ambigous Input redirection\n");
                    deleteNodes(startNode);
                    startNode = NULL;
                    return NULL;
                }
                /*Redirect input from file*/
                currentNode->inRedir = REDIR_IN;
                inputRedirect = true;
                break;
                /*************************************************************/
            case APPEND_OUT:
                if(currentNode->command == NULL)
                {
                    fprintf(stderr, "Ivalid null command\n");
                    deleteNodes(startNode);
                    startNode = NULL;
                    return NULL;
                }
                if(currentNode->outRedir != 0)
                {
                    fprintf(stderr, "Ambigous Output redirection\n");
                    deleteNodes(startNode);
                    startNode = NULL;
                    return NULL;
                }
                /*Redirect output to file but for appending*/
                currentNode->outRedir = APPEND_OUT;
                outputRedirect = true;
                break;
                /*************************************************************/
            case REDIR_ERR:
                if(currentNode->command == NULL)
                {
                    fprintf(stderr, "Ivalid null command\n");
                    deleteNodes(startNode);
                    startNode = NULL;
                    return NULL;
                }
                if(currentNode->errRedir != 0)
                {
                    fprintf(stderr, "Ambigous Error redirection\n");
                    deleteNodes(startNode);
                    startNode = NULL;
                    return NULL;
                }
                /*Redirect sdterr to file*/
                currentNode->errRedir = REDIR_ERR;
                errorRedirect = true;
                break;
                /*************************************************************/
            case APPEND_ERR:
                if(currentNode->command == NULL)
                {
                    fprintf(stderr, "Ivalid null command\n");
                    deleteNodes(startNode);
                    startNode = NULL;
                    return NULL;
                }
                if(currentNode->errRedir != 0)
                {
                    fprintf(stderr, "Ambigous Error redirection\n");
                    deleteNodes(startNode);
                    startNode = NULL;
                    return NULL;
                }
                /*Redirect stderr to file to be appended*/
                currentNode->errRedir = APPEND_ERR;
                errorRedirect = true;
                break;
                /*************************************************************/
            case REDIR_ERR_OUT:
                if(currentNode->command == NULL)
                {
                    fprintf(stderr, "Ivalid null command\n");
                    deleteNodes(startNode);
                    startNode = NULL;
                    return NULL;
                }
                if(currentNode->errRedir != 0)
                {
                    fprintf(stderr, "Ambigous Error redirection\n");
                    deleteNodes(startNode);
                    startNode = NULL;
                    return NULL;
                }
                /*redirect stderr to stdout*/
                currentNode->errRedir = REDIR_ERR_OUT;
                if(currentNode->outRedir != 0)
                {
                    int len = strlen(currentNode->outFile) + 1;
                    currentNode->errFile = (char*) malloc(len);
                    if(currentNode->errFile == NULL)
                    {
                        perror("Malloc Error Occured");
                        exit(1);
                    }
                    memset(currentNode->errFile, 0, len);
                    strncpy(currentNode->errFile, currentNode->outFile, len);
                }
                break;
                /*************************************************************/
            case SEMICOLON:
                /*create next command*/
                nextNode = newNode(currentNode);
                currentNode->next = nextNode;
                currentNode = nextNode;
                break;
                /*************************************************************/
            case PIPE:
                if(currentNode->command == NULL)
                {
                    fprintf(stderr, "Invalid null command\n");
                    deleteNodes(startNode);
                    startNode = NULL;
                    return NULL;
                }
                if(currentNode->outRedir != 0)
                {
                    fprintf(stderr, "Ambigous output redirection\n");
                    deleteNodes(startNode);
                    startNode = NULL;
                    return NULL;
                }
                /*create next command*/
                nextNode = newNode(currentNode);
                currentNode->next = nextNode;
                /*redirect output of curr command to input of next command*/
                currentNode->outRedir = PIPE;
                nextNode->inRedir = PIPE;

                currentNode = nextNode;
                break;
                /*************************************************************/
            case AMP:
                if(currentNode->command == NULL)
                {
                    fprintf(stderr, "Invalid null command\n");
                    deleteNodes(startNode);
                    startNode = NULL;
                    return NULL;
                }
                if(currentNode->runBG == 1)
                {
                    fprintf(stderr, "Misplaced &\n");
                    deleteNodes(startNode);
                    startNode = NULL;
                    return NULL;
                }
                currentNode->runBG = 1;
                break;
                /*************************************************************/
            case WORD:
                if(outputRedirect)
                {
                    /*Current Word is a redirection file for output*/
                    int len = strlen(lexeme) + 1;
                    currentNode->outFile = (char*) malloc(len);
                    if(currentNode->outFile == NULL)
                    {
                        perror("Malloc Error Occured");
                        exit(1);
                    }
                    memset(currentNode->outFile, 0, len);
                    strncpy(currentNode->outFile, lexeme, len);
                    outputRedirect = false;
                    break;
                }
                if(inputRedirect)
                {
                    /*Current Word is a redirection file for input*/
                    int len = strlen(lexeme) + 1;
                    currentNode->inFile = (char*) malloc(len);
                    if(currentNode->inFile == NULL)
                    {
                        perror("Malloc Error Occured");
                        exit(1);
                    }
                    memset(currentNode->inFile, 0, len);
                    strncpy(currentNode->inFile, lexeme, len);
                    inputRedirect = false;
                    break;
                }
                if(errorRedirect)
                {
                    /*Current Word is a redirection file for error*/
                    int len = strlen(lexeme) + 1;
                    currentNode->errFile = (char*) malloc(len);
                    if(currentNode->errFile == NULL)
                    {
                        perror("Malloc Error Occured");
                        exit(1);
                    }
                    memset(currentNode->errFile, 0, len);
                    strncpy(currentNode->errFile, lexeme, len);
                    errorRedirect = false;
                    break;
                }
                if(currentNode->command == NULL)
                {
                    /*Current Word is assumed to be a command*/
                    int len = strlen(lexeme) + 1;
                    currentNode->command = (char*) malloc(len);
                    if(currentNode->command == NULL)
                    {
                        perror("Malloc Error Occured");
                        exit(1);
                    }
                    memset(currentNode->command, 0, len);
                    strncpy(currentNode->command, lexeme, len);
                    break;
                }
                if(currentNode->command != NULL)
                {
                    /*Current Word is assumed to be a an argument*/
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
                        exit(1);
                    }
                    memset(newArg->data, 0, len);
                    strncpy(newArg->data, lexeme, len);
                    break;
                }
                /*************************************************************/
            default:
                /*If input was redirected to anything except PIPE
                    *And no redirection file was specified*/
                if((currentNode->inRedir != 0) &&
                    (currentNode->inRedir != PIPE) &&
                    (currentNode->inFile == NULL))
                {
                    fprintf(stderr, "Missing name for redirect\n");
                    deleteNodes(startNode);
                    startNode = NULL;
                }
                /*If output was redirected to anything except PIPE
                    *And no redirection file was specified*/
                else if((currentNode->outRedir != 0) &&
                    (currentNode->outRedir != PIPE) &&
                    (currentNode->outFile == NULL))
                {
                    fprintf(stderr, "Missing name for redirect\n");
                    deleteNodes(startNode);
                    startNode = NULL;
                }
                /*If error was redirected to anything except REDIR_ERR_OUT
                    *And no redirection file was specified*/
                else if((currentNode->errRedir != 0) &&
                    (currentNode->errRedir != REDIR_ERR_OUT) &&
                    (currentNode->errFile == NULL))
                {
                    fprintf(stderr, "Missing name for redirect\n");
                    deleteNodes(startNode);
                    startNode = NULL;
                }
                scanning = false;
        }
        lastToken = token;
    }
    return startNode;
}

void deleteNodes(Node* startNode)
{
    if(startNode != NULL)
    {
        deleteNodes(startNode->next);
        deleteWords(startNode->arguments);
        free(startNode);
    }
}

void deleteWords(Word* startWord)
{
    if(startWord != NULL)
    {
        deleteWords(startWord->next);
        free(startWord);
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
        currentNode = currentNode->next;
        if((currentNode!= NULL) && (currentNode->inRedir != PIPE))
            printf(";\n");
    }
    printf(" --: EOL\n");
}

void execAllCommands(Node* startNode, PIDNode** childPIDs)
{
    Node* currentNode = startNode;
    while((currentNode != NULL) && (currentNode->command != NULL))
    {
        pid_t childPID = execCommand(currentNode);
        if(childPID == -1)
        {
            return;
        }
        if(currentNode->runBG)
        {
            /*Add child pid to list of PIDs*/
            PIDNode* lastPID = getLastPID(childPIDs[0]);
            PIDNode* newPID = newPIDNode(lastPID);
            newPID->pid = childPID;
            if(childPIDs[0] == NULL)
            {
                childPIDs[0] = newPID;
            }
            lastPID->next = newPID;
        }
        else
        {
            /*Wait on child*/
            pid_t ret = waitpid(childPID, NULL, 0);
            if(ret == -1)
            {
                perror("Error waiting on child");
                exit(1);
            }
        }

        currentNode = currentNode->next;
    }
}

pid_t execCommand(Node* command)
{
    int len = 2;
    char** argList = calloc(len, sizeof(char*));
    argList[0] = strdup(command->command);
    argList[1] = NULL;

    Word* currentWord = command->arguments;
    while(currentWord != NULL)
    {
        len++;
        argList = realloc(argList, sizeof(char*) * len);
        if(argList == NULL)
        {
            perror("realloc() failed");
            return -1;
        }
        argList[len - 2] = strdup(currentWord->data);
        argList[len - 1] = NULL;
        currentWord = currentWord->next;
    }

    /*Fork Time*/
    pid_t childPID = fork();
    if(childPID == -1)
    {
        perror("Fork Error!");
        exit(1);
    }

    if(childPID == 0) //This is the Child
    {
        int ret = execvp(command->command, argList);
        if(ret == -1)
        {       
            perror(command->command);
            exit(1);
        }
    }

    return childPID;
}

void waitOnChildren(PIDNode* childPIDs)
{
    pid_t pid = waitpid(-1, NULL, WNOHANG);
    if(pid == -1)
    {
        if(errno == ECHILD)
        {
            errno = 0;
            return;
        }
        perror("waitpid() failed");
        exit(1);
    }
    while(pid != 0)
    {
        removePID(childPIDs, pid);
        pid_t pid = waitpid(-1, NULL, WNOHANG);
        if(pid == -1)
        {
            if(errno == ECHILD)
            {
                errno = 0;
                return;
            }
            perror("waitpid() failed");
            exit(1);
        }
    }
    return;
}

PIDNode* getLastPID(PIDNode* childPIDs)
{
    PIDNode* currentPID = childPIDs;
    PIDNode* ret = currentPID;
    while(currentPID != NULL)
    {
        ret = currentPID;
        currentPID = currentPID->next;
    }
    return ret;

}

void removePID(PIDNode* childPIDs, pid_t pid)
{
    /*Search for pid*/
    BOOL found = false;
    PIDNode* currentPID = childPIDs;
    PIDNode* node2Del = NULL;
    while((currentPID != NULL) && (!found))
    {
        if(currentPID->pid == pid)
        {
            found = true;
            node2Del = currentPID;
        }
        currentPID = currentPID->next;
    }
    if(found)
    {
        /*Delete node2Del*/
        if(node2Del->prev != NULL)
        node2Del->prev->next = node2Del->next;
        if(node2Del->next != NULL)
        node2Del->next->prev = node2Del->prev;
        free(node2Del);
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
    if(currentNode->inRedir != 0)
    {
        if(currentNode->inRedir == REDIR_IN)
            printf("<\n --: %s\n", currentNode->inFile);
    }
    if(currentNode->outRedir != 0)
    {
        if(currentNode->outRedir == REDIR_OUT)
            printf(">\n --: %s\n", currentNode->outFile);

        else if(currentNode->outRedir == APPEND_OUT)
            printf(">>\n --: %s\n", currentNode->outFile);

        else if(currentNode->outRedir == PIPE)
        {
            printf("|\n");
        }
    }
    if(currentNode->errRedir != 0)
    {
        if(currentNode->errRedir == REDIR_ERR)
            printf("2>\n --: %s\n", currentNode->errFile);

        else if(currentNode->errRedir == APPEND_ERR)
            printf("2>>\n --: %s\n", currentNode->errFile);
        else if(currentNode->errRedir == REDIR_ERR_OUT)
            printf("2>1\n");
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

void setAllBG(Node* startNode)
{
    Node* currentNode = startNode;
    while((currentNode != NULL) && (currentNode->command != NULL))
    {
        currentNode->runBG = 1;
        currentNode = currentNode->next;
    }
}


