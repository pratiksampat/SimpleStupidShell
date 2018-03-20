/*
    Functionality implemted :
        clear
        cd
        ls <-l | -lu> <file/dirname>
        run executables from the cwd
*/

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
#include "shell.h"
int main(){
    char command[MAX_COMMAND_LENGTH];
    char **params ;

    // Can have 10 params of length 1024
    params = (char **)malloc(MAX_PARAM_LENGTH * sizeof(char *));
    for(int i=0; i<MAX_PARAM_LENGTH; i++){
        params[i] = (char *)malloc(MAX_COMMAND_LENGTH * sizeof(char));
    }
    _flushParams(params);
    while(1){
        char *cwd = getcwd(buf,sizeof(buf));
        printf("\e[34m%s:\e[32m$ ",cwd); // A little overboard with colors I think?

        if(fgets(command, sizeof(command), stdin) == NULL) break;
        strtok(command, "\n"); // Remove trailing newline character
        int paramCount = _commandToParams(command,params);
        if(paramCount < 0){
            continue;
        }
        else{
            //printf("Total Number of parameters : %d\n",paramCount);
            //printf("The parameters are : \n");
            // for(int i=0; i<=paramCount; i++){
            //     printf("[%d] %s\n",i,params[i]);
            // }
            if(strcmp(params[0],"clear") == 0 && paramCount == 0){
                clr();
            }
            else if(strcmp(params[0],"cd")==0){
                changeDir(params,paramCount);
            }
            else if(strcmp(params[0],"exit")==0){
                printf("exit\n");
                exit(0);
            }
            else if(strcmp(params[0],"ls")==0){
                listDir(params,paramCount);
            }
            else if(params[0][0] == '\n'){ // just the enter key pressed
                continue;
            }
            else{
                //printf("Function not implemented yet\n");
                pid_t pid;
                if(pid=fork() == 0){
                    //child
                    int status = execl(params[0],params[0],(char *)0);
                    if(status ==  -1){
                        perror("Exec Failed");
                    }
                }
                else{
                    wait(0);
                }
            }
            
            _flushParams(params);
        }
    }
    return 0;
}