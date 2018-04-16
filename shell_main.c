/*
    Functionality implemted :
        clear
        cd
        Run shell commands using execvp
        piping
        history
        IO redirection
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
    char **history;
    history = (char **)malloc(25 * sizeof(char *));
    for(int i=0; i<MAX_PARAM_LENGTH; i++){
        history[i] = (char *)malloc(MAX_COMMAND_LENGTH * sizeof(char));
    }
    int hisIndex = 0;
    clr();
    _flushParams(params);
    while(1){
        infile_set = 0;
		outfile_set = 0;
        char *cwd = getcwd(buf,sizeof(buf));
        printf("\e[34m%s:\e[32m$\e[37m ",cwd); // A little overboard with colors I think?

        if(fgets(command, sizeof(command), stdin) == NULL) break;
        strtok(command, "\n"); // Remove trailing newline character

		int input_len = strlen(command);
        if(input_len==1){continue;} // Fix for segFault after 16 empty enters workaround, but figure out what's wrong
        // First check if input file specified
		int i = 0;

		while((i < input_len) && (command[i] != '<') ){i++;}
		
		// An error handler needs to be added below
		if( i != input_len ){ // An input file was specified
			infile_set = 1;

			command[i] = ' ';
			i++;

			while(command[i] == '\t' || command[i] == ' ' ) {i++;} // Skip whitespace
			// i now points to start of infile
			
			int j = i;
			while( (j < input_len) && (command[j] != '\t') && (command[j] != ' ') ){j++;} // Scroll till whitespace
			// The infile path now lies between [i, j[, its length is (j-i)
			
			strncpy(infile, command + i, (j-i)); // memset here???
			memset(command+i, ' ', (j-i));

		}
		//printf("INFILE: %s\n", infile);
		i = 0;
		while((i < input_len) && (command[i] != '>') ){i++;}
		outmode_out = 0;
		outmode_err = 0;
		outmode_app = 0;
		// An error handler needs to be added below
		if( i != input_len ){ // An input file was specified
			outfile_set = 1;
			
			command[i] =  ' ';
			i++;
			
			if(command[i-1] == '2') { outmode_err = 1; command[i-1] = ' '; }
			else if(command[i-1] == '&') { outmode_out = 1; command[i-1] = ' '; }
			else {outmode_out = 1;}

			if(command[i+1] == '>'){outmode_app = 1; command[i+1] = ' '; i++; }

			while(command[i] == '\t' || command[i] == ' ' ){i++;} // Skip whitespace
			// i now points to start of outfile
			
			int j = i;
			while( (j < input_len) && (command[j] != '\t') && (command[j] != ' ') ){j++;} // Scroll till whitespace
			// The outfile path now lies between [i, j[, its length is (j-i)
			

			strncpy(outfile, command + i, (j-i)); // memset here???
			memset(command+i, ' ', (j-i));
			//printf("OUTFILE: %s\n", outfile);
		}
		i = 0;

        strcpy(history[hisIndex++],command);
       	if(hisIndex == 25) 
       		hisIndex = 0;
        // The comparision with h is an offense must not do it!! but with lack of time and motivation in life :/
        if(_search(command,'|') == 1 && command[0] != 'h'){ // Handle piping of commands
            split = '|';
            int paramCount = _commandToParams(command,params,split);
            for(int i=1; i<=paramCount; i++){
                _fixSpaces(params[i]); // Those additional annoying spaces lingering after the | remove them if there
            }
            // for(int i=0; i<=paramCount; i++){
            //     printf("%s %ld\n",params[i],strlen(params[i]));
            // }
            pipeThis(params,paramCount,infile,outfile);
        }
        else{
            split = ' ';
            int paramCount = _commandToParams(command,params,split);
            if(infile[0]!='\0' || outfile[0]!='\0'){
                paramCount --; // handling an edge case in case of io redirection
            }
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
                else if(strcmp(params[0],"history")==0){
                	if(paramCount ==0){
		            	for( int i=0; i<hisIndex; i++){
				        		printf("[%d] %s\n",i+1,history[i]);
				        	}
                	}
		            else{
		            	//horrible way to do this but no time :/
		            	int index = _searchHis(history,command+15,hisIndex);
		            	if(index == -1)
		            		perror("command not found\n");
		            	else
		            		printf("[%d] %s\n",index+1,history[index]);
		            }	
                }
                else if(params[0][0] == '\n'){ // just the enter key pressed
                    continue;
                }
                else{
                    free(params[paramCount+1]);
                    params[paramCount+1] = NULL;
                    if(strcmp(params[0],"env") == 0)
                        setenv("SHELL","./shell",1);
                    pid_t pid;
                    if(pid=fork() == 0){
                        //child
                        int outFlag = -1;
                        int inFlag = -1;
                        int outfd;
                        int infd;
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

                        int status = execvp(params[0],params);
                        if(status ==  -1){
                            perror("Command not found");
                        }
                        if(outFlag)
                            close(outfd);
                        if(inFlag)
                            close(infd);
                    }
                    else{
                        wait(0);  
                        params[paramCount+1] = (char *)malloc(MAX_COMMAND_LENGTH * sizeof(char));
                        setenv("SHELL","/bin/bash",1);
                    }  
                }
            }
            _flushParams(params);
        }
        infile[0] = '\0';
        outfile[0] = '\0';
        command[0] = '\0';
    }
    return 0;
}

