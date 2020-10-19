#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>

#define CMDLINE_MAX 512

//Global Variable
int pipe_index = 0;

typedef struct{
    char* cmd;
    char** args;
    char* filename;
}Command;

typedef struct{
        Command *pipe1;
        Command *pipe2;
        Command *pipe3;
        Command *pipe4;
}Pipecmd;

int isRedirection(char* cmdString) {
        if (strchr(cmdString, '>') == NULL) {
                return 0;
        }
        return 1;
}

int isPipe(char* cmdString){
        if(strchr(cmdString, '|') == NULL){
                //There is not even one pipe sign in command
                return 0;
        }
        return 1;
}

void split_command(char* cmdString, Command* com){
        char* commandline = malloc(512 * sizeof(char));
        strcpy(commandline, cmdString);
        
        char* spaceDelimiter = " ";
        char* string;

        int index = 0;
        string = strtok(cmdString, spaceDelimiter);
        if (string != NULL) {
                strcpy(com->cmd, string);
        }

        while (string != NULL) {
                strcpy(com->args[index], string);
                string = strtok(NULL, spaceDelimiter);
                index++;
        }
        while (index < 16) {
                com->args[index] = NULL;
                index++;
        }
        free(commandline);
}

void split_command_redirection(char* cmdString, Command* com) {

        char* commandline = malloc(512 * sizeof(char));
        strcpy(commandline, cmdString);
        char* checkRedirection = malloc(512 * sizeof(char));
        strcpy(checkRedirection, cmdString);

        
        
        char* redirectionDelimiter = ">>";
        char* spaceDelimiter = " ";
        char* string;
        char* beforeRedirection;
        char* afterRedirection;

        if (isRedirection(checkRedirection)) { // Redirection is present
                beforeRedirection = strtok(commandline, redirectionDelimiter);
                afterRedirection = strtok(NULL, redirectionDelimiter);
                afterRedirection = strtok(afterRedirection, spaceDelimiter); // removes leading whitespace after '>' operator

                if (afterRedirection == NULL) { // Error management
                        com->filename = NULL;
                } else {
                strcpy(com->filename, afterRedirection);
                }

                int index = 0;
                string = strtok(beforeRedirection, spaceDelimiter);
                if (string != NULL) {
                        strcpy(com->cmd, string);
                }

                while (string != NULL) {
                        strcpy(com->args[index], string);
                        string = strtok(NULL, spaceDelimiter);
                        index++;
                }
                while (index < 16) {
                        com->args[index] = NULL;
                        index++;
                }
        } else { // NO redirection
                int index = 0;
                string = strtok(cmdString, spaceDelimiter);
                if (string != NULL) {
                        strcpy(com->cmd, string);
                }

                while (string != NULL) {
                        strcpy(com->args[index], string);
                        string = strtok(NULL, spaceDelimiter);
                        index++;
                }
                while (index < 16) {
                        com->args[index] = NULL;
                        index++;
                }
         }
        free(commandline);
        free(checkRedirection);
}

void split_pipe(char* cmdString, Pipecmd *pipe){
        char* commandline = malloc(512 * sizeof(char));
        strcpy(commandline, cmdString);

        char* pipeDelimiter = "|";
        char* spaceDelimiter = " ";
        char* string;

        char* pipe1Str = malloc(32 * sizeof(char));
        char* pipe2Str = malloc(32 * sizeof(char));
        char* pipe3Str = malloc(32 * sizeof(char));
        char* pipe4Str = malloc(32 * sizeof(char));

        int has3rdStr = 0;
        int has4thStr = 0;

        string = strtok(commandline, pipeDelimiter);
        if(string != NULL){
                strcpy(pipe1Str, string);
                printf("first: %s\n", pipe1Str);
                pipe_index++;
                string = strtok(NULL, pipeDelimiter);
                strcpy(pipe2Str, string);
                printf("second: %s\n", pipe2Str);
                string = strtok(NULL, pipeDelimiter);
                pipe_index++;
                string = strtok(NULL, pipeDelimiter); 
        }
        while(string != NULL){
                if(pipe_index == 2){
                        strcpy(pipe3Str, string);
                        printf("third: %s\n", pipe3Str);
                        pipe_index++;
                        has3rdStr = 1;
                        
                }
                if(pipe_index == 3){
                        strcpy(pipe4Str, string);
                        printf("fouth: %s\n", pipe4Str);
                        has4thStr = 1;
                }
                // if(pipe_index == 3){
                //         strcpy(pipe4Str, string);
                //         printf("fouth: %s\n", pipe4Str);
                // }
                string =strtok(NULL, pipeDelimiter);
                
        }
        split_command(pipe1Str, pipe->pipe1);
        printf("first command: %s\n", pipe->pipe1->cmd);
        split_command(pipe2Str, pipe->pipe2);
        printf("second command: %s\n", pipe->pipe2->cmd);

        if(has3rdStr == 1){
                split_command(pipe3Str, pipe->pipe3);
                printf("third command: %s\n", pipe->pipe3->cmd);
        }
        if(has4thStr == 1){
                split_command(pipe4Str, pipe->pipe4);
                printf("fouth command: %s\n", pipe->pipe4->cmd);
        }
        free(commandline);
        free(pipe1Str);
        free(pipe2Str);
        free(pipe3Str);
        free(pipe4Str);
}

int sshellSystem(Command* com, int redirectionFlag){
        pid_t pid;
        // char *args[] = {"sh","-c",cmdString, NULL};
        int exitStatus;

        pid = fork();

        if(pid == 0){
                // Child
                if (redirectionFlag) { // If there is redirection

                    if (com->filename == NULL) {
                        fprintf(stderr, "Error: no output file\n");
                        exit(1);
                    }

                    int fd;
                    fd = open(com->filename, O_WRONLY | O_CREAT | O_TRUNC, 0644);
                    if (fd == -1) {
                        fprintf(stderr, "Error: cannot open output file\n");
                        exit(1);
                    }
                    dup2(fd, STDOUT_FILENO);
                    close(fd);
                    execvp(com->cmd, com->args);
                    perror("execvp");
                    exit(1);
                } else { // No redirection
                    execvp(com->cmd, com->args);
                    perror("execvp");
                    exit(1);
                }

        }else if(pid > 0){
                // Parent
                int status;
                waitpid(pid, &status, 0);
                exitStatus = WEXITSTATUS(status);
        }else{
                // Fork error
                perror("fork");
                exit(1);
        }
        return exitStatus;
}

int builtin_exit(){
        fprintf(stderr, "Bye...\n");
        return 0;
}

int builtin_cd(Command* com){
        // Assume exactly one argument for cd
        chdir(com->args[1]);
        return 0;

}

int builtin_pwd(char* workingDirectory){
        fprintf(stderr, "%s\n", workingDirectory);
        return 0;
}



void print_completation(char* cmdCopy, int returnVal){
        fprintf(stderr, "+ completed '%s': [%d]\n",
                        cmdCopy, returnVal);
}

int main(void)
{
        char cmd[CMDLINE_MAX];

        while (1) {
                char *nl;
                int retval;


                /* Print prompt */
                /* Follow assignment instruction
                   print 'sshell@ucd$ ' when ready to accept input
                */
                printf("sshell@ucd$ ");
                fflush(stdout);

                /* Get command line */
                fgets(cmd, CMDLINE_MAX, stdin);

                /* Print command line if stdin is not provided by terminal */
                if (!isatty(STDIN_FILENO)) {
                        printf("%s", cmd);
                        fflush(stdout);
                }

                /* Remove trailing newline from command line */
                nl = strchr(cmd, '\n');
                if (nl){
                        *nl = '\0';
                }
                /*
                        Make a copy of cmd entered by user
                */
                int rawLen = strlen(cmd);
                char* cmdCopy = (char *) malloc(rawLen);
                strcpy(cmdCopy, cmd);


                /* Regular command */
                // retval = system(cmd);

                Command* com = NULL;
                com = malloc(sizeof(Command));
                com->cmd = malloc(32 * sizeof(char));
                com->args = malloc(16 * sizeof(char*));

                
                for (int i = 0; i < 16; i++) {
                    com->args[i] = malloc(32 * sizeof(char));
                }
                com->filename = malloc(32 * sizeof(char));

                Pipecmd* pipe = NULL;
                pipe = malloc(sizeof(Pipecmd));
                pipe->pipe1 = malloc(sizeof(Command));
                pipe->pipe1->cmd = malloc(32 * sizeof(char));
                pipe->pipe1->args = malloc(16 * sizeof(char*));
                for (int i = 0; i < 16; i++) {
                    pipe->pipe1->args[i] = malloc(32 * sizeof(char));
                }


                pipe->pipe2 = malloc(sizeof(Command));
                pipe->pipe2->cmd = malloc(32 * sizeof(char));
                pipe->pipe2->args = malloc(16 * sizeof(char*));
                for (int i = 0; i < 16; i++) {
                    pipe->pipe2->args[i] = malloc(32 * sizeof(char));
                }

                pipe->pipe3 = malloc(sizeof(Command));
                pipe->pipe3->cmd = malloc(32 * sizeof(char));
                pipe->pipe3->args = malloc(16 * sizeof(char*));
                for (int i = 0; i < 16; i++) {
                    pipe->pipe3->args[i] = malloc(32 * sizeof(char));
                }

                pipe->pipe4 = malloc(sizeof(Command));
                pipe->pipe4->cmd = malloc(32 * sizeof(char));
                pipe->pipe4->args = malloc(16 * sizeof(char*));
                for (int i = 0; i < 16; i++) {
                    pipe->pipe4->args[i] = malloc(32 * sizeof(char));
                }
                
                int redirectionFlag = 0;
                if(isRedirection(cmdCopy)){
                        redirectionFlag = 1;
                }

                int pipeFlag = 0;
                if(isPipe(cmdCopy)){
                        pipeFlag = 1;
                }
                
                if(pipeFlag == 1){
                        split_pipe(cmdCopy, pipe);
                } else if(redirectionFlag == 1 && pipeFlag == 0){
                        split_command_redirection(cmdCopy, com);
                } else if(pipeFlag == 0 && redirectionFlag == 0){
                        split_command(cmdCopy, com);
                }

                // int redirectionFlag = 0;
                // if (isRedirection(cmdCopy)) {
                //         redirectionFlag = 1;
                //         split_command_redirection(cmdCopy, com);
                // }
                
                // else{
                //         split_command(cmdCopy, com);
                // }


                

                

                if (!strcmp(com->cmd, "exit")) {
                        /* Builtin command */
                        // Exit command --- 'exit'
                        retval = builtin_exit();
                        print_completation(cmd, retval);
                        break;
                }else if(!strcmp(com->cmd, "cd")){
                        // Change directory --- 'cd'
                        retval = builtin_cd(com);
                        print_completation(cmd, retval);
                }else if(!strcmp(com->cmd, "pwd")){
                        // Print working directory command --- 'pwd'
                        char working_dir[80];
                        getcwd(working_dir, sizeof(working_dir));
                        retval = builtin_pwd(working_dir);
                        print_completation(cmd,retval);

                }else{
                        retval = sshellSystem(com, redirectionFlag);
                        // If retval returns 1, it means there was error
                        if (retval == 0) { // If there was no error
                            print_completation(cmd,retval);
                        }
                }

                // Free the memorry
                free(com);
                free(cmdCopy);
                pipe_index = 0;
        }

        return EXIT_SUCCESS;
}
