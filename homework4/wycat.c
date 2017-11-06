/*
 * wycat.c
 * Author: Wyatt Emery
 * Date: Sep 29, 2016
 *
 * COSC 3750, Homework4
 *
 * Description
 * */

#include<stdio.h>
#include<errno.h>
#include<string.h>

int main (int argc, char** argv)
{
    char buffer[4096];

    if(argc<=1)
    {
        while(fgets(buffer, sizeof buffer, stdin)!=0)
        {
            int byteCount=0;
            while(buffer[byteCount]!=0)
            {
                byteCount++;
            }
            fwrite(buffer, 1, byteCount, stdout);
        }
    }
    else
    {
	int i;
        for( i=1; i<argc; i++)
        {
            int strcmpReturn=strcmp(argv[i],"-");

            if(strcmpReturn==0)
            {
                while(fgets(buffer, sizeof buffer, stdin)!=0)
                {
                    int byteCount=0;
                    while(buffer[byteCount]!=0)
                    {
                        byteCount++;
                    }
                fwrite(buffer, 1, byteCount, stdout);
                }
            }
            else
            {
                FILE *infile=fopen(argv[i], "r");
                if(infile==NULL)
                {
                    char error[100]="wycat:";
                    strcat(error, argv[i]);
                    perror(error);
                }
                else
                {
                    size_t bytecount = 1;
                    while(bytecount!=0)
                    {
                        bytecount=fread(buffer, 1, sizeof buffer, infile);
                        fwrite(buffer, 1, bytecount, stdout);
                    }
                    fclose(infile);
                }
            }
        }
    }
    return 0;
}
