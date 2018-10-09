#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<unistd.h>
#include<sys/types.h>
#include<sys/wait.h>


#define MAXLETTERS 100 //max number of letters supported
#define MAXCOMMANDS 100 //max number of commands supported

//parse the string and execute
void execArgs(char** parsedArgs){

	pid_t pid = fork();

	if(pid == -1){
		printf("\n Failed forking a child..");
		return;
	}else if(pid == 0){

		char *paths = getenv("PATH");
		char *buffer;

		char *token;
		char **directories = malloc(sizeof(char) * 1024);
		const char s[1] = ":";
		int i = 0;
		token = strtok(paths,s);
		while(token != NULL){
			directories[i] = token;
			token = strtok(NULL,s);
			i++;


		}
		int ndirectories = i;
		char s1[1] = "/"; 

		for(int j=0; j<ndirectories;j++){
			char *thisDir = malloc(sizeof(char) *10000);
			for(int x=0; x<MAXCOMMANDS ; x++){
			 	thisDir[x] = directories[j][x];
			 }
			strcat(thisDir,s1);
			strcat(thisDir,parsedArgs[0]);
			//printf("This Directory: %s\n", thisDir);
			if(access(thisDir,F_OK) >= 0){
				if(execv(thisDir, parsedArgs) < 0){
					printf("\n Could not execute command...");
				}
				exit(0);
			}
			
		}
		printf("Not found in directories\n");

	}else{
		wait(NULL);
		return;
	}
}


//define help function
void open_help(){
	puts("\n *** Welcome to help ****"
		"\n List of Commands Supported :"
		"\n >cd"
		"\n >ls"
		"\n >exit"
		"\n improper space handling");
	return;
}

//Define some of the commands which cannot work directly
int userDefine_cmd(char** parsedArgs){

	int listCount = 3, i, switchArg = 0;
	char* listOfCmds[listCount];

	listOfCmds[0] = "cd";
	listOfCmds[1] = "exit";
	listOfCmds[2] = "help";

	for(i=0; i<listCount; i++){
		if(strcmp(parsedArgs[0], listOfCmds[i]) == 0){
			switchArg = i+1;
			break;
		}
	}

	switch(switchArg){
		case 1:
			chdir(parsedArgs[1]);
			return 1;
		case 2:
			printf("Bye! Have a nice day!\n");
			exit(0);
		case 3:
			open_help();
			return 1;
		default:
			break;
	}
	return 0;
}

//Check if the command has space and then store each command
void parse_space(char* string, char** parsedArgs){
	int i;

	for(i=0; i< MAXLETTERS; i++){
		parsedArgs[i] = strsep(&string, " ");

		//if in parsing we detect <> we need to redirect it elsewhere

		if(parsedArgs[i] == NULL){
			break;
		}
		if(strlen(parsedArgs[i]) == 0){
			i--;
		}
	}
}

//Process the string to kno which command it is
int process_string(char* string, char** parsedArgs)
{
	//See if the string has space in it
	parse_space(string, parsedArgs);

	//check if the command is one defined here or not
	if(userDefine_cmd(parsedArgs)){
		return 0;
	}else{
		return 1;
	}
}

//read input from user
int userInput(char* str)
{	
	//initialize buffer to take the user input
	char* buffer = (char *) malloc(sizeof(char) * 1024);
	
	printf("\n>>>>");

	int i = 0;
	while (1) {
		scanf("%c", &buffer[i]);
	    if (buffer[i] == '\n') {
	      break;
	    }
	    else {
	      i++;
	    }
	}
	buffer[i] = '\0';


	if(strlen(buffer) != 0){
		strcpy(str, buffer);
		return 0;
	}else{
		return 1;
	}
}

// print the current the working directory
void printDirectory()
{
	//initialize array to store current working directory
	char current_work_Dir[1024];
	//get the current working directory and print it
	getcwd(current_work_Dir, sizeof(current_work_Dir));
	printf("\n Dir: %s", current_work_Dir);
}

//Method to start the shell.
void shell_init()
{
	printf("\n\n\t *****My shell******\n");
	char* user_name = getenv("USER");
	printf("\n\n USER is: @%s",user_name );
	printf("\n");
}

//Main method
int main()
{
	char input_string[MAXCOMMANDS];
	char *parsedArgs[MAXLETTERS];
	int flag = 0;

	//Initialization of shell
	shell_init();

	while(1){

		//printing the current directory
		printDirectory();

		//Taking the user input
		if(userInput(input_string))
			continue;
		//parsing the user input
		flag = process_string(input_string, parsedArgs);

		//process
		if(flag == 1)
			execArgs(parsedArgs);
	}

	return 0;
}