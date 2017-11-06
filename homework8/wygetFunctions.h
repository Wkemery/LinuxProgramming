/*
 * wygetFunctions.h
 * Author: Wyatt Emery
 * Date: OCT 29, 2016
 *
 * COSC 3750, Homework8
 *
 */

#include<stdio.h>
#include<sys/types.h>

extern void parseURL(char** host, char** pathRs, char** resource, const char* URL, char** errbuf);
    /* parseURL() takes in a standard URL and seperates out the hostname which must end in either .xx or .xxx.
     *HTTP:// and www. are optional however, will not be added in if left out. URL is not modified. errbuf
     * is only used if an error occurs. host, pathRs, and resource should be freed. and possibly errbuf. */

extern int parseResponse(char** header, char** body, const char* response, char** errbuf);
    /* parseResponse()  pulls the header and body out of a HTTP response message. the header only consists of
     * the first line and the body is everything after the two newlines that seperate the header and body.
     * errbuf will only be used in the event of an error and will contain an error message. header, body and
     * errbuf should be freed. It returns the number of characters in the body*/

extern FILE* NCCreateFile(const char* path);
    /*NCCreateFile creates a file at the given path without clobbering a file if it already exists.
     * Its naming scheme goes name.1 name.2 and so on until it finds an unused name */

extern FILE* opt_dSetup(const char* prefix, const char* URL, short opt_nc, char** errbuf);
    /* opt_dSetup() takes a specified prefix, a URL, and an opt_nc flag. It will create a directory tree
     * using prefix and URL. It will create the file at the end of URL. if URL ends in a / it will create
     * a file named index.html. it returns an open for writing FILE* to the file it created. If it executes
     * successfully *errbuf will be NULL. If not, errbuf will contain an error message. the data in errbuf
     * is malloc'ed and should be freed. */

extern void getHTMLDoc(char* host, const char* pathRs, FILE* outFile, char** errbuf);
    /* getHTMLDoc() takes a host, path/resource, an open outfile, and errbuf. It will create a TCP connection
     * to host on port 80 and send an HTTP GET message, requesting the specified path/resource. It calls
     * parseResponse() to seperate the header and body of the response message from the host. It will write
     * the entire body of the http message to outFile. It will not close outfile and It will put an error
     * message in errbuf if an error is encountered. If it executes successfully, *errbuf will be NULL. the
     * data in errbuf is malloc'ed and should be freed.*/

extern int createDir(const char* path, mode_t perms);
    /* creatDir() creates a directory at the specified path(if that path exists). It will remove a file of the same
     * name as the directory in order to create the directory. It will touch the directory if it already exists.
     * it will return -1 if an error occured. Mainly the path where a directory was supposed to be created did not
     * exist.*/

extern void createDirTree(const char*path);
    /* createDirTree() will create all directories along a path if they do not exist, or touch them if they do.
     * It will not create the file at the end of the path, if there is one. All directories along the way are
     * created with DEFAULT_PERMS(0000755) */