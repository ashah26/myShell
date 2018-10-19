#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<unistd.h>
#include<sys/types.h>
#include<sys/wait.h>
#include<fcntl.h>


#define MAXLETTERS 100 //max number of letters supported
#define MAXCOMMANDS 100 //max number of commands supported

struct Node{
    int pid;
    char *cmd;
    struct Node *next;
};

void push(struct Node** head_ref, int pid, char *cmd)
{
    struct Node* new_node = (struct Node*) malloc(sizeof(struct Node));
    new_node->pid  = pid;
    new_node->cmd  = cmd;
    new_node->next = NULL;

    struct Node* last = (*head_ref);
    while (last->next != NULL)
        last = last->next;
    last->next = new_node;
}

void deleteNode(struct Node **head_ref, int pid)
{
    struct Node *prev = (*head_ref);
    struct Node *node = (*head_ref)->next;

    while(node!=NULL){
        if(node->pid == pid){
            prev->next = node->next;
            free(node);
            break;
        }
        prev = node;
        node = node->next;
    }
}

void printList(struct Node *head)
{
    // printf("\n printing linked list:::\n");
    struct Node *node = head->next;
    while (node != NULL)
    {
        printf(" pid: %d, cmd: %s \n ", node->pid, node->cmd);
        node = node->next;
    }
}

void cleanProcesses(struct Node **head, int killFlag){
    struct Node* node = *head;

    while(node != NULL){
        if(waitpid(node->pid, NULL, WNOHANG) == -1){
            //delete completed processes
            deleteNode(head, node->pid);
        }else{
        	//printf("In CleanProcess Else: Process still running %d\n", node->pid);
            if(killFlag){
                deleteNode(head, node->pid);
            }
        }
        node = node->next;
    }
}

void open_help(){
    puts("\n *** Welcome to help ****"
         "\n List of Commands Supported :"
         "\n >cd"
         "\n >ls"
         "\n >exit"
         "\n improper space handling");
}

//Define some of the commands which cannot work directly
int userDefine_cmd(char** parsedArgs, struct Node **head){

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
            cleanProcesses(head,1);
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

//parse the string and execute
void execArgs(char** parsedArgs, struct Node **head1, char *input_string){
    if(userDefine_cmd(parsedArgs,head1)){
        return;
    }
    int background = 0;

    //print background processes
    if(strcmp(parsedArgs[0], "processes") == 0){
        cleanProcesses(head1, 0);
        printList((*head1));
        return;
    }

    //detect background processes
    if(strcmp(parsedArgs[0], "bg") == 0){
        background = 1;
        for(int i = 0; parsedArgs[i] !=NULL; i++){
            parsedArgs[i] = parsedArgs[i+1];
        }
    }
    pid_t pid = fork();

    if(pid == -1){
        printf("\n Failed forking a child..");
        return;
    }
    else if(pid == 0){

        //if is IO redirection
        int file_i, in =0, out= 0;
        char inputArray[64], outputArray[64];

        for(file_i = 0; parsedArgs[file_i] != NULL; file_i++)
        {

            if(strcmp(parsedArgs[file_i], ">") == 0){
                parsedArgs[file_i] = NULL;
                strcpy(outputArray, parsedArgs[file_i+1]);
                out = 2;
            }
            else if (strcmp(parsedArgs[file_i], "<") == 0){
                parsedArgs[file_i] = NULL;
                strcpy(inputArray, parsedArgs[file_i+1]);
                in = 2;
            }
        }

        if(in){
            int fd0;
            if((fd0 = open(inputArray, O_RDONLY, 0)) < 0){
                perror("Could not open input file");
                exit(0);
            }
            dup2(fd0,0);
            close(fd0);
        }

        if(out){
            int fd1;
            if((fd1 = open(outputArray, O_WRONLY | O_CREAT, 0644)) < 0){
            }

            dup2(fd1, STDOUT_FILENO);
            close(fd1);
        }

        char *paths = getenv("PATH");

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

        for(int j=0; j<ndirectories;j++){
            char *thisDir = malloc(sizeof(char) *10000);

            for(int x=0; x<MAXCOMMANDS ; x++){
                thisDir[x] = directories[j][x];
            }

            strcat(thisDir,"/");
            strcat(thisDir,parsedArgs[0]);

            //printf("This Directory: %s\n%s\n%d\n))", thisDir, parsedArgs[0], access(thisDir,F_OK));

            if(access(thisDir,F_OK) >= 0){
                if(execv(thisDir, parsedArgs) < 0){
                    printf("\n Could not execute command...");
                }
                exit(0);
            }
        }
        printf("Not found in directories\n");
    }else{
        if(!background){
            wait(NULL);
        }else{
            printf("Inserting New Command\n");
            // cleanProcesses(head1, 0);
            push(head1 ,pid, input_string);
        }
        return;
    }
}


//Check if the command has space and then store each command
void parse_space(char* string, char** parsedArgs){
    int i;

    for(i=0; i< MAXLETTERS; i++){
        parsedArgs[i] = strsep(&string, " ");


        if(parsedArgs[i] == NULL){
            break;
        }
        if(strlen(parsedArgs[i]) == 0){
            i--;
        }
    }
}

//read input from user
int userInput(char* str)
{
    //initialize buffer to take the user input
    char c;
    int count = 0;
    char* buffer = (char *) malloc(sizeof(char) * 1024);

    printf("\n>>>> \n");
    while((c= getchar()) != '\n'){
        buffer[count++] = c;
    }
    buffer[count] = '\0';
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
    // int flag = 0;
    struct Node* head = NULL;
    head = (struct Node*)malloc(sizeof(struct Node));
    head->next = NULL;
    //Initialization of shell
    shell_init();

    while(1){

        //printing the current directory
        printDirectory();

        //Taking the user input

        if(userInput(input_string))
            continue;
        //parsing the user input
        char *passing_string = (char *) malloc(sizeof(char) * 1024);
        strcpy(passing_string, input_string);
        parse_space(input_string, parsedArgs);

        //process
        execArgs(parsedArgs, &head, passing_string);
    }
}