/*
 * MyTar.h
 * Author: Wyatt Emery
 * Date: OCT 7, 2016
 *
 * COSC 3750, Homework6
 *
 * These functions help wytar.c accomplish its tasks. They do most of the work
 * for wytar
 */

#include<stdio.h>
#include<sys/types.h>

#ifndef TAR_HEADER
#define TAR_HEADER
struct TarHeader {
    char name[100];
    char mode[8];
    char uid[8];
    char gid[8];
    char size[12];
    char mtime[12];
    char checksum[8];
    char typeflag[1];
    char linkname[100];
    char magic[6];
    char version[2];
    char uname[32];
    char gname[32];
    char devmajor[8];
    char devminor[8];
    char prefix[155];
    char pad[12];
};

/*---------------------------Function Prototypes---------------------------*/

extern struct TarHeader* createTarH(char* filename);
/*
 * createTarH will return a pointer to a TarHeader struct created using
 * the information about the file, filename. if an error occurs it will return NULL.
 * it uses malloc to create a TarHeader and the data returned should be freed.
 * It will not print any errors but will set errno.
 */


extern struct TarHeader* importTarH(char* rawHeader);
/*
 * importTarH will return a pointer to a TarHeader struct created from the data read
 * in from a tar file given to the function via rawHeader.rawHeader must be 512 bytes or behavior is undefined.
 * this uses malloc to create a TarHeader and the data returned should be freed
 */


extern void tarDir(FILE* tarFile, struct TarHeader* tarHeader, char* dirPath);
/*
 * tarDir will recursively tar up a directory and its all subdirectories. It will create
 * a header for each object in the directories and write that header to the tar file passed to it.
 * dirPath is the path of the directory to be tared. tarHeader will be written out as well.
 */


extern void writeToTar(FILE* tarFile, struct TarHeader* tarHeader, char* filename);
/*
 * writeToTar will first write out the tarHeader passed. Then it will open filename and
 * read in all bytes from that file based on the size of the file. As it goes it will write
 * out every byte to tarFile. This will write complete 512 blocks to tarfile to keep the position
 * in the stream on even 512 byte blocks
 */

extern void writeToFile( FILE* tarFile, FILE* outFile, struct TarHeader* tarHeader);
/*
 * writeToFile writes out exactly size bytes specified in the tarheader. It writes to outfile
 * and reads from tarFile. It will read 512 byte blocks from tarfile so the position in the stream
 * is at the correct position for the next read on the tarfile
 */

extern FILE* createFile(struct TarHeader* tarHeader);
/*
 * createFile will create a file in the correct directory with the correct permissions specified
 * by tarHeader. It will create directories and links. If it creates one of those it will return NULL
 * if if creates a file, it will return a pointer to that open file. the FILE* returned should be closed.
 */

extern int createDir(char* path, mode_t perms);
/*
 * creatDir creates a directory at the specified path(if that path exists). It will remove a file of the same
 * name as the directory in order to create the directory. It will touch the directory if it already exists.
 * it will return -1 if an error occured. Mainly the path where a directory was supposed to be created did not
 * exist.
 */

extern void createDirTree(char* path);
/*
 * createDirTree will create all directories along a path if they do not exist, or touch them if they do.
 * It will not create the file at the end of the path, if there is one. All directories along the way are
 * created with DEFAULT_PERMS(0000755)
 */

extern int octToInt(char* string);
/*
 * octToInt takes in an octal value represented by an ASCII char* and converts it to an int and returns that int
 */

extern mode_t chToOct(char* string);
/*
 * chToOct takes an octal value represented by an ASCII char* and converts it to an octal mode_t that is returned and
 * can be passed to functions that need permission information
 */
#endif
