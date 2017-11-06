/*
 * TarFunctions.c
 * Author: Wyatt Emery
 * Date: OCT 7, 2016
 *
 * COSC 3750, Homework6
 *
 * These are the function definitions for MyTar.h
*/
#define BUFFSIZE 100
#define BLOCKSIZE 512
#define STDBUFF 4096
#define DEFAULT_PERMS 0000755

#include"MyTar.h"
#include<stdio.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<unistd.h>
#include<stdlib.h>
#include<string.h>
#include<tar.h>
#include<pwd.h>
#include<grp.h>
#include<math.h>
#include<fcntl.h>
#include<errno.h>
#include<dirent.h>


typedef unsigned char BYTE;

struct TarHeader* createTarH(char* filename)
{
    struct TarHeader* tarHeader = (struct TarHeader*) malloc(BLOCKSIZE);
    memset(tarHeader, 0, BLOCKSIZE);
    struct stat fileStats;
    int statRet = lstat(filename, &fileStats);
    if(statRet != 0)
    {
        errno = ENOENT;
        return NULL;
    }
    char buffer[BUFFSIZE];
    memset(buffer, 0, BUFFSIZE);
    int i = 0;
    /*---------------------------write filename to Tar Header---------------------------*/
    if(strlen(filename) > 256)
    {
        errno = ENAMETOOLONG;
        perror(filename);
        return NULL;
    }


    if((strlen(filename) > 100))
    {
        short looking = 1;
        i = strlen(filename) - 1;
        int k = 0;
        while(looking && (i >= 0))
        {
            if(filename[i] == '/' && (i <= 155))
            {
                //found the slash where it can be broken and the prefix will not be larger than 155 characters
                looking = 0;
                if(S_ISDIR(fileStats.st_mode) && ( k > 99))
                {
                    errno = ENAMETOOLONG;
                   // char error[STDBUFF] = "Directory names must be less than 100 characters: ";
                    //strcat(error, filename);
                    //perror(error);
                    return NULL;
                }
                memcpy(tarHeader->prefix, filename, (i));
                memcpy(tarHeader->name, &filename[i+1], (strlen(filename) - i - 1));
            }
            k++;
            i--;
        }
    }
    else
    {
        memcpy(tarHeader->name, filename, strlen(filename));
    }

    /*---------------------------write permissions to Tar Header---------------------------*/

    memset(buffer, 0, BUFFSIZE);
    snprintf(buffer, 8, "%o", fileStats.st_mode);
    buffer[0] = '0';
    for(i = 0; i < (7 - strlen(buffer)); i++)
    {
        tarHeader->mode[i] = '0'; //zero pad the permissions
    }
    strcat(tarHeader->mode, buffer);
    memset(tarHeader->mode, '0', 4);

    /*---------------------------write uid and gid to Tar Header---------------------------*/
    memset(buffer, 0, BUFFSIZE);
    snprintf(buffer, 8, "%o", fileStats.st_uid);

    for(i = 0; i < (7 - strlen(buffer)); i++)
    {
        tarHeader->uid[i] = '0'; //zero pad the uid
    }
    strcat(tarHeader->uid, buffer);

    memset(buffer, 0, BUFFSIZE);
    snprintf(buffer, 8, "%o", fileStats.st_gid);

    for(i = 0; i < (7 - strlen(buffer)); i++)
    {
        tarHeader->gid[i] = '0'; //zero pad the gid
    }
    strcat(tarHeader->gid, buffer);

    /*---------------------------write mtime to Tar Header---------------------------*/
    memset(buffer, 0, BUFFSIZE);
    snprintf(buffer, 12, "%lo", fileStats.st_mtim.tv_sec);
    for(i = 0; i < (11 - strlen(buffer)); i++)
    {
        tarHeader->mtime[i] = '0';//zero pad the time
    }
    strcat(tarHeader->mtime, buffer);

    /*---------------------------write typeflag to Tar Header---------------------------*/
    if(S_ISDIR(fileStats.st_mode))
    {
        tarHeader->typeflag[0] = DIRTYPE;
        // if directory name is less than 99 characters append a / and a \0
        if(strlen(tarHeader->name) < 99) strcat(tarHeader->name, "/");
        //if direcotry name is = to 99 characters append only a /
        else if (strlen(tarHeader->name) == 99) tarHeader->name[99] = '/';
    }
    else if(S_ISLNK(fileStats.st_mode)) tarHeader->typeflag[0] = SYMTYPE;
    else tarHeader->typeflag[0] = REGTYPE;

    /*---------------------------write size to Tar Header---------------------------*/
    if((tarHeader->typeflag[0] == DIRTYPE) || (tarHeader->typeflag[0] == SYMTYPE)) memset(tarHeader->size, '0', 11);
    else
    {
        memset(buffer, 0, BUFFSIZE);
        snprintf(buffer, 12, "%lo", fileStats.st_size);
        for(i = 0; i < (11 - strlen(buffer)); i++)
        {
            tarHeader->size[i] = '0';//zero pad the size
        }
        strcat(tarHeader->size, buffer);
    }

    /*---------------------------write linkname to Tar Header---------------------------*/
    if(tarHeader->typeflag[0] == SYMTYPE)
    {
        char lnkdFile[101];
        memset(lnkdFile, 0, 101);
        readlink(filename, lnkdFile, sizeof lnkdFile);
        if(lnkdFile[100] == '\0') // the path was less than 101 characters
        {
            memcpy(tarHeader->linkname, lnkdFile, strlen(lnkdFile));
        }
        else //linkname was over 100 chars and link cannot be stored in tar
        {
            errno = ENAMETOOLONG;
            return NULL;
        }
    }

    /*---------------------------write uname and gname to Tar Header---------------------------*/

    struct passwd* passwdFile;
    struct group* grpFile;
    passwdFile=getpwuid(fileStats.st_uid);
    grpFile=getgrgid(fileStats.st_gid);
    if((passwdFile!= NULL) && (strlen(passwdFile->pw_name) <= 32))
    {
        memcpy(tarHeader->uname, passwdFile->pw_name, strlen(passwdFile->pw_name));
    }
    if((grpFile!= NULL) && (strlen(passwdFile->pw_name) <=32))
    {
        memcpy(tarHeader->gname, grpFile->gr_name, strlen(grpFile->gr_name));
    }

    /*---------------------------write magic to Tar Header---------------------------*/
    if((tarHeader->uname[0] != '\0') && (tarHeader->gname != '\0')) strcpy(tarHeader->magic, TMAGIC);

    /*---------------------------write version to Tar Header---------------------------*/
    memcpy(tarHeader->version, TVERSION, 2);

    /*---------------------------write devmajor, devminor to Tar Header---------------------------*/
    memset(tarHeader->devmajor, '0', 7);
    memset(tarHeader->devminor, '0', 7);

    /*---------------------------write checkksum to Tar Header---------------------------*/
    memset(tarHeader->checksum, 32, 8); //set checksum to all spaces
    //calculate checksum
    BYTE* p = (BYTE*)&tarHeader->name;
    unsigned int sum = 0;
    for(i = 0; i < BLOCKSIZE; i++)
    {
        sum+=*(p+i);
    }

    memset(buffer, 0, BUFFSIZE);
    snprintf(buffer, 6, "%o", sum);
    char checksum[BUFFSIZE];
    memset(checksum, 0, BUFFSIZE);
    for(i = 0; i < (6 - strlen(buffer)); i++)
    {
        checksum[i] = '0';//zero pad the checksum
    }
    strcat(checksum, buffer);
    memcpy(tarHeader->checksum, checksum, 7);

    return tarHeader;
}

void tarDir(FILE* tarFile, struct TarHeader* tarHeader, char* dirPath)
{
    if(tarHeader != NULL) writeToTar(tarFile, tarHeader, dirPath); //write out tarheader with dir info first
    else perror("Null TarHeader");

    struct dirent* dirEntry;
    DIR* directory=opendir(dirPath);
    if(directory!= NULL) dirEntry = readdir(directory);
    else perror("Null directory");
    while(dirEntry != NULL)
    {
        if((strcmp(dirEntry->d_name, ".")!=0) && (strcmp(dirEntry->d_name, "..")!=0))
        {
            char thisPath[STDBUFF];
            memset(thisPath, 0, STDBUFF);
            memcpy(thisPath, dirPath, strlen(dirPath));
            strcat(thisPath, "/");
            strcat(thisPath, dirEntry->d_name);
            struct stat fileStats;
            lstat(thisPath, &fileStats);
            if(S_ISDIR(fileStats.st_mode))
            {
                //append a / to thisPath
                struct TarHeader* dirHeader = createTarH(thisPath);
                if(dirHeader == NULL)
                {
                    perror("Null tarheader");
                    return;
                }
                else tarDir(tarFile, dirHeader, thisPath);
            }
            else
            {
                struct TarHeader* fileHeader = createTarH(thisPath);
                if(fileHeader == NULL)
                {
                    perror("Error null Tarheader");
                    return;
                }
                else writeToTar(tarFile, fileHeader, thisPath);
            }
        }
        dirEntry = readdir(directory);
    }
    closedir(directory);
}

void writeToTar(FILE* tarFile, struct TarHeader* tarHeader, char* filename)
{
    if(tarHeader == NULL)
    {
        perror("Null Tarheader");
        return;
    }
    else if(tarFile == NULL)
    {
        perror("could not access tarfile");
        return;
    }

    char buffer[BLOCKSIZE];
    //write out tar header first
    memset(buffer, 0, BLOCKSIZE);
    memcpy(buffer, tarHeader, BLOCKSIZE);
    int fwrRet = fwrite(buffer, 1, BLOCKSIZE, tarFile);
    if(fwrRet != BLOCKSIZE) perror("lost access to TarFile");
    memset(buffer, 0, BLOCKSIZE);
    FILE* file = fopen(filename, "r");
    if(file!= NULL)
    {
        if(tarHeader->typeflag[0] == REGTYPE)
        {
            //read in contents of file, write out contents of file
            int size = octToInt(tarHeader->size);
            int blocks = size/BLOCKSIZE;
            int rmndr = size%BLOCKSIZE;
            int i = 0;

            if(size >= BLOCKSIZE)
            {
                for(i = 0; i < blocks; i++)
                {
                    //read in 512 bytes, write out 512 bytes
                    int frdRet = fread(buffer, 1, BLOCKSIZE, file);
                    if(frdRet != BLOCKSIZE) perror("lost access to File");
                    int fwrRet = fwrite(buffer, 1, BLOCKSIZE, tarFile);
                    if(fwrRet != BLOCKSIZE) perror("lost access to TarFile");
                    memset(buffer, 0, BLOCKSIZE);
                }
            }
            if(rmndr != 0)
            {
                int frdRet = fread(buffer, 1, rmndr, file); //read in remaining bytes
                if(frdRet != rmndr) perror("lost access to File");
                int fwrRet = fwrite(buffer, 1, BLOCKSIZE, tarFile); //write out those bytes and finish the block
                if(fwrRet != BLOCKSIZE) perror("lost access to TarFile");
            }
        }
        fclose(file);
    }
    
}

void writeToFile( FILE* tarFile, FILE* outFile, struct TarHeader* tarHeader)
{
        char buffer[BLOCKSIZE];
        memset(buffer, 0, BLOCKSIZE);
        int k = 0;
        int size = octToInt(tarHeader->size);
        int blocks = size/BLOCKSIZE;
        int rmndr = size%BLOCKSIZE;

        if(size >= BLOCKSIZE)
        {
            for(; k < blocks; k++)
            {
                //read in 512 bytes, write out 512 bytes
                int frdRtn = fread(buffer, 1, BLOCKSIZE, tarFile);
                if(frdRtn < 512)
                {
                    perror("lost access to tar file");
                    return;
                }
                int fwrRtn = fwrite(buffer, 1, BLOCKSIZE, outFile);
                if(fwrRtn < 512)
                {
                    char error[STDBUFF] = "lost access to ";
                    strcat(error, tarHeader->name);
                    perror(error);
                    return;
                }
                memset(buffer, 0, sizeof buffer); // reset buffer just in case
            }
        }
        if(rmndr > 0)
        {
            int frdRet = fread(buffer, 1, BLOCKSIZE, tarFile); // read in full block 512 bytes
            if(frdRet != BLOCKSIZE) perror("lost access to TarFile");
            int fwrRet = fwrite(buffer, 1, rmndr, outFile); //write out remaining bytes
            if(fwrRet != rmndr) perror("lost access to File");
        }
}

struct TarHeader* importTarH(char* rawHeader)
{
    struct TarHeader* ret = (struct TarHeader*) malloc(BLOCKSIZE);
    memcpy(ret, rawHeader, BLOCKSIZE);

    return ret;
}

extern FILE* createFile(struct TarHeader* tarHeader)
{
    FILE* file = NULL;

    char filename[257];
    memset(filename, 0, 257);

    //if there is no prefix value, just use name
    if(tarHeader->prefix[0] == '\0') strncpy(filename, tarHeader->name, 101);
    else
    {
        //size 257 because maximum filename size is 256, plus a '\0'
        strncpy(filename, tarHeader->prefix, 156);
        strcat(filename, "/");
        strncat(filename, tarHeader->name, 100);
    }
    //get permissions
    mode_t perms = chToOct(tarHeader->mode);

    //if the file is a dir
    if(tarHeader->typeflag[0] == DIRTYPE)
    {
        //create directory
        int crtDirRet = createDir(filename, perms);
        if(crtDirRet != 0) createDirTree(filename);

    }
    else if(tarHeader->typeflag[0] == SYMTYPE) // link
    {
        int smlnkRet = symlink(tarHeader->linkname, filename);
        if(smlnkRet != 0)
        {
            if(errno == EEXIST)
            {
                remove(filename);
                smlnkRet = symlink(tarHeader->linkname, filename);
            }
            else if(errno == ENOENT)
            {
                //directory where trying to create link does not exist
                createDirTree(filename);//create directory(s)
                smlnkRet = symlink(tarHeader->linkname, filename);//create link
            }
        }
        if(smlnkRet != 0) perror("symlink failed!");
    }
    else //treat as regular file
    {
        //create file and open file
        int crtRet = creat(filename, perms);
        if(crtRet == -1)
        {
            if(errno == ENOENT)
            {
                //Directory where trying to create file does not exist.
                createDirTree(filename); //create directory(s)
                crtRet = creat(filename, perms); // create file
            }
        }
        if(crtRet == -1)
        {
            char error[STDBUFF] = "an error occured when trying to create file: ";
            strcat(error, filename);
            perror(error);
            return NULL;
        }
        file = fopen(filename, "w");
    }
        return file;
}


int createDir(char* path, mode_t perms)
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

void createDirTree(char*path)
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

int octToInt(char* string)
{
    int i = 0;
    int size = strlen(string);
    int digits[15];
    memset(digits, 0, sizeof(int) * 15);
    int total = 0;
    for(i = 0; i < size; i++)
    {
        digits[i] = string[size - i - 1] - 48;
    }
    for(i = 0; i < size; i++)
    {
        total+= digits[i] * pow(8, i);

    }
    return total;
}

mode_t chToOct(char* string)
{
    int i = 0;
    int size = strlen(string);
    int digits[15];
    memset(digits, 0, sizeof(int) * 15);
    mode_t total = 0;
    for(i = 0; i < size; i++)
    {
        digits[i] = string[size - i - 1] - 48;
    }
    for(i = 0; i < size; i++)
    {
        total+= digits[i] * pow(8, i);

    }
    return total;
}

