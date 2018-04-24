#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <dirent.h>
#include <sys/stat.h>
#include <time.h>
#include <pwd.h>
#include <grp.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <sys/time.h>

#ifndef SHELL_H
#define SHELL_H
#define MAX_COMMAND_LENGTH  64
#define MAX_PARAM_LENGTH    16
#define MAX_PIPES           10

#define LINE_SZ 8192 // # of characters allowed per line; We dont allow multi-line commands (yet)

#define CALL system
char input_buffer[LINE_SZ]; // Holds each line read in. memset this after every line? 

char infile[1024];
int infile_set; // Used as boolean flag
//int in_fd = STDIN_FILENO;

char outfile[1024];
int outfile_set; // Used as boolean flag
//int out_fd = STDOUT_FILENO;

int outmode_out;
int outmode_err;
int outmode_app;

//char delims[] = " \t";

char buf[MAX_COMMAND_LENGTH]; //Generic buffer used in multiple places

struct logs{
	char **input;
	long *timeTaken;
	long *user;
};

int _commandToParams(char *command, char **params, char split);
void _fixSpaces(char *str);
void _flushParams(char **params);
int _search(char *command,char c);
int _searchCommand(char **history, char *command,int len);

void pipeThis(char *command, char **params, int paramCount, char *infile, char *outfile);
void _printPerm(struct stat mystat);
void clr();
void changeDir(char **params, int paramCount);
void call(char *command);
void listDir(char **params, int paramCount, char *infile, char *outfile);
int wildMatch(char *str, char *pattern, int n, int m);
void _writePerm(struct stat mystat,int outfd);

#endif
