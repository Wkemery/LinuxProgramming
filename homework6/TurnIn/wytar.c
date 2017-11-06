/*
 * wytar.c
 * Author: Wyatt Emery
 * Date: OCT 6, 2016
 *
 * COSC 3750, Homework6
 *
 * wytar is a simplified version of the tar utility. it uses the POSIX ustar
 * tar header format. It supports options -c, -x, -f. while extracting, wytar
 * will overwrite any files in the directory that are being extracted from the tar.
 * all tar files created with wytar can be untarred with the real tar utility using option
 * -H ustar. all files tarred with the real tar utility using option -H ustar can be untarred
 * with wytar. wytar only works with links, directories and regular files.
 */

#define STDBUFF 4096
#define BLOCKSIZE 512

#define true 1
#define false 0
typedef short BOOL; // I used a short because 32 bits for either a 1 or 0 just seemed like too much waste

#include"MyTar.h"
#include<stdio.h>
#include<errno.h>
#include<stdlib.h>
#include<string.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<unistd.h>
#include<tar.h>



int main (int argc, char** argv)
{
    if(argc < 2)
    {
        errno=EINVAL;
        perror("wytar requires arguments, Exiting Now");
        exit(EXIT_FAILURE);
    }

    BOOL processingOptions = true;
    BOOL option_x = false;
    BOOL option_c = false;
    BOOL option_f = false;
    char* targetFile;

    /*---------------------------Process Options---------------------------*/
    int i = 1;

    while(processingOptions && i < argc) //for all options
    {

        //if current arg starts with -, its an option
        if(argv[i][0] == '-')
        {
            //process multiple options in this argument if there is more than 1
            int j = 1;
            char currentOption = argv[i][j];
            while(currentOption != '\0')
            {
                if(currentOption == 'x')
                {
                    option_x = true;
                    j++;
                    currentOption = argv[i][j];
                }
                else if(currentOption == 'c')
                {
                    option_c = true;
                    j++;
                    currentOption = argv[i][j];
                }
                else if(currentOption == 'f')
                {
                    if((i+1) < argc) //if there is an argument following this one
                    {
                        option_f = true;
                        targetFile = argv[i+1];
                        i++; //skip the next argument
                        currentOption = '\0'; //stop the loop
                    }
                    else //there is no argument to follow up -f
                    {
                        errno = EINVAL; //set errno to invalid argument
                        perror("Error! no filename specified after -f");
                        exit(EXIT_FAILURE);
                    }
                }
                else //current option is not a valid one
                {
                    errno = EINVAL; //set errno to invalid argument
                    perror("Error!");
                    exit(EXIT_FAILURE);
                }
            }
            i++;
        }
        else //current argument did not start with a "-" ,done processing options
        {
            processingOptions = false;
        }
    }

    /*---------------------------Process Parameters---------------------------*/

    if(option_x && option_c)
    {
        errno = EINVAL;
        perror("Error! Cannot have option -c and option -x set at the same time!");
        exit(EXIT_FAILURE);
    }

    if(!option_x && !option_c)
    {
        errno = EINVAL;
        perror("Error! Cannot have neither option -c nor option -x set");
        exit(EXIT_FAILURE);
    }
    if(!option_f)
    {
        errno=EINVAL;
        perror("Error! no option -f detected");
        exit(EXIT_FAILURE);
    }

    /*---------------------------Create a tar File---------------------------*/
    if(option_c)
    {
        if(argv[i]!=NULL) //there are arguments to tar up
        {
            //create and open tar file for writing
            FILE* tarFile = fopen(targetFile, "w");
            for(;i < argc; i++) //for every argument to tar up(the remaining arguments)
            {
                //Create a tar header from the file
                struct TarHeader* tarHeader = createTarH(argv[i]);
                if(tarHeader != NULL)
                {
                    //if file is directory, tar up all files in directory
                    if(tarHeader->typeflag[0] == DIRTYPE)
                    {
                        char dirPath[STDBUFF];
                        memset(dirPath, 0, STDBUFF);
                        memcpy(dirPath, argv[i], strlen(argv[i]));
                        tarDir(tarFile, tarHeader, dirPath);
                    }
                    else writeToTar(tarFile, tarHeader, argv[i]); //else write out file to tar
                }
                else //print error for this file but do not exit program
                {
                    char error[STDBUFF] = "Could not create tar header from file: ";
                    strcat(error, argv[i]);
                    perror(error);
                }
                free(tarHeader);
            }

            char nulls[BLOCKSIZE];
            memset(nulls, 0, sizeof nulls);
            int k = 0;
            /* I could have just made nulls an array of size 1024 but I prefered two blocks of 512
             * bytes instead of 1 block of 1024 bytes
            */
            for(; k < 2; k++) fwrite(nulls, BLOCKSIZE, 1, tarFile);
            fclose(tarFile);
        }
        else // there are not any arguments to tar up
        {
            errno=EINVAL;
            perror("Error! no files listed to tar up");
            exit(EXIT_FAILURE);
        }

    }
    /*---------------------------Extract A Tar File---------------------------*/
    else if(option_x)
    {
        //Open tar file
        FILE* tarFile = fopen(targetFile, "r");
        if(tarFile != NULL)
        {
            BOOL reading = true;
            char buffer[BLOCKSIZE];
            memset(buffer, 0, sizeof buffer);

            while(reading) //for every record in the tar file
            {
                int frdRet = fread(buffer, 1, BLOCKSIZE, tarFile);
                if(frdRet != BLOCKSIZE) perror("TarFile incorrect size!");

                //contruct tar header from record's first 512 bytes
                struct TarHeader* tarHeader = importTarH(buffer);

                //create file from tar header information
                FILE* outFile = createFile(tarHeader);

                //If its a regular file write filedata to outfile
                writeToFile(tarFile, outFile, tarHeader);

                free(tarHeader);
                if(outFile != NULL) fclose(outFile);

                //check for end of tar file
                char endBuffer[1024];
                memset(endBuffer, 0, sizeof endBuffer);
                frdRet = fread(endBuffer, 1, (BLOCKSIZE * 2), tarFile);
                if(frdRet != (BLOCKSIZE * 2))
                {
                    char error[STDBUFF] = "Invalid TarFile: Incorrect Size";
                    fwrite(error, 1, strlen(error), stderr);
                }
                BOOL allNulls = true;
                int i = 0;
                while((i < 1024) && allNulls)
                {
                    if(endBuffer[i] != '\0')  allNulls = false;
                    i++;
                }
                //if all 1024 characters were \0 then we've reached the end of the tar file
                if(allNulls == true) reading = false;
                else fseek(tarFile, -1024, SEEK_CUR);
            }
            fclose(tarFile);
        }
        else
        {
            perror("tar file could not be opened");
        }
    }
    else perror("Something went wrong");

    return 0;
}


