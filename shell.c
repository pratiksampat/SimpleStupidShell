/*
    Functionality implemted :
        clear
        cd
*/

#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<string.h>
#include<errno.h>

#define MAX_COMMAND_LENGTH 1024
#define MAX_PARAM_LENGTH    10

char buf[MAX_COMMAND_LENGTH]; //Generic buffer used in multiple places

//Converts the command to params and return the number of parameters in the command
//Note : for extraction the command is at params[0] the parameters start after that
//TODO : Handling multiple commands using the (|)
int commandToParams(char *command, char **params){
    int paramCount = 0;
    int count = 0;
    for(int i=0; i<strlen(command); i++){
        if(command[i] == ' '){
            paramCount++;
            count = 0;
        } 
        else{
            params[paramCount][count] = command[i];
            count++;
        }
    }
    return paramCount;
}

void flushParams(char **params){
    //memset didn't work, so iterating through all :/
    for(int i=0; i<MAX_PARAM_LENGTH;i++){
        for(int j=0; j<MAX_COMMAND_LENGTH;j++){
            params[i][j] = '\0';
        }
    }
}

/********************************************************/
/*                    Terminal Commands                 */
/********************************************************/

void clr(){
    printf("\e[1;1H\e[2J"); // Got this from SO. works!👍
}

void changeDir(char **params, int paramCount){
    int success;
    if(paramCount == 1){
        success = chdir(params[1]);
        if(success == -1){
            perror("cd Error");
        }
    }
    else if(paramCount == 0){
        success = chdir("/home");
        if(success == -1){
            perror("cd Error");
        }
    }
}
int main(){
    char command[MAX_COMMAND_LENGTH];
    char **params ;

    // Can have 10 params of length 1024
    params = (char **)malloc(MAX_PARAM_LENGTH * sizeof(char *));
    for(int i=0; i<MAX_PARAM_LENGTH; i++){
        params[i] = (char *)malloc(MAX_COMMAND_LENGTH * sizeof(char));
    }
    flushParams(params);
    while(1){
        char *cwd = getcwd(buf,sizeof(buf));
        printf("\e[34m%s:\e[32m$ ",cwd); // A little overboard with colors I think?

        if(fgets(command, sizeof(command), stdin) == NULL) break;
        strtok(command, "\n"); // Remove trailing newline character
        int paramCount = commandToParams(command,params);
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
            else{
                printf("Function not implemented yet\n");
            }
            
            flushParams(params);
        }
    }
    return 0;
}