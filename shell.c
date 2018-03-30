#include "shell.h"

//Converts the command to params and return the number of parameters in the command
//Note : for extraction the command is at params[0] the parameters start after that

int _commandToParams(char *command, char **params, char split){
    int paramCount = 0;
    int count = 0;
    for(int i=0; i<strlen(command); i++){
        if(command[i] == split){
            if(split == ' '){
                params[paramCount][count] = '\0';
            }
            else if(split == '|'){
                count--;
                params[paramCount][count] = '\0'; 
            }
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

void _fixSpaces(char *str){
    if(str[0]!=' '){
        return;
    }
    int i;
    for(i=1; i<strlen(str); i++){
        str[i-1]= str[i]; // Just move all the characters one behind
    }
    str[i-1] = '\0';
}

void _flushParams(char **params){
    //memset didn't work, so iterating through all :(
    for(int i=0; i<MAX_PARAM_LENGTH;i++){
        for(int j=0; j<MAX_COMMAND_LENGTH;j++){
            params[i][j] = '\0';
        }
    }
}

int _search(char *command,char c){
    for(int i=0; i<strlen(command); i++){
        if(command[i] == c)
            return 1;
    }
    return -1;
}

void pipeThis(char **params, int paramCount){
    FILE *pipe_fp[paramCount+1];
    char readbuf[80];
    // popening the seperated commands and storing them in a pipe_file
    for(int i=0; i<=paramCount; i++){
        if(i==0){
            if (( pipe_fp[i] = popen(params[i], "r")) == NULL)
            {
                perror("popen");
                exit(1);
            }
        }
        else{
            if (( pipe_fp[i] = popen(params[i], "w")) == NULL)
            {
                perror("popen");
                exit(1);
            }
        }
    }
    //Processing the pipes by storing them into a buffer and putting them in the next piped command
    for(int i=0; i<=paramCount;i++){
        while(fgets(readbuf, 80, pipe_fp[i])){
            fputs(readbuf, pipe_fp[i+1]);
        }
    
    }
    //close all the pipes
    for(int i=0; i<=paramCount;i++){
        pclose(pipe_fp[i]);
    }
}


void _printPerm(struct stat mystat){
    printf((S_ISDIR(mystat.st_mode)) ? "d" : "-");
    printf((mystat.st_mode & S_IRUSR) ? "r" : "-");
    printf((mystat.st_mode & S_IWUSR) ? "w" : "-");
    printf((mystat.st_mode & S_IXUSR) ? "x" : "-");
    printf((mystat.st_mode & S_IRGRP) ? "r" : "-");
    printf((mystat.st_mode & S_IWGRP) ? "w" : "-");
    printf((mystat.st_mode & S_IXGRP) ? "x" : "-");
    printf((mystat.st_mode & S_IROTH) ? "r" : "-");
    printf((mystat.st_mode & S_IWOTH) ? "w" : "-");
    printf((mystat.st_mode & S_IXOTH) ? "x" : "-");
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
    else{
        perror("Invalid usage of command :");
    }
}


    // Commented out this implimentation as got execvp to work - may come back to this for regex (maybe not)
// void listDir(char **params, int paramCount){
//     DIR *dir;
//     struct dirent *dp;
//     struct stat mystat;
//     char *cwd = getcwd(buf,sizeof(buf));
//     dir = opendir(cwd);
//     if(paramCount == 0){
//         int count = 0;
//         while ((dp=readdir(dir)) != NULL) {
//             if(dp->d_name[0]!='.'){
//                 count ++;
//                 printf("%s\t\t",dp->d_name);
//             }
//             if(count==3){
//                 printf("\n");
//                 count = 0;
//             }
//         }
//         printf("\n");
//     }
//     else if(paramCount >= 1){
//         if(strcmp(params[1],"-l") == 0 || strcmp(params[1],"-lu") == 0 ){
//             while ((dp=readdir(dir)) != NULL) {
//                 if(dp->d_name[0]!='.'){ //  takes care of . , .. and hidden files
//                     stat(dp->d_name,&mystat);
                    
//                     struct passwd *pw = getpwuid(mystat.st_uid);
//                     struct group  *gr = getgrgid(mystat.st_gid);
//                     char *mtime;
//                     if(strcmp(params[1],"-l") == 0)
//                         mtime = ctime(&mystat.st_mtime);
//                     else if(strcmp(params[1],"-lu") == 0)
//                         mtime = ctime(&mystat.st_atime);
//                     strtok(mtime, "\n");
                    
//                     if(params[2][0]!='\0'){
//                         if(strcmp(dp->d_name,params[2]) == 0){
//                             _printPerm(mystat);
//                             printf(" %ld %s %s %ld %s %s",mystat.st_nlink,pw->pw_name,gr->gr_name,mystat.st_size,mtime,dp->d_name);
//                             break;
//                         }
//                     }
//                     else{
//                         _printPerm(mystat);
//                         printf(" %ld %s %s %ld %s %s",mystat.st_nlink,pw->pw_name,gr->gr_name,mystat.st_size,mtime,dp->d_name);

//                     }
//                     printf("\n");
//                 }
//             }
//             printf("\n");
//         }
//         else{
//             while ((dp=readdir(dir)) != NULL) {
//                 if(dp->d_name[0]!='.'){
//                     if(strcmp(dp->d_name,params[1]) == 0){
//                         printf("%s",dp->d_name);
//                     }
//                 }
//             }
//             printf("\n");
//         }
//     }
// }