/*
 * wygetFunctions.c
 * Author: Wyatt Emery
 * Date: OCT 29, 2016
 *
 * COSC 3750, Homework8
 *
 */

#include"wygetFunctions.h"
#include<sys/types.h>
#include<string.h>
#include <sys/stat.h>
#include <unistd.h>
#include<stdlib.h>
#include <regex.h>
#include<stdio.h>
#include"socketfun.h"


#define STDBUFF 4096
#define DEFAULT_PERMS 0000755

void parseURL(char** host, char** pathRs, char** resource, const char* URL, char** errbuf)
{
    regex_t pattern;
    size_t nmatch = 10;
    regmatch_t matches[10];
    int ret = regcomp(&pattern, "^([a-z]*://)?((www\\.)?[a-z\\.]*\\.[a-z]{2,3})/?(.*)", REG_EXTENDED);
    //int ret = regcomp(&pattern, "^([a-z]*://)?((www\\.)?[a-z\\.]*\\.[a-z]{2,3})(.*)", REG_EXTENDED);
    if(ret)
    {/*this should never happen, left in for debugging*/
        char errbuf[STDBUFF];
        int size=regerror(ret,&pattern,errbuf,STDBUFF);
        fprintf(stderr,"%s",errbuf);
        if(size > STDBUFF - 1) fprintf(stderr,"... more\n");
        else fprintf(stderr,"\n");
    }

    ret=regexec(&pattern,URL,nmatch,matches,0);
    if(ret != REG_NOMATCH)
    {
        /*malloc data for host*/
        int len = matches[2].rm_eo - matches[2].rm_so;
        *host = malloc(len + 1);
        memset(*host, 0, len + 1);
        memcpy(*host, URL+matches[2].rm_so, len);

        /*Malloc data for pathRs*/
        len = matches[4].rm_eo - matches[4].rm_so;
        *pathRs=malloc(len + 2);
        memset(*pathRs, 0, len + 2);
        pathRs[0][0] = '/';
        memcpy(pathRs[0]+1, URL+matches[4].rm_so, len);

        int i = strlen(*pathRs) - 1;
        while(i >= 0)
        {
            /*find index of first slash from end*/
            if(pathRs[0][i] == '/') break;
            i--;
        }
        i++;
        if(i < 0)
        {
            /*if i is less than zero no slashes were encountered and something
             *went terribly wrong*/
            char* error = "Fatal Error";
            *errbuf = malloc(strlen(error) + 1);
            strcpy(*errbuf, error);
            return;
        }
        /*Mallco data for resource*/
        len = strlen(*pathRs) - i;
        if(len == 0)
        {
            len = 11; //for index.html
            char* temp = "index.html";
            *resource = malloc(len + 1);
            memset(*resource, 0, len +1);
            memcpy(*resource, temp, len);
          
        }
        else
        {
            *resource = malloc(len + 1);
            memset(*resource, 0, len +1);
            memcpy(*resource, *pathRs+i, len);
        }


    }
    else
    {
        char err[STDBUFF];
        regerror(ret,&pattern,err,STDBUFF);
        if(strcmp(err, "No match") == 0)
        {
            char error[STDBUFF];
            sprintf(error, "Unrecognizable URL: %s", URL);
            *errbuf = malloc(strlen(error) + 1);
            strcpy(*errbuf, error);
            return;
        }
        else
        {
            char error[STDBUFF];
            sprintf(error, "Regex error: %s", err);
            *errbuf = malloc(strlen(error) + 1);
            strcpy(*errbuf, error);
            return;
        }
    }
    //printf("%s, %s, %s", host[0], pathRs[0], resource[0]);
}

int parseResponse(char** header, char** body, const char* response, char** errbuf)
{
    regex_t pattern;
    size_t nmatch = 10;
    regmatch_t matches[10];

    /*I was unable to get the regex expression to capture what I needed with only one regcomp
     * so I split it into two*/

    /*This is for the header*/
    int ret = regcomp(&pattern, "^HTTP/1.. ([0-9]{3} [A-Za-z ]*)\r\n", REG_EXTENDED);
    if(ret)
    {/*this should never happen, left in for debugging*/
        char errbuf[STDBUFF];
        int size =regerror(ret,&pattern,errbuf,STDBUFF);
        fprintf(stderr,"%s",errbuf);
        if(size > STDBUFF - 1) fprintf(stderr,"... more\n");
        else fprintf(stderr,"\n");
    }

    ret=regexec(&pattern,response,nmatch,matches,0);
    if(ret != REG_NOMATCH)
    {
        /*Malloc data for header*/
        int len = matches[1].rm_eo - matches[1].rm_so;
        *header = malloc(len + 1);
        memset(*header, 0, len + 1);
        memcpy(*header, response+matches[1].rm_so, len);

    }
    else
    {
        char err[STDBUFF];
        regerror(ret,&pattern,err,STDBUFF);
        if(strcmp(err, "No match") == 0)
        {
            char error[STDBUFF];
            sprintf(error, "Error reading response HTTP message");
            *errbuf = malloc(strlen(error) + 1);
            strcpy(*errbuf, error);
            return -1;
        }
        else
        {
            char error[STDBUFF];
            sprintf(error, "Regex error: %s", err);
            *errbuf = malloc(strlen(error) + 1);
            strcpy(*errbuf, error);
            return -1;
        }
    }
/******************************************************************************/

    /*This is for the body*/
    regfree(&pattern);
    ret = regcomp(&pattern, "(\r\n\r\n([^\n]|[\n])*)", REG_EXTENDED);
    if(ret)
    {//this should never happen, left in for debugging
        char errbuf[STDBUFF];
        int size =regerror(ret,&pattern,errbuf,STDBUFF);
        fprintf(stderr,"%s",errbuf);
        if(size > STDBUFF - 1) fprintf(stderr,"... more\n");
        else fprintf(stderr,"\n");
    }

    ret=regexec(&pattern,response,nmatch,matches,0);
    if(ret != REG_NOMATCH)
    {
        /*Malloc data for body*/
        int len = matches[1].rm_eo - matches[1].rm_so - 4;
        *body = malloc(len + 1);
        memset(*body, 0, len + 1);
        memcpy(*body, response+matches[1].rm_so + 4, len);
    }
    else
    {
        char err[STDBUFF];
        regerror(ret,&pattern,err,STDBUFF);
        if(strcmp(err, "No match") == 0)
        {
            char error[STDBUFF];
            sprintf(error, "Error reading response HTTP message");
            *errbuf = malloc(strlen(error) + 1);
            strcpy(*errbuf, error);
            return -1;
        }
        else
        {
            char error[STDBUFF];
            sprintf(error, "Regex error: %s", err);
            *errbuf = malloc(strlen(error) + 1);
            strcpy(*errbuf, error);
            return -1;
        }
    }
    return matches[1].rm_eo - matches[1].rm_so - 4;
}


FILE* NCCreateFile(const char* path)
{
    struct stat fileStats;
    char temp[STDBUFF];
    char num[10];
    int i = 1;
    memset(temp, 0, STDBUFF);
    memcpy(temp, path, strlen(path));
    int ret = stat(temp, &fileStats);
    /*Keep stating and checking if the file exists. There is nothing to prevent he integer i
     *from rolling over but I think I'm safe */
    while(ret == 0)
    {
        memset(temp, 0, STDBUFF);
        memcpy(temp, path, strlen(path));
        sprintf(num, "%d", i);
        strcat(temp, ".");
        strcat(temp, num);
        ret = stat(temp, &fileStats);
        i++;
    }
    return fopen(temp, "w");
}

FILE* opt_dSetup(const char* prefix, const char* URL, short opt_nc, char** errbuf)
{
    *errbuf = NULL;
    FILE* outFile;
    /*create dir tree from host, path and prefix*/
    char dirStructure[STDBUFF];
    memset(dirStructure, 0, STDBUFF);
    char temp[STDBUFF];
    memset(temp, 0, STDBUFF);



    if(strlen(prefix) != 0)
    {
        strcat(dirStructure, prefix);
        strcat(dirStructure, "/");
    }

    char* host = NULL;
    char* pathRs = NULL;
    char* resource = NULL;

    parseURL(&host, &pathRs, &resource, URL, errbuf);

    strcat(dirStructure, host);
    strcat(dirStructure, pathRs);

    if(pathRs[strlen(pathRs) - 1] == '/') strcat(dirStructure, "index.html");
    createDirTree(dirStructure);

    if(opt_nc) /*create new file with .n appended to name*/
    {
        outFile = NCCreateFile(dirStructure);
    }
    else /*create/clobber file*/
    {
        outFile = fopen(dirStructure, "w+");
    }
    if(outFile == NULL)
    {
        char error[STDBUFF];
        sprintf(error, "Error creating file: %s", dirStructure);
        *errbuf = malloc(strlen(error) + 1);
        strcpy(*errbuf, error);
    }
    free(host);
    free(pathRs);
    free(resource);

    return outFile;
}

void getHTMLDoc(char* host, const char* pathRs, FILE* HTMLdoc, char** errbuf)
{
    *errbuf = NULL;
    int ret;
    char* header = NULL;
    char* body = NULL;
    char buf[STDBUFF];

    /*open a connection to host*/
    int socket = request_connection(host, 80);
    if(socket == 1)
    {
        char error[STDBUFF];
        sprintf(error, "Error creating connection to %s", host);
        *errbuf = malloc(strlen(error) + 1);
        strcpy(*errbuf, error);
        if(socket!= 0) close(socket);
        return;
    }

    /*create get message*/
    char msg[STDBUFF];
    memset(msg, 0, STDBUFF);
    char* format = "GET %s HTTP/1.0\nHost: %s\nconnection: Closed\n\n";
    sprintf(msg, format, pathRs, host);
    //if(pathRs[strlen(pathRs) - 1] == '/') strcat(msg, "index.html");

    //printf("%s\n", msg);
    /*send get message*/
    ret = write(socket, msg, strlen(msg));
    if(ret == 0)
    {
        char* error = "Socket unexpectedly closed";
        *errbuf = malloc(strlen(error) + 1);
        strcpy(*errbuf, error);
        if(socket!= 0) close(socket);
        return;
    }
    else if(ret == -1) /*write error*/
    {
        char* error = "Error writing to Socket";
        *errbuf = malloc(strlen(error) + 1);
        strcpy(*errbuf, error);
        if(socket!= 0) close(socket);
        return;
    }
/******************************************************************************/

    /*read from socket*/
    memset(buf, 0, STDBUFF);
    int readRet = read(socket, buf, STDBUFF - 1);
    if(ret == 0)
    {
        char* error = "Socket unexpectedly closed";
        *errbuf = malloc(strlen(error) + 1);
        strcpy(*errbuf, error);
        if(socket!= 0) close(socket);
        return;
    }

    /*pull out header and body*/
    int writeOut = parseResponse(&header, &body, buf, errbuf);
    if(*errbuf != NULL)
    {
        free(header);
        free(body);
        if(socket!= 0) close(socket);
        return;
    }

    if(strcmp(header, "200 OK") != 0)
    {
        char error[STDBUFF];
        sprintf(error, "HTTP Error: %s", header);
        *errbuf = malloc(strlen(error) + 1);
        strcpy(*errbuf, error);
        free(header);
        free(body);
        if(socket!= 0) close(socket);
        return;
    }

    char path[STDBUFF];
    memset(path, 0, STDBUFF);
    char pathFile[STDBUFF];
    memset(pathFile, 0, STDBUFF);

/******************************************************************************/

    /*write body to file*/
    ret = fwrite(body, 1, writeOut, HTMLdoc);
    if(ret == -1)
    {
        char* error = "Error writing to file";
        *errbuf = malloc(strlen(error) + 1);
        strcpy(*errbuf, error);
        free(header);
        free(body);
        return;
    }

    while(readRet != 0)/* if filled up buf there is more to read*/
    {
        /*read in from socket*/
        memset(buf, 0, STDBUFF);
        readRet = read(socket, buf, STDBUFF - 1);
        if(readRet == -1)
        {
            char* error = "Error Writing to Socket";
            *errbuf = malloc(strlen(error) + 1);
            strcpy(*errbuf, error);
            free(header);
            free(body);
            if(socket!= 0) close(socket);
            return;
        }

        /*write out to file*/
        ret = fwrite(buf, 1, readRet, HTMLdoc);
        if(ret == -1)
        {
            char* error = "Error writing to file";
            *errbuf = malloc(strlen(error) + 1);
            strcpy(*errbuf, error);
            free(header);
            free(body);
            if(socket!= 0) close(socket);
            return;
        }
    }
    free(header);
    free(body);
    if(socket!= 0) close(socket);
}

int createDir(const char* path, mode_t perms)
{
    mode_t permissions = perms;
    if(permissions == 0) permissions = DEFAULT_PERMS; //set default perms
    struct stat fileStats;
    int statRet = lstat(path, &fileStats);
    if(statRet != 0) // file does not exist
    {
        int ret = mkdir(path, permissions);
        if(ret != 0) return -1;
    }
    else //file does exist
    {
        if(S_ISDIR(fileStats.st_mode)) //and its a dir
        {
            char command[STDBUFF] = "touch ";
            strcat(command, path);
            system(command);
        }
        else //not a directory
        {
            int unlRet = unlink(path);
            if(unlRet != 0) return -1;
            int ret = mkdir(path, permissions);
            if(ret != 0) return -1;
        }
    }
    return 0;
}

void createDirTree(const char*path)
{
    char dir[STDBUFF];
    memset(dir, 0, STDBUFF);
    short looking = 1;
    int i = 0;
    while(looking)
    {
        dir[i] = path[i];
        while((path[i] != '/') && (i < strlen(path) - 1))
        {
            i++;
            dir[i] = path[i];
        }
        if(i == (strlen(path) - 1)) looking = 0;
        else createDir(dir, DEFAULT_PERMS);
        i++; //step past slash
    }
}
