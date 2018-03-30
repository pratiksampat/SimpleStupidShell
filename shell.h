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

#ifndef SHELL_H
#define SHELL_H
#define MAX_COMMAND_LENGTH  64
#define MAX_PARAM_LENGTH    16
#define MAX_PIPES           10

char buf[MAX_COMMAND_LENGTH]; //Generic buffer used in multiple places

int _commandToParams(char *command, char **params, char split);
void _fixSpaces(char *str);
void _flushParams(char **params);
int _search(char *command,char c);

void pipeThis(char **params, int paramCount);
void _printPerm(struct stat mystat);
void clr();
void changeDir(char **params, int paramCount);

#endif