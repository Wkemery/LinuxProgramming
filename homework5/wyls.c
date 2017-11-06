/*
wycat.c
Author: Wyatt Emery
Date: Sep 30, 2016

COSC 3750, Homework4

Description
*/

#include<stdio.h>
#include<errno.h>
#include<string.h>
#include<unistd.h>
#include<stdlib.h>
#include<errno.h>
#include<sys/stat.h>
#include<dirent.h>
#include<pwd.h>
#include<grp.h>
#include<time.h>


void listDir(const char* dir, const short option_n, const short option_h);
void longList(const char* filename, const char* filePath, const short option_n, const short option_h);
void longListcwd(const short option_n, const short option_h);

int main (int argc, char** argv)
{
    short option_n=0;
    short option_h=0;
    
    if(argc < 2) longListcwd(0,0); //No Arguments, long list cwd  
    else //process arguments
    {
        int processOptions=1;
        int processParameters=0;
        int i = 1;
        for(; i<argc; i++) //for every argument
        {
            if(processOptions)
            {
                //if current arg starts with -, its an option
                if(argv[i][0]=='-')
                {
                    //process multiple options in this argument if there is more than 1
                    int j=1;
                    char currentOption = argv[i][j];
                    while(currentOption!='\0')
                    {
                        if(currentOption=='n') option_n=1; //set option_n flag
                        else if(currentOption=='h') option_h=1; // set option_h flag
                        else
                        {
                            errno=EINVAL; //set errno to invalid argument
                            perror("That was not a valid option: Exiting now");
                            exit(EXIT_FAILURE);
                        }
                        j++;
                        currentOption=argv[i][j];
                    }
                }
                //This argurment does not begin with -, Done processing Options
                else
                {
                    processOptions=0;
                    processParameters=1;
                }
            }
            
            //process parameters
            if(processParameters)
            {
                //get stats on current argument
                struct stat fileStats;
                int statReturn=stat(argv[i], &fileStats);
                if(statReturn==0) //if stat successfully got stats
                {
                    //if current arg is a dir
                    if (S_ISDIR(fileStats.st_mode)) listDir(argv[i], option_n, option_h); //long list items in dir
                    else longList(argv[i],argv[i], option_n, option_h); //long list info on current arg
                }
                else //error opening file
                {
                    char error[100]="unable to retrieve stats on ";
                    strcat(error,argv[i]);
                    perror(error);
                }
            }
        }
        
        // if there were options but no parameters
        if(!processParameters) 
        {
	    // long list the current dir with the set options
            longListcwd(option_n, option_h);
        }
    }
    return 0;
}


//----------------------------Functions--------------------------------

/*
 * 
 */
void longList(const char* filename,const char* filePath, const short option_n, const short option_h)
{
    struct stat fileStats;
    struct stat linkStats;
    lstat(filePath, &linkStats);
    //for links
    int statReturn=stat(filePath, &fileStats); 
    short isLink = 0;
    
    //if stat opened file successfully
    if(statReturn==0)
    {
        //print permissions
        if(S_ISDIR(fileStats.st_mode)) printf("d");
        else if(S_ISLNK(linkStats.st_mode))
        {
            isLink = 1;
            printf("l");
        }
        else printf("-");
        if((fileStats.st_mode & S_IRUSR) == S_IRUSR) printf("r");//check for u read
        else printf("-"); 
        if((fileStats.st_mode & S_IWUSR) == S_IWUSR) printf("w");//check for u write
        else printf("-"); 
        if((fileStats.st_mode & S_IXUSR) == S_IXUSR) printf("x");//check for u execute
        else printf("-"); 
        if((fileStats.st_mode & S_IRGRP) == S_IRGRP) printf("r");//check for g read
        else printf("-"); 
        if((fileStats.st_mode & S_IWGRP) == S_IWGRP) printf("w");//check for g write
        else printf("-"); 
        if((fileStats.st_mode & S_IXGRP) == S_IXGRP) printf("x");//check for g execute
        else printf("-"); 
        if((fileStats.st_mode & S_IROTH) == S_IROTH) printf("r");//check for o read
        else printf("-"); 
        if((fileStats.st_mode & S_IWOTH) == S_IWOTH) printf("w");//check for o write
        else printf("-"); 
        if((fileStats.st_mode & S_IXOTH) == S_IXOTH) printf("x ");//check for o execute
        else printf("- "); 
//------------------------------------------------------------------------------------------------       
        //if option-n print username and group name
        if(option_n)
        {
            struct passwd* passwdFile;
            struct group* grpFile;
            passwdFile=getpwuid(fileStats.st_uid);
            grpFile=getgrgid(fileStats.st_gid);
            printf("%s %s ",passwdFile->pw_name, grpFile->gr_name);
        }
        else // print uid and gid
        {
            printf("%d %d ", fileStats.st_uid, fileStats.st_gid);
        }
//------------------------------------------------------------------------------------------------       
        //if option-h print size as human readable
	if(isLink && option_h) printf("%6ld ", linkStats.st_size);
	else if(isLink && !option_h) printf("%7ld ", linkStats.st_size); 
        else if(option_h)
        {
            int rmndr=0;
            char amount=0;
            //if size is greater than 1 GB
            double hSize = 0;
            if(fileStats.st_size>1073741824) 
            {
                //divide by 1 GB=2^30 bytes
                amount='G';
                hSize=fileStats.st_size/1073741824.0;
                rmndr=fileStats.st_size%1073741824;
            }
            //else if size is greater than 1 MB
            else if(fileStats.st_size>1048576.0)
            {
                //divide by 1 MB=2^20 bytes
                amount='M';
                hSize=fileStats.st_size/1048576.0;
                rmndr=fileStats.st_size%1048576;
            }
            //else if size is greater than 1KB
            else if(fileStats.st_size>1024)
            {
                //divide by 1KB=2^10 bytes
                amount='K';
                hSize=fileStats.st_size/1024.0;
                rmndr=fileStats.st_size%1024;
            }
            
            if(amount==0)
            {
                //size did not need to be converted
                printf("%6ld ", fileStats.st_size);
            }
            
            /*
	     * this strange equation is to hopefully prevent a file with some remainder printing with 
	     * 1 decimal place and that decimal place is a zero.
	     * for example, if a file had a size of 18.00555K it would print out as 18.0K
	     * this should only allow a decimal to print if the decimal value is greater than .04
	     * and so will round up to .1, Although its not perfect.
	     */
            else if(((double)rmndr/fileStats.st_size) < .003) 
            {
		//print without decimals if decimal value insignificant eg <.04
                printf("%5.0f%c ", hSize, amount);
            }
            else //else print at 1 decimal place
            {
                printf("%5.1f%c ", hSize, amount);
            }
        }
        // not -h print size as is
        else printf("%7ld ", fileStats.st_size);
//------------------------------------------------------------------------------------------------
        //print last modified time
        char timeOutput[100];
        memset(timeOutput, 0, sizeof timeOutput);
        time_t currentTime;
        time(&currentTime);
        time_t* theTime = &fileStats.st_mtim.tv_sec;
        struct tm* fileTime = localtime(theTime);
        
        //if older than 180 days only print day and year, omit time
        if(*theTime < (currentTime - 15552000)) strftime(timeOutput, sizeof timeOutput, "%b %e  %Y ", fileTime);
        else strftime(timeOutput, sizeof timeOutput, "%b %e %H:%M ", fileTime);
        printf("%s", timeOutput);
//------------------------------------------------------------------------------------------------        
        //print filename or file linked to
        if(isLink)
        {
            char lnkdFile[100];
            memset(lnkdFile, 0, sizeof lnkdFile);
            readlink(filename, lnkdFile, sizeof lnkdFile);
            printf("%s -> %s\n", filename, lnkdFile);
        }
        else printf("%s\n", filename);  
    }
    
    else // stat could not open the file.
    {
        char error[100]= "Error: could not open ";
        strcat(error, filename);
        perror(error);
    } 
}

/*
 * 
 */
void listDir(const char* dir, const short option_n, const short option_h)
{
    struct dirent *dirEntry;
    DIR *directory=opendir(dir);
    char* path = strdup(dir);
    strcat(path,"/");
    if(directory!=NULL)
    {
        dirEntry=readdir(directory);
        while(dirEntry!=NULL)
        {
            //printf("%s\n", dirEntry->d_name);
            if((strcmp(dirEntry->d_name, ".")!=0) && (strcmp(dirEntry->d_name, "..")!=0))
            {
                char cwd[4096];
                memset(cwd, 0, sizeof cwd);
                getcwd(cwd, sizeof cwd);
                if(strcmp(cwd, dir)!=0)
                {
                    char* filePath = strdup(path);
                    strcat(filePath, dirEntry->d_name);
                    //printf("%s\n", filePath);
                    longList(dirEntry->d_name,filePath,option_n,option_h);
                    free(filePath);
                    filePath=0;
                }
                else
                {
                    longList(dirEntry->d_name, dirEntry->d_name, option_n, option_h);
                }
            }
            
           dirEntry=readdir(directory);
        }
        closedir(directory);
    }
    else perror("Could not open directory");
}

/*
 * 
 */
void longListcwd(const short option_n, const short option_h)
{
    char cwd[4096];
    memset(cwd, 0, sizeof cwd);
    getcwd(cwd, sizeof cwd);
    //printf("%s\n", path);
    listDir(cwd, option_n, option_h);
}
