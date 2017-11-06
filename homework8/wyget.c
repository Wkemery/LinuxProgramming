/*
* wyget.c
* Author: Wyatt Emery
* Date: OCT 29, 2016
*
* COSC 3750, Homework8
*
* wyget will download files using http only. It takes various options
* and it takes urls. If a valid URL is entered, it will try to connect
* to it. If it cannot connect, it just hangs, while it is trying to connect.
* I think all errors are detected and printed out. I used a char* error to 
* pass error messages between functions. The functions I defined will detect
* (hopefully) all errors and set errbuf to a reasonable error message using
* information that main might not have. however, main checks the char* error
* for being NULL. 
* Also a noted exploit is due to the fact
*/

#define STDBUFF 4096
#define true 1
#define false 0
typedef short BOOL;


#include<stdio.h>
#include<errno.h>
#include<stdlib.h>
#include<unistd.h>
#include"wygetFunctions.h"
#include<string.h>
#include <sys/stat.h>

/******************************************************************************/

int main (int argc, char** argv)
{
    opterr = 0; //disable getopt error messages

    /*Option Flags*/
    BOOL opt_i = false, opt_o = false, opt_nc = false, opt_p = false;
    BOOL opt_d = false;
    /*Option Arguments*/
    char* inputFile = NULL;
    char* outputFile = NULL;
    char* prefix = NULL;
    FILE* oFile = NULL;
    int option;

    /*Parse Options*/
    while ((option = getopt(argc, argv, ":i:o:n::p:d")) != -1)
    {
        switch(option)
        {
            case 'i':
                opt_i = true;
                inputFile = optarg;
                break;
            case 'o':
                opt_o = true;
                outputFile = optarg;
                break;
            case 'n':
                if(optarg[0] == 'c') opt_nc = true;
                else fprintf(stderr, "Invalid Flag: n%s\n",optarg);
                break;
            case 'p':
                opt_p = true;
                prefix = optarg;
                break;
            case 'd':
                opt_d = true;
                break;
            case '?':
                fprintf(stderr,"Invalid flag: %c\n",optopt);
                return 1;
            default:
                fprintf(stderr,"Missing Argument: %c\n", optopt);
                return 1;
        }
    }

/******************************************************************************/

    if((!opt_i) && !(optind < argc))
    {
        fprintf(stderr, "No URLs specified\n");
        return 1;
    }

    if(!opt_p)
    {
        prefix = malloc(1);
        prefix[0] = 0;
    }
    if(opt_o)
    {
        if(opt_nc)
        {
            oFile = NCCreateFile(outputFile);
        }
        else /*create/clobber file*/
        {
            oFile = fopen(outputFile, "w");
        }
    }

    if(optind < argc) /*if there are URL arguments to wyget*/
    {
        int i = optind;
        int ret;
        char buf[STDBUFF];
        char* error = NULL;
        while(i < argc)
        {
            error = NULL;
            char* host = NULL;
            char* pathRs = NULL;
            char* resource = NULL;
            FILE* outFile = NULL;

            parseURL(&host, &pathRs, &resource, argv[i], &error);
            if(error != NULL) fprintf(stderr, "%s: %s\n", argv[i], error);
            else
            {
                if(opt_d)
                {/*create dirstructure and file*/
                    outFile = opt_dSetup(prefix, argv[i], opt_nc, &error);
                }
                else/*create file in current dir*/
                {
                    if(opt_nc) outFile = NCCreateFile(resource);
                    else outFile = fopen(resource, "w+");
                }

                if(error != NULL) fprintf(stderr, "%s: %s\n", argv[i], error);
                else
                {
                    /*download stuff from that URL*/
                    getHTMLDoc(host, pathRs, outFile, &error);
                    if(error != NULL) fprintf(stderr, "%s: %s\n", argv[i], error);
                    else
                    {
                        if(opt_o)
                        {
                            rewind(outFile);
                            ret = fread(buf, 1, STDBUFF - 1, outFile);
                            while(ret != 0)
                            {
                                fwrite(buf, 1, ret, oFile);
                                ret = fread(buf, 1, STDBUFF - 1, outFile);
                            }
                        }
                    }
                }
            }
            if(host != NULL) free(host);
            if(pathRs != NULL) free(pathRs);
            if(resource != NULL) free(resource);
            if(error != NULL) free(error);
            if(outFile != NULL) fclose(outFile);
            i++;
        }
    }
/******************************************************************************/

    if(opt_i)
    {
        FILE* infile = fopen(inputFile, "r");
        if(infile == NULL)
        {
            perror("could not open file");
            return 1;
        }
        char* url = NULL;
        ssize_t gtlnReturn = 0;
        size_t len = 0;

        while((gtlnReturn = getline(&url, &len, infile)) != -1)
        {
            url[strlen(url) - 1] = '\0'; //get rid of \n
            int ret;
            char buf[STDBUFF];
            char* error = NULL;
            char* host = NULL;
            char* pathRs = NULL;
            char* resource = NULL;
            FILE* outFile = NULL;

            parseURL(&host, &pathRs, &resource, url, &error);
            if(error != NULL) fprintf(stderr, "%s: %s\n", url, error);
            else
            {
                if(opt_d)
                {/*create dirstructure and file*/
                    outFile = opt_dSetup(prefix, url, opt_nc, &error);
                }
                else/*create file in current dir*/
                {
                    if(opt_nc) outFile = NCCreateFile(resource);
                    else outFile = fopen(resource, "w+");
                }

                if(error != NULL) fprintf(stderr, "%s: %s\n", url, error);
                else
                {
                    /*download stuff from that URL*/
                    getHTMLDoc(host, pathRs, outFile, &error);
                    if(error != NULL) fprintf(stderr, "%s: %s\n", url, error);
                    else
                    {
                        if(opt_o)
                        {
                            rewind(outFile);
                            ret = fread(buf, 1, STDBUFF - 1, outFile);
                            while(ret != 0)
                            {
                                fwrite(buf, 1, ret, oFile);
                                ret = fread(buf, 1, STDBUFF - 1, outFile);
                            }
                        }
                    }
                }
            }
            if(host != NULL) free(host);
            if(pathRs != NULL) free(pathRs);
            if(resource != NULL) free(resource);
            if(error != NULL) free(error);
            if(outFile != NULL) fclose(outFile);

        }
        free(url);
        fclose(infile);
    }

    //close o file
    if(oFile!= NULL)fclose(oFile);
    return 0;
}






