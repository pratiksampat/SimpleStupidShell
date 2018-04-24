#include "shell.h"

//Converts the command to params and return the number of parameters in the command
//Note : for extraction the command is at params[0] the parameters start after that

int _commandToParams(char *command, char **params, char split){
	int paramCount = 0;
	int count = 0;
	for(int i=0; i<strlen(command); i++){
		if(command[i] == split && command[i-1]!=command[i]){
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
		else if( command[i] == split && command[i-1]==command[i]){
			i++;
			i--; // WTF? weird does not work if I remove this 
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


int _searchCommand(char **history, char *command,int len){
	for(int i=0; i<len; i++){
		if(strcmp(history[i],command)==0)
			return i;
	}
	return -1;
}

void pipeThis(char *command, char **params, int paramCount, char *infile, char *outfile){
	int flag = -1;
	char **tempParams;
	tempParams = (char **)malloc(MAX_PARAM_LENGTH * sizeof(char *));
	for(int i=0; i<MAX_PARAM_LENGTH; i++){
		tempParams[i] = (char *)malloc(MAX_COMMAND_LENGTH * sizeof(char));
	}
	// for(int i=0; i<paramCount;i++){
	// 	printf("~%s~\n",params[i]);
	// }
	// printf("~~~~~~~~~~~~~~~~\n");
	if(flag==1 && !fork()){ // This is neccessary as execvp is a system call and we need the control back after it's done
		char split = ' ';
		int outFlag = 0;
		int inFlag = 0;
		int outfd;
		int infd;
		//Handle all the pipes
		for(int i=0; i<paramCount; i++){
			int pd[2];
			pipe(pd);
			int tempParamCount = _commandToParams(params[i],tempParams,split);
			tempParams[tempParamCount+1] = NULL;
			// printf("INNER TPC: %d\n", tempParamCount);
			// for(int i=0; i<=tempParamCount;i++){
			// 	printf("~%s~\n",tempParams[i]);
			// }
			if (!fork()) {
					dup2(pd[1], 1);
					if(outfile[0]!='\0'){
						perror("OUT\n");
						outFlag = 1;
						outfd = open(outfile, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
						dup2(outfd, pd[1]); // output it to a file
					}
					if(infile[0]!='\0'){
						perror("IN\n");
						inFlag = 1;
						infd = open(infile, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
						dup2(infd, pd[0]);
					}
					int status = execvp(tempParams[0],tempParams);
					if(status == -1){
						perror("No such command.");
					}
					if(outFlag)
						close(outfd);
					if(inFlag)
						close(infd);
				
			}
			else{
				wait(0);
				dup2(pd[0], 0);
				close(pd[1]);
			}
			
		}
		// _flushParams(tempParams);
		int tempParamCount = _commandToParams(params[paramCount],tempParams,split);
		tempParams[tempParamCount] = NULL;
		//printf("TPC: %d\n", tempParamCount);
		// printf("~~~~~~~~~~~~~~~~\n");
		// for(int i=0; i<tempParamCount;i++){
		// 		printf("~%s~\n",tempParams[i]);
		// }
		if(outfile[0]!='\0'){
			outFlag = 1;
			outfd = open(outfile, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
			dup2(outfd, 1);
		}
		if(infile[0]!='\0'){
			inFlag = 1;
			infd = open(infile, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
			dup2(infd, 0);
		}
		int status = execvp(tempParams[0],tempParams);
		if(status == -1){
			perror("No such command.");
		}
		if(outFlag)
			close(outfd);
		if(inFlag)
			close(infd);
	}
	else{
		wait(0);
	}
	if(flag == -1){
		call(command);
	}
	_flushParams(params);
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

void call(char *command){
	CALL(command);
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
void listDir(char **params, int paramCount){
    DIR *dir;
    struct dirent *dp;
    struct stat mystat;
    char *cwd = getcwd(buf,sizeof(buf));
    dir = opendir(cwd);
    if(paramCount == 0){
        int count = 0;
        while ((dp=readdir(dir)) != NULL) {
            if(dp->d_name[0]!='.'){
                count ++;
                printf("%s\t",dp->d_name);
            }
            // if(count==3){
            //     printf("\n");
            //     count = 0;
            // }
        }
        printf("\n");
    }
    else if(paramCount >= 1){
        if(strcmp(params[1],"-l") == 0 || strcmp(params[1],"-lu") == 0 ){
            while ((dp=readdir(dir)) != NULL) {
                if(dp->d_name[0]!='.'){ //  takes care of . , .. and hidden files
                    stat(dp->d_name,&mystat);
					
                    struct passwd *pw = getpwuid(mystat.st_uid);
                    struct group  *gr = getgrgid(mystat.st_gid);
                    char *mtime;
                    if(strcmp(params[1],"-l") == 0)
                        mtime = ctime(&mystat.st_mtime);
                    else if(strcmp(params[1],"-lu") == 0)
                        mtime = ctime(&mystat.st_atime);
                    strtok(mtime, "\n");
					
                    if(params[2][0]!='\0'){
                        if(wildMatch(dp->d_name,params[2],strlen(dp->d_name),strlen(params[2])) == 1){
                            _printPerm(mystat);
                            printf(" %ld %s %s %ld %s %s\n",mystat.st_nlink,pw->pw_name,gr->gr_name,mystat.st_size,mtime,dp->d_name);
                        }
                    }
                    else{
                        _printPerm(mystat);
                        printf(" %ld %s %s %ld %s %s\n",mystat.st_nlink,pw->pw_name,gr->gr_name,mystat.st_size,mtime,dp->d_name);

                    }
                }
            }
            printf("\n");
        }
        else{
            while ((dp=readdir(dir)) != NULL) {
                if(dp->d_name[0]!='.'){
                    if(wildMatch(dp->d_name,params[1],strlen(dp->d_name),strlen(params[1])) == 1){
                        printf("%s\n",dp->d_name);
                    }
                }
            }
        }
    }
}

int wildMatch(char *str, char *pattern, int n, int m){
	if(m==0){
		return n==0;
	}
	int lookup[n+1][m+1];
	memset(lookup, 0, sizeof(lookup));

	lookup[0][0] = 1;
	// Only '*' can match with empty string
	for (int j = 1; j <= m; j++){
		if (pattern[j - 1] == '*')
        	lookup[0][j] = lookup[0][j - 1];
	}
	// fill the table in bottom-up fashion
    for (int i = 1; i <= n; i++)
    {
        for (int j = 1; j <= m; j++)
        {
            // Two cases if we see a '*'
            // a) We ignore ‘*’ character and move
            //    to next  character in the pattern,
            //     i.e., ‘*’ indicates an empty sequence.
            // b) '*' character matches with ith
            //     character in input
            if (pattern[j - 1] == '*')
                lookup[i][j] = lookup[i][j - 1] ||
                               lookup[i - 1][j];
 
            // Current characters are considered as
            // matching in two cases
            // (a) current character of pattern is '?'
            // (b) characters actually match
            else if (pattern[j - 1] == '?' ||
                    str[i - 1] == pattern[j - 1])
                lookup[i][j] = lookup[i - 1][j - 1];
 
            // If characters don't match
            else lookup[i][j] = 0;
        }
    }
 
    return lookup[n][m];
     
}