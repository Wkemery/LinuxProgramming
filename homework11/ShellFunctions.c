/*
 * ShellFunctions.c
 * Author: Wyatt Emery
 * Date: NOV 29, 2016
 *
 * COSC 3750, Homework11
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
#include <fcntl.h>


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
    ret->argCount = 0;
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

        if(!((token == SEMICOLON) || (token == EOL)) && (lastToken == AMP))
        {
            fprintf(stderr, "Misplaced &\n");
            scanning = false;
            deleteNodes(startNode);
            startNode = NULL;
            return NULL;
        }
        /*ampersand at end of command means set all to run in background*/
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
                    /*
                    int len = strlen(currentNode->outFile) + 1;
                    currentNode->errFile = (char*) malloc(len);
                    if(currentNode->errFile == NULL)
                    {
                        perror("Malloc Error Occured");
                        exit(1);
                    }
                    memset(currentNode->errFile, 0, len);
                    strncpy(currentNode->errFile, currentNode->outFile, len);
                    */
                    currentNode->errFile = strdup(currentNode->outFile);
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
                    /*int len = strlen(lexeme) + 1;
                    currentNode->outFile = (char*) malloc(len);
                    if(currentNode->outFile == NULL)
                    {
                        perror("Malloc Error Occured");
                        exit(1);
                    }
                    memset(currentNode->outFile, 0, len);
                    strncpy(currentNode->outFile, lexeme, len);
                    outputRedirect = false;
                    */
                    currentNode->outFile = strdup(lexeme);
                    break;
                }
                if(inputRedirect)
                {
                    /*Current Word is a redirection file for input*/
                    /*int len = strlen(lexeme) + 1;
                    currentNode->inFile = (char*) malloc(len);
                    if(currentNode->inFile == NULL)
                    {
                        perror("Malloc Error Occured");
                        exit(1);
                    }
                    memset(currentNode->inFile, 0, len);
                    strncpy(currentNode->inFile, lexeme, len);
                    inputRedirect = false;
                    */
                    currentNode->inFile = strdup(lexeme);
                    break;
                }
                if(errorRedirect)
                {
                    /*Current Word is a redirection file for error*/
                    /*int len = strlen(lexeme) + 1;
                    currentNode->errFile = (char*) malloc(len);
                    if(currentNode->errFile == NULL)
                    {
                        perror("Malloc Error Occured");
                        exit(1);
                    }
                    memset(currentNode->errFile, 0, len);
                    strncpy(currentNode->errFile, lexeme, len);
                    errorRedirect = false;
                    */
                    currentNode->errFile = strdup(lexeme);
                    break;
                }
                if(currentNode->command == NULL)
                {
                    /*Current Word is assumed to be a command*/
                    /*int len = strlen(lexeme) + 1;
                    currentNode->command = (char*) malloc(len);
                    if(currentNode->command == NULL)
                    {
                        perror("Malloc Error Occured");
                        exit(1);
                    }
                    memset(currentNode->command, 0, len);
                    strncpy(currentNode->command, lexeme, len);
                    */
                    currentNode->command = strdup(lexeme);
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
                    /*int len = strlen(lexeme) + 1;
                    newArg->data = (char*) malloc(len);
                    if(newArg->data == NULL)
                    {
                        perror("Malloc Error Occured");
                        exit(1);
                    }
                    memset(newArg->data, 0, len);
                    strncpy(newArg->data, lexeme, len);
                    */
                    newArg->data = strdup(lexeme);
                    currentNode->argCount++;
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

void removePID(PIDNode* node2Del)
{
    if(node2Del->prev != NULL)
        node2Del->prev->next = node2Del->next;
    if(node2Del->next != NULL)
        node2Del->next->prev = node2Del->prev;
    free(node2Del);

}

void execAllCommands(Node* startNode, PIDNode** childPIDs)
{
    Node* currentNode = startNode;
    int frontEndPipe[2];
    int backEndPipe = -1;
    frontEndPipe[0] = -1;
    frontEndPipe[1] = -1;
    int ret;
    while((currentNode != NULL) && (currentNode->command != NULL))
    {
        /*Pipe Handling*/

        /* If current node input redirection is PIPE then the previous node had
         * to output to a pipe. */
        if(currentNode->inRedir == PIPE)
        {
            /* backEndPipe keeps a reference to the read end of the previous
             * pipe before a new pipe is opened. */
            backEndPipe = dup(frontEndPipe[0]);
            if(backEndPipe == -1) perror("dup error1");

            ret = close(frontEndPipe[0]);
            frontEndPipe[0] = -1;
            if(ret == -1)
            {
                perror("close error");
                if(errno == EBADF) errno = 0;
                else perror("close error");
            }
        }
        if(currentNode->outRedir == PIPE)
        {
            /* If this command's output is redirected to a pipe a new Pipe must
             * be opened for it to write to. If it is supposed to read from a
             * pipe then that data is referenced by backEndPipe.*/
            ret = pipe(frontEndPipe);
            if(ret == -1) perror("pipe Error");
        }

        /**********************************************************************/
        pid_t childPID = execCommand(currentNode, backEndPipe,
                                     frontEndPipe[1]);
        /**********************************************************************/

        /* close the write end of the current Pipe but leave the read end open
         * for the next command which there has to be a next command. */
        if(frontEndPipe[1] != -1)
        {
            ret = close(frontEndPipe[1]);
            frontEndPipe[1] = -1;
            if(ret == -1)
            {
                perror("close error");
                if(errno == EBADF) errno = 0;
                else perror("close error");
            }
        }

        /* Close the read end of the previous pipe before moving on to the next
         * command if there is a next command. */
        if(backEndPipe != -1)
        {
            ret = close(backEndPipe);
            backEndPipe = -1;
            if(ret == -1)
            {
                if(errno == EBADF) errno = 0;
                else perror("close error");

            }
        }

        /**********************************************************************/

        /*End Pipe handling*/

        if(currentNode->runBG)
        {
            /*Add child pid to list of PIDs*/
            PIDNode* lastPID = getLastPID(childPIDs[0]);
            PIDNode* newPID = newPIDNode(lastPID);
            newPID->pid = childPID;
            if(lastPID != NULL)
                lastPID->next = newPID;
            if(childPIDs[0] == NULL)
                childPIDs[0] = newPID;

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

pid_t execCommand(Node* command, int backEndPipe, int frontEndPipe)
{
    /*Fork Time*/
    pid_t childPID = fork();
    if(childPID == -1)
    {
        /*On fork error both the parent and child should exit*/
        perror("Fork Error!");
        exit(1);
    }

    if(childPID == 0) //This is the Child
    {
        /* Create Arglist for exec. The memory in the structs is pointed to be 
         * the char*s in the arglist*/
        char** argList = calloc((command->argCount + 2), sizeof(char*));
        Word* currentWord = command->arguments;
        argList[0] = strdup(command->command);        
        int i = 1;
        while(currentWord != NULL)
        {
            argList[i] = currentWord->data;
            currentWord = currentWord->next;
            i++;
        }

/*****************************************************************************/
        int infile;
        int outfile;
        int errfile;

        /* this is to keep a copy of the original stdout which I may need
         * depending on the order of stdout redirection and stderr redirection
         * to sdout*/
        int temp_stdout;
        temp_stdout = dup(STDOUT_FILENO);

        /*Redirect input/output/error if necessary*/


        switch(command->inRedir)
        {
            case REDIR_IN:
                infile = open(command->inFile, O_RDONLY);
                dup2(infile, STDIN_FILENO);
                close(infile);
                break;
            case PIPE:
                dup2(backEndPipe, STDIN_FILENO);
                close(backEndPipe);
                break;
            default:
                break;
        }

        switch(command->outRedir)
        {
            case REDIR_OUT:
                outfile = open(command->outFile, O_WRONLY | O_CREAT | O_TRUNC,
                               00666);
                dup2(outfile, STDOUT_FILENO);
                close(outfile);
                break;
            case APPEND_OUT:
                outfile = open(command->outFile, O_WRONLY | O_APPEND | O_CREAT,
                               00666);
                dup2(outfile, STDOUT_FILENO);
                close(outfile);
                break;
            case PIPE:
                dup2(frontEndPipe, STDOUT_FILENO);
                close(frontEndPipe);
                break;
            default:
                break;
        }

        switch(command->errRedir)
        {
            case REDIR_ERR:
                errfile = open(command->errFile, O_WRONLY | O_CREAT | O_TRUNC,
                               00666);
                dup2(errfile, STDERR_FILENO);
                close(errfile);
                break;
            case APPEND_ERR:
                errfile = open(command->errFile, O_WRONLY | O_APPEND | O_CREAT,
                               00666);
                dup2(errfile, STDERR_FILENO);
                close(errfile);
                break;
            case REDIR_ERR_OUT:
                if(command->errFile == NULL)
                    dup2(temp_stdout, STDERR_FILENO);
                else
                    dup2(STDOUT_FILENO, STDERR_FILENO);
                break;
            default:
                break;
        }

        close(temp_stdout);

        /*Execute Command*/
        int ret = execvp(command->command, argList);
        if(ret == -1)
        {
            perror(command->command);
            exit(1);
        }
    }

    return childPID;
}

PIDNode* waitOnChildren(PIDNode* childPIDs)
{
    PIDNode* ret = childPIDs;
    if(childPIDs == NULL)
    {
        return NULL;
    }
    PIDNode* currentPID = childPIDs;

    /* Go though each PID and try to wait on it. If successfully waited, remove
     * PID from list of PIDs. */
    while(currentPID != NULL)
    {
        pid_t waitOn = currentPID->pid;
        pid_t pid = waitpid(waitOn, NULL, WNOHANG);
        if(pid == -1)
        {
            if(errno == ECHILD)
            {
                errno = 0;
                return ret;
            }
            perror("waitpid() failed");
            exit(1);
        }
        else if(pid != 0)
        {
            PIDNode* node2Del = currentPID;
            if(node2Del->prev == NULL)
                ret = node2Del->next;

            currentPID = currentPID->next;
            removePID(node2Del);
        }
        else
        {
            currentPID = currentPID->next;
        }
    }

    return ret;
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




