/*
	Functionality implemted :
		clear
		cd
		Run shell commands using execvp
		piping
		history
		IO redirection
		Wildcards
			Usage : ls *.c		// Matches all the c files
					ls *hell* 	// Matches all files that has hell in between like shell.c, shell_main.o etc
		Alias 
			Usage : alias <command to alias> <aliased command>
					alias show // to show the aliased 
		logs
			Usage :
				logs begin
				logs end
				logs show
*/
#include "shell.h"

int main(){
	char command[MAX_COMMAND_LENGTH];	// Command entered by the user
	char **params ;			  //Array of strings to store the split the command for processing 
	char **history;		   	 //Store the history
	char **originalList;  	// Array of strings to keep track of the original unaliased version
	char **aliasedList;	   // Array of strings to keep track of the original unaliased version
	char split = ' ';     // Default spilt of commands (' ')
	int hisIndex = 0;  	 // Index to keep track for the history elements
	int aliasIndex = 0; // index to keep track of number of aliasted elements
	int logFlag  = 0; // Flag to check if wished to be logged
	int logIndex = 0;// Index to keep track for the logs.

	struct timeval stop, start;
	struct logs myLog;
	
	// Can have 10 params of length 64
	params = (char **)malloc(MAX_PARAM_LENGTH * sizeof(char *));
	for(int i=0; i<MAX_PARAM_LENGTH; i++){
		params[i] = (char *)malloc(MAX_COMMAND_LENGTH * sizeof(char));
	}

	//Can have 25 commands each of length 64
	history = (char **)malloc(25 * sizeof(char *));
	for(int i=0; i<MAX_PARAM_LENGTH; i++){
		history[i] = (char *)malloc(MAX_COMMAND_LENGTH * sizeof(char));
	}

	//Can have 25 commands each of length 64
	originalList = (char **)malloc(25 * sizeof(char *));
	for(int i=0; i<MAX_PARAM_LENGTH; i++){
		originalList[i] = (char *)malloc(MAX_COMMAND_LENGTH * sizeof(char));
	}
	
	//Can have 25 commands each of length 64
	aliasedList = (char **)malloc(25 * sizeof(char *));
	for(int i=0; i<MAX_PARAM_LENGTH; i++){
		aliasedList[i] = (char *)malloc(MAX_COMMAND_LENGTH * sizeof(char));
	}
	_flushParams(originalList);
	_flushParams(aliasedList);

	char *str = malloc(100);

	char **aliaParams;
	//2 parameters of length 64
	aliaParams = (char **)malloc(2 * sizeof(char *));
	for(int i=0; i<MAX_PARAM_LENGTH; i++){
		params[i] = (char *)malloc(MAX_COMMAND_LENGTH * sizeof(char));
	}

	FILE *fp;
	char * line = NULL;
    size_t len = 0;
    ssize_t read;
	//_flushParams(aliaParams);
	fp = fopen(".alia","r");
	if(fp != NULL){
		while ((read = getline(&line, &len, fp)) != -1) {
			printf("Retrieved line of length %zu :\n", read);
			printf("%s", line);
			int flag = 0;
			for(int i=0, j=0; i<read; i++){
				
				if(!flag){
					if(line[i] == '=')
						flag = 1;
					else if(line[i] != '\n')
						originalList[aliasIndex][i] = line[i];
						 
					
				}
				else{
					//printf("ENtered\n");
					if(line[i]!='\n'){
						aliasedList[aliasIndex][j] = line[i];
						j++;
					}
					
				}
			}
			aliasIndex++;
		}
		for(int i=0; i<aliasIndex; i++){
			printf("%s\n%s\n",originalList[i],aliasedList[i]);
		}
		fclose(fp);
		if (line)
			free(line);
	}
	
	//Log data for upto 50 commands
	myLog.input = (char **)malloc(50 * sizeof(char *));
	for(int i=0; i<MAX_PARAM_LENGTH; i++){
			myLog.input[i] = (char *)malloc(MAX_COMMAND_LENGTH * sizeof(char));
	}

	myLog.user = (long *)malloc(50 * sizeof(long));

	myLog.timeTaken = (long *)malloc(50 * sizeof(long));

	clr();
	_flushParams(params);
	while(1){
		infile_set = 0;
		outfile_set = 0;
		char *cwd = getcwd(buf,sizeof(buf));
		printf("\e[32m%s:\e[34m$\e[37m ",cwd); // A little overboard with colors I think?

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
			memset(command+i, '\0', (j-i));

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
			memset(command+i, '\0', (j-i));
			//printf("OUTFILE: %s\n", outfile);
		}
		i = 0;
		i = _searchCommand(aliasedList,command,aliasIndex);
		if(i!=-1){
			strcpy(command,originalList[i]);
		}
		strncpy(history[hisIndex++],command,sizeof(command));
		   if(hisIndex == 25) 
			   hisIndex = 0;

		if(logFlag){
			strncpy(myLog.input[logIndex],command,sizeof(command));
			uid_t u = getuid();
			myLog.user[logIndex] = u;
		}
		if(_search(command,'|') == 1 && strncmp(command,"history",7) != 0 && strncmp(command,"alias",5) != 0  ){ // Handle piping of commands
			split = '|';
			int paramCount = _commandToParams(command,params,split);
			for(int i=1; i<=paramCount; i++){
				_fixSpaces(params[i]); // Those additional annoying spaces lingering after the | remove them if there
			}
			if(logFlag){
				gettimeofday(&start, NULL);
			}
			pipeThis(command,params,paramCount,infile,outfile);
			if(logFlag){
				gettimeofday(&stop, NULL);
				myLog.timeTaken[logIndex] = stop.tv_usec - start.tv_usec;
				logIndex ++;
			}
		}
		else{
			split = ' ';
			if(command[strlen(command)-1]==' '){
				command[strlen(command)-1] = '\0'; // Fixing the extra space issue
			}
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
				if((strncmp(params[0],"clear",5) == 0 || strncmp(params[0],"cls",3) == 0) && paramCount == 0){
					clr();
				}
				else if(strncmp(params[0],"cd",2)==0){
					changeDir(params,paramCount);
				}
				else if(strncmp(params[0],"exit",4)==0){
					printf("exit\n");
					exit(0);
				}
				else if(strcmp(params[0],"ls")==0){
				    listDir(params,paramCount,infile,outfile);
				}
				else if(strncmp(params[0],"history",7)==0){
					if(paramCount ==0){
						for( int i=0; i<hisIndex; i++){
								printf("[%d] %s\n",i+1,history[i]);
							}
					}
					else{
						//Horrible way to do this but no time :/
						int index = _searchCommand(history,command+15,hisIndex);
						if(index == -1)
							perror("command not found\n");
						else
							printf("[%d] %s\n",index+1,history[index]);
					}	
				}
				else if(strncmp(params[0],"alias",5) == 0){
					if(aliasIndex == 24){
						printf("Max Alias Limit Reached\n");
						continue;
					}
					if(paramCount == 0){
						printf("Invalid Usage of command\nUsage :\n\t alias <original> <alias>\n\t alias show\n");
						continue;
					}
					else if(paramCount == 1){
						if(strncmp(params[1],"show",4) == 0){
							if(aliasIndex != 0){
								printf("%15s%15s\n","Original","Aliased");
								for(int i=0; i<aliasIndex; i++){
									printf("%15s%15s\n",originalList[i],aliasedList[i]);
								}
							}
							else{
								printf("Empty List\n");
							}

						}
						else{
							printf("Invalid Usage of command\n");
							continue;
						}
					}
					else{
						int flag = 0;
						char *temp = (char *)calloc(64 , sizeof(char));
						for(int i=1; i<paramCount; i++){
							strcat(temp,params[i]);
							strcat(temp," ");
						}
						if(aliasIndex!=0){
							int in = _searchCommand(aliasedList,params[paramCount],aliasIndex);
							if(in != -1){
								printf("Alias Already exists\n");
								flag = 0;
							}
							else{
								strcpy(originalList[aliasIndex],temp);
								strcpy(aliasedList[aliasIndex],params[paramCount]);
								printf("%15s%15s\n","Original","Aliased");
								printf("%15s%15s\n",originalList[aliasIndex],aliasedList[aliasIndex]);
								aliasIndex ++;	
								flag = 1;
							}
						}
						else{
							strcpy(originalList[aliasIndex],temp);
							strcpy(aliasedList[aliasIndex],params[paramCount]);
							printf("%15s%15s\n","Original","Aliased");
							printf("%15s%15s\n",originalList[aliasIndex],aliasedList[aliasIndex]);
							aliasIndex ++;	
							flag = 1;
						}
						free(temp);
						temp = NULL;
						int aliaFile = open(".alia", O_RDWR | O_CREAT | O_APPEND, S_IRUSR | S_IWUSR);  
						if(flag){
							strcpy(str,originalList[aliasIndex-1]);
							strcat(str,"=");
							strcat(str,aliasedList[aliasIndex-1]);
							strcat(str,"\n");
							printf("%s",str);
							long len = strlen(str);
							write(aliaFile,originalList[aliasIndex-1],strlen(originalList[aliasIndex-1]));
							write(aliaFile,"=",1);
							write(aliaFile,aliasedList[aliasIndex-1],strlen(aliasedList[aliasIndex-1]));
							write(aliaFile,"\n",1);
							for(int i=0; i<100; i++){
								str[i] = '\0';
							}
						}
						close(aliaFile);
						//_flushParams(aliasedList);
						//_flushParams(originalList);
					}
				}
				else if(strncmp(params[0],"logs",3) == 0){
					if(strncmp(params[1],"begin",5) == 0){
						if(!logFlag){
							logFlag = 1;
							printf("\tLogging \e[32mBegins\n");
						}
						else{
							printf("Already Logging\n");
						}
					}
					else if(strncmp(params[1],"end",3) == 0){
						logFlag = 0;
						printf("\tLogging \e[31mEnds\n");
					}
					else if(strncmp(params[1],"show",4) == 0){
						if(logIndex>0){
							printf("%15s%15s%25s\n","User","Command","Time Taken(ms)");
							for(int i=0; i<logIndex; i++){
								printf("%15ld%25s%15ld\n",myLog.user[i],myLog.input[i],myLog.timeTaken[i]);
							}
						}
						else{
							printf("No logs to show.\n");
						}
					}
					else{
						printf("Invalid Usage of logs\n");
					}
				}
				else if(params[0][0] == '\n'){ // just the enter key pressed
					continue;
				}
				// else if(strcmp(params[0],"editor") == 0){
				//     if(!fork()){
				//         execvp(params[0],params);
				//     }
				//     else{
				//         wait(0);
				//     }
				// }
				else{
					free(params[paramCount+1]);
					params[paramCount+1] = NULL;
					if(strncmp(params[0],"env",3) == 0)
						setenv("SHELL","./shell",1);
					if(logFlag){
						gettimeofday(&start, NULL);
					}
					pid_t pid;
					if(pid=fork() == 0){
						//child
						int outFlag = 0;
						int inFlag = 0;
						int outfd;
						int infd;
						int tempfd;
						if(outfile[0]!='\0'){
							outFlag = 1;
							outfd = open(outfile, O_RDWR | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
							dup2(outfd, 1);
						}
						if(infile[0]!='\0'){
							inFlag = 1;
							infd = open(infile, O_RDWR | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
							dup2(infd, 0);
						}
						 if(!fork()){ //write to a temporary file will be used for network comm
							tempfd = open(".temp", O_RDWR | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
							write(tempfd,"Given command : ",16);
							write(tempfd, command, input_len);
							write(tempfd,"\n",1);
							dup2(tempfd, 1);
						}
						else{
							wait(0);
						}
						// for(int i=0; i<=paramCount ; i++){
						//     printf("%s\n",params[i]);
						// }
						//printf("%s\n",params[paramCount+1]);
						//params[paramCount+1] = NULL;
						int status = execvp(params[0],params);
						if(status ==  -1){
							perror("Command not found");
						}
						if(outFlag)
							close(outfd);
						if(inFlag)
							close(infd);
						close(tempfd);
					}
					else{
						wait(0);  
						if(logFlag){
							gettimeofday(&stop, NULL);
							myLog.timeTaken[logIndex] = stop.tv_usec - start.tv_usec;
							logIndex ++;
						}
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

