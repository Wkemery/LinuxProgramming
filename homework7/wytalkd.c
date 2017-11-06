/*
* wytalkd.c
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
#include<unistd.h>
#include<string.h>
#include<stdlib.h>

/******************************************************************************/


int main (int argc, char** argv)
{
    BOOL talking = true;
    char buf[SIZE];
    char out[SIZE];
    int rtn;
    char hostname[SIZE];
    memset(hostname, 0, SIZE);

    rtn = gethostname(hostname, SIZE);
    if(rtn == -1)
    {
        perror("hostname error");
        exit(1);
    }

    //create socket and bind to port 51100
    int sockfd = serve_socket(hostname, 51100);

    int socket = accept_connection(sockfd);

    while(talking)
    {
        memset(out, 0, SIZE);
        memset(buf, 0, SIZE);
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
/******************************************************************************/

        memset(buf, 0, SIZE);
        char* crtn = fgets(buf, SIZE, stdin);
        if(crtn == NULL) //user closed input or there was an error
        {
            if(errno != 0) perror("Error reading from stdin");
            write(socket, "\0", 1);
            talking = false;

            memset(out, 0, SIZE);
            memset(buf, 0, SIZE);
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
                if(strlen(out) == (SIZE - 1)) //if out is full, write out
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
                talking = false;
            }
            else if(rtn == 0)
            {
                //socket closed
                talking = false;
            }

            rtn = write(socket, "\0", 1);
            if(rtn == -1)
            {
                perror("writing to socket failed");
                talking = false;
            }
            else if(rtn == 0)
            {
                //socket closed
                talking = false;
            }
        }
    }
    if(socket != 0)
    {
      rtn = close(socket);
      if(rtn == -1) perror("closing socket failed");
    }
    
    return 0;
}
