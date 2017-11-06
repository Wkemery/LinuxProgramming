/*
 * ShellFunctions.h
 * Author: Wyatt Emery
 * Date: NOV 29, 2016
 *
 * COSC 3750, Homework11
 *
 * header file for ShellFunctions.c
 */


#define STDBUFF 4096
typedef struct node Node;
typedef struct word Word;
typedef struct pidNode PIDNode;

#include<stdlib.h>
#include<sys/types.h>

#ifndef SHELL_FUNCTIONS
#define SHELL_FUNCTIONS

struct node
{
    Node* next;
    Node* prev;
    char* command;
    Word* arguments;
    int argCount;
    int runBG;
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

struct pidNode
{
    PIDNode* next;
    PIDNode* prev;
    pid_t pid;
};
/*****************************************************************************/

extern Node* newNode(Node* prevNode);
    /* Uses malloc to allocate data for a node and sets the members of the
     * struct to their default values. prevNode is set to the created node's
     * prev pointer to continue linked list.
     * Returns a pointer to the created Node */

extern Word* newWord(Word* prevWord);
    /* Uses malloc to allocate data for a new word and sets
     * the members of the struct to their default values.
     * prevWord is used to continue the linked list and
     * is set to the created node's prev pointer.
     * Returns a pointer to the created Word */

extern PIDNode* newPIDNode(PIDNode* prevPIDNode);
    /* Uses malloc to allocate data for a new word and sets the members of the
     * struct to their default values. prevPIDNode is used to continue the
     * linked list and is set to the created node's prev pointer
     * Returns a pointer to the created PIDNode. */

extern Node* parseInput(char* lineptr);
    /* This function has a very large switch statement that acts as a finite
     * state machine for the various states entered into based on input.
     * It returns a pointer to the first node in the linked list of
     * command nodes. */

extern void deleteNodes(Node* startNode);
    /* Recursively frees all the data along the linked list of nodes
     * and Word arguments */

extern void deleteWords(Word* startWord);
    /*Helper function to deleteNodes. It does the same thing as deleteNodes,
     * but for Words. */

extern void removePID(PIDNode* node2Del);
    /* This function will remove a given PIDNode from the linked list of
     * PIDnodes and adjust the others members of the list accordingly. It frees 
     * the PIDNode that it removes from the list. */

extern void execAllCommands(Node* startNode, PIDNode** childPIDs);
    /* execAllCommands executes all commands in the linked list of commands
     * pointed to by startNode. It takes a reference to the pointer of childPID
     * nodes. It needs a reference in order to change what childPIDs points
     * to outside the function. This ensures the pointer passed to
     * execAllCommands always points to either the first node in the linked list
     * of PIDNodes or NULL.
     * It handles building the list of childPIDs to be waited on as they are
     * created.
     * It sets up and tears down pipes before and after calling execCommand.*/

extern pid_t execCommand(Node* command, int backEndPipe, int frontEndPipe);
    /* Helper comand to execAllCommands. It creates the arglist from command's
     * arguments. It calls fork and execvp to execute the command. backEndPipe
     * is the fd that this command is meant to read from if its input is
     * redirected to a pipe. frontEndPipe is the fd this command is meant to
     * write to if its output is redirected to a Pipe. All redirection is done
     * here. If the process is the parent, the pid of the child is returned. */

extern PIDNode* waitOnChildren(PIDNode* childPIDs);
    /* waitOnChildren iterates through the list of childPIDs and waits on each
     * one. It waits with WNOHANG so if a process has not changed state, it
     * moves on. as processes are successfully waited on, the are removed
     * from the list. It returns a pointer to the first PIDNode in the list,
     * in case the first one was removed. WaitOnChildren calls removePID which 
     * will free the PIDNodes along the way. */

extern PIDNode* getLastPID(PIDNode* childPIDs);
    /* This function returns a PIDNode* to the very last PIDNode in the linked
     * list of PIDs. This is useful for adding on a PID to the end of
     * the list. */

extern Word* getLastWord(Word* startWord);
    /* This function returns a Word* to the very last word in the linked list of
     * arguments. This is useful for adding on an argument to the end of a list
     * of Words without doing too much work in main. */

extern void setAllBG(Node* startNode);
    /* In the case that an & is encountered at the end of a string of commands,
     * the Nodes for multiple commands need to be changed such that they are
     * executed in the background. This function does that. */


#endif
