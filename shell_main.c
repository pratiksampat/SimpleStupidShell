/*
    Functionality implemted :
        clear
        cd
        Run shell commands using execvp
        piping
*/
#include "shell.h"

int main(){
    char command[MAX_COMMAND_LENGTH];
    char **params ;
    char split = ' ';
    // Can have 10 params of length 1024
    params = (char **)malloc(MAX_PARAM_LENGTH * sizeof(char *));
    for(int i=0; i<MAX_PARAM_LENGTH; i++){
        params[i] = (char *)malloc(MAX_COMMAND_LENGTH * sizeof(char));
    }
    _flushParams(params);
    while(1){
        char *cwd = getcwd(buf,sizeof(buf));
        printf("\e[34m%s:\e[32m$\e[37m ",cwd); // A little overboard with colors I think?

        if(fgets(command, sizeof(command), stdin) == NULL) break;
        strtok(command, "\n"); // Remove trailing newline character
        
        if(_search(command,'|') == 1){ // Handle piping of commands
            split = '|';
            int paramCount = _commandToParams(command,params,split);
            for(int i=1; i<=paramCount; i++){
                _fixSpaces(params[i]); // Those additional annoying spaces lingering after the | remove them if there
            }
            pipeThis(params,paramCount);
        }
        else{
            split = ' ';
            int paramCount = _commandToParams(command,params,split);
            if(paramCount < 0){
                continue;
            }
            else{
                //printf("Total Number of parameters : %d\n",paramCount);
                //printf("The parameters are : \n");
                // for(int i=0; i<=paramCount; i++){
                //     printf("[%d] %s\n",i,params[i]);
                // }
                if((strcmp(params[0],"clear") == 0 || strcmp(params[0],"cls") == 0) && paramCount == 0){
                    clr();
                }
                else if(strcmp(params[0],"cd")==0){
                    changeDir(params,paramCount);
                }
                else if(strcmp(params[0],"exit")==0){
                    printf("exit\n");
                    exit(0);
                }
                // else if(strcmp(params[0],"ls")==0){
                //     listDir(params,paramCount);
                // }
                else if(params[0][0] == '\n'){ // just the enter key pressed
                    continue;
                }
                else{
                    free(params[paramCount+1]);
                    params[paramCount+1] = NULL;
                    //printf("Function not implemented yet\n");
                    pid_t pid;
                    if(pid=fork() == 0){
                        //child
                        int status = execvp(params[0],params);
                        if(status ==  -1){
                            perror("Command not found");
                        }
                                    
                    }
                    else{
                        wait(0);  
                        params[paramCount+1] = (char *)malloc(MAX_COMMAND_LENGTH * sizeof(char));
                    }  
                }
            }
        }
        _flushParams(params);
    }
    return 0;
}

