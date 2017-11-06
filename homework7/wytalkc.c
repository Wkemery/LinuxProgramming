/*
* wytalkc.c
* Author: Wyatt Emery
* Date: OCT 18, 2016
*
* COSC 3750, Homework7
*
* Description
*/

#define SIZE 4096
#define true 1
#define false 0
typedef short BOOL;

#include"socketfun.h"

#include<stdio.h>
#include<errno.h>
#include<string.h>
#include<unistd.h>
#include<stdlib.h>
#include <fcntl.h>

/******************************************************************************/


int main (int argc, char** argv)
{
    int rtn;
    BOOL talking = true;
    char buf[SIZE];

    if(argc != 2)
    {
        perror("incorrect number or arguments");
        exit(1);
    }

    int socket = request_connection(argv[1], 51100);

    while(talking)
    {
        memset(buf, 0, SIZE);
        char* crtn = fgets(buf, SIZE, stdin);

        if(crtn == NULL) //user closed input or there was an error
        {
            if(errno != 0) perror("Error reading from stdin");
            write(socket, "\0", 1);
            talking = false;
        }
        else //user entered something to be sent
        {
            while(strlen(crtn) == SIZE - 1)
            {
                rtn = write(socket, buf, strlen(buf));
                crtn = fgets(buf, SIZE, stdin);
                if(crtn == NULL) //user closed input or there was an error
                {
                    if(errno != 0) perror("Error reading from stdin");
                    write(socket, "\0", 1);
                    talking = false;
                }
            }
            rtn = write(socket, buf, strlen(buf));
            if(rtn == -1)
            {
                perror("writing to socket failed");
                break;
            }
            else if(rtn == 0)
            {
                //socket closed
                break;
            }

            rtn = write(socket, "\0", 1);
            if(rtn == -1)
            {
                perror("writing to socket failed");
                break;
            }
            else if(rtn == 0)
            {
                //socket closed
                break;
            }
        }

/******************************************************************************/

        memset(buf, 0, SIZE);
        char out[SIZE];
        memset(out, 0, SIZE);

        rtn = read(socket, buf, 1);

        if(rtn == 0) break;
        while(buf[0] != '\0')
        {
            if(rtn == -1 )
            {
                perror("reading from socket failed");
                talking = false;
                break;
            }
            else if(rtn == 0)
            {
                //socket closed
                break;
            }
            if(strlen(out) == (SIZE - 1)) //if out is full, write out to stdout
            {
                rtn = fwrite(out, 1, strlen(out), stdout);
                if(rtn < strlen(out)) perror("error writing to stdout");
                memset(out, 0, SIZE);
            }
            strcat(out, buf);
            rtn = read(socket, buf, 1);
        }
        rtn = fwrite(out, 1, strlen(out), stdout);
        if(rtn < strlen(out)) perror("error writing to stdout");
    }
    if(socket != 0)
    {
      rtn = close(socket);
      if(rtn == -1) perror("closing socket failed");
    }
    return 0;
}
