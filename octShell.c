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
int mislocatedOut = 0;

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
        char* string;

        char* pipe1Str = malloc(32 * sizeof(char));
        char* pipe2Str = malloc(32 * sizeof(char));
        char* pipe3Str = malloc(32 * sizeof(char));
        char* pipe4Str = malloc(32 * sizeof(char));

        int has3rdStr = 0;
        int has4thStr = 0;
        if(strncmp(cmdString, pipeDelimiter, 1) == 0){
                pipe_index++;
                strcpy(pipe->pipe1->cmd, "NULLPIPE");
                return;
        } else {
                string = strtok(commandline, pipeDelimiter);
                if(string != NULL){
                        strcpy(pipe1Str, string);
                        pipe_index++;
                        if(isRedirection(pipe1Str) == 1){
                                mislocatedOut = 1;
                        }
                        if(strcmp(pipe1Str, " ") == 0){
                                strcpy(pipe->pipe1->cmd, "NULLPIPE");
                        }
                        string = strtok(NULL, pipeDelimiter);
                        if(string == NULL){
                                strcpy(pipe->pipe2->cmd, "NULLPIPE");
                        } else {
                                strcpy(pipe2Str, string);
                                if(strcmp(pipe2Str, " ") == 0){
                                        strcpy(pipe->pipe2->cmd, "NULLPIPE");
                                }
                                string = strtok(NULL, pipeDelimiter);
                                
                        }
                }
                        
        }
                
        while(string != NULL){
                if(pipe_index == 1){
                        if(string == NULL){
                                strcpy(pipe->pipe3->cmd, "NULLPIPE");
                                break;
                        }
                        strcpy(pipe3Str, string);
                        pipe_index++;
                        if(isRedirection(pipe1Str) == 1 || isRedirection(pipe2Str) == 1){
                                mislocatedOut = 1;
                                break;
                        }
                        if(strcmp(pipe3Str, " ") == 0){
                                strcpy(pipe->pipe3->cmd, "NULLPIPE");
                                break;
                        } else {
                                has3rdStr = 1;
                                string =strtok(NULL, pipeDelimiter);
                                if(string == NULL){
                                        strcpy(pipe->pipe4->cmd, "NULLPIPE");
                                        break;
                                }
                        }
                        
                }
                if(pipe_index == 2){
                        pipe_index++;
                        if(isRedirection(pipe1Str) == 1 || isRedirection(pipe2Str) == 1 || isRedirection(pipe3Str) == 1){
                                mislocatedOut = 1;
                                break;
                        }
                        strcpy(pipe4Str, string);
                        if(strcmp(pipe4Str, " ") == 0){
                                strcpy(pipe->pipe4->cmd, "NULLPIPE");
                                break;
                        } else {
                                has4thStr = 1;
                                break;
                        }
                        
                }

        }
        split_command(pipe1Str, pipe->pipe1);
        
        // isMislocatedOutput(pipe1Cpy);
        if(has3rdStr == 0){
                split_command_redirection(pipe2Str, pipe->pipe2);
                
        }
        if(has4thStr == 0 && has3rdStr == 1){
                split_command(pipe2Str, pipe->pipe2);
                split_command_redirection(pipe3Str, pipe->pipe3);
        }
        
        if(has4thStr == 1){
                split_command(pipe2Str, pipe->pipe2);
                split_command(pipe3Str, pipe->pipe3);
                split_command_redirection(pipe4Str, pipe->pipe4);
                
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

int* sshellSystem_pipe(Pipecmd* cmdPipe, int redirectionFlag){
        pid_t pipe1Pid;
        pid_t pipe2Pid;
        pid_t pipe3Pid;
        pid_t pipe4Pid;

        int first_fd[2];
        int second_fd[2];
        int third_fd[2];

        static int exitStatus[4];
        int totalPipeSiganl = pipe_index;
        int status[4];
        if(totalPipeSiganl == 1){
                pipe(first_fd);

                pipe1Pid = fork();
                if(pipe1Pid == 0){

                        close(first_fd[0]); // Close the pipe input
                        dup2(first_fd[1], STDOUT_FILENO);
                        close(first_fd[1]);
                        execvp(cmdPipe->pipe1->cmd, cmdPipe->pipe1->args);
                        perror("execvp");
                        exit(1);

                } else if(pipe1Pid != 0){
                        waitpid(pipe1Pid, &status[0],0);
                        exitStatus[0] = WEXITSTATUS(status[0]);
                }
                pipe2Pid = fork();
                if(pipe2Pid == 0){

                        if (redirectionFlag) { // If there is redirection
                                close(first_fd[1]);
                                dup2(first_fd[0], STDIN_FILENO);
                                close(first_fd[0]);

                                if (cmdPipe->pipe2->filename == NULL) {
                                        fprintf(stderr, "Error: no output file\n");
                                        exit(1);
                                }

                                int fdirection;
                                fdirection = open(cmdPipe->pipe2->filename, O_WRONLY | O_CREAT | O_TRUNC, 0644);
                                if (fdirection == -1) {
                                        fprintf(stderr, "Error: cannot open output file\n");
                                        exit(1);
                                }
                                dup2(fdirection, STDOUT_FILENO);
                                close(fdirection);
                                close(first_fd[1]);
                                dup2(first_fd[0], STDIN_FILENO);
                                close(first_fd[0]);
                                execvp(cmdPipe->pipe2->cmd, cmdPipe->pipe2->args);
                                perror("execvp");
                                exit(1);
                        } else { // No redirection
                                close(first_fd[1]);
                                dup2(first_fd[0], STDIN_FILENO);
                                close(first_fd[0]);
                                execvp(cmdPipe->pipe2->cmd, cmdPipe->pipe2->args);
                                perror("execvp");
                                exit(1);
                        }
                

                        
                } else if (pipe2Pid != 0){
                        close(first_fd[0]); // Totally close the pipe
                        close(first_fd[1]); // Totally close the pipe
                        waitpid(pipe2Pid, &status[1],0);
                        exitStatus[1] = WEXITSTATUS(status[1]);
                }
        } else if(totalPipeSiganl == 2){
                pipe(first_fd);
                pipe(second_fd);
                pipe1Pid = fork();
                if(pipe1Pid == 0){
                        //Child 1
                        // printf("processing left\n");
                        close(first_fd[0]);
                        dup2(first_fd[1], STDOUT_FILENO);
                        close(first_fd[1]);
                        // close(fd[1]);
                        // printf("left cmd :%s\n", cmdPipe->pipe1->cmd);
                        // write(fd[1], send, 7);
                        execvp(cmdPipe->pipe1->cmd, cmdPipe->pipe1->args);
                        perror("execvp");
                        exit(1);
                } else if(pipe1Pid != 0){
                        //parent
                        waitpid(pipe1Pid, &status[0],0);
                        exitStatus[0] = WEXITSTATUS(status[0]);
                }
                
                pipe2Pid = fork();
                if(pipe2Pid == 0){
                        close(first_fd[1]);
                        dup2(first_fd[0], STDIN_FILENO);
                        close(first_fd[0]);

                        close(second_fd[0]);
                        dup2(second_fd[1], STDOUT_FILENO);
                        close(second_fd[1]);
                        execvp(cmdPipe->pipe2->cmd, cmdPipe->pipe2->args);
                        perror("execvp");
                        exit(1);
                } else if (pipe2Pid != 0){
                        // waitpid(pipe1Pid, &status[0],0);
                        close(first_fd[0]);
                        close(first_fd[1]);
                        waitpid(pipe2Pid, &status[1],0);
                        // exitStatus[0] = WEXITSTATUS(status[0]);
                        exitStatus[1] = WEXITSTATUS(status[1]);
                }
                pipe3Pid = fork();
                if(pipe3Pid == 0){
                        if (redirectionFlag) { // If there is redirection
                                close(second_fd[1]);
                                dup2(second_fd[0], STDIN_FILENO);
                                close(second_fd[0]);

                                if (cmdPipe->pipe3->filename == NULL) {
                                        fprintf(stderr, "Error: no output file\n");
                                        exit(1);
                                }

                                int fdirection;
                                fdirection = open(cmdPipe->pipe3->filename, O_WRONLY | O_CREAT | O_TRUNC, 0644);
                                if (fdirection == -1) {
                                        fprintf(stderr, "Error: cannot open output file\n");
                                        exit(1);
                                }
                                dup2(fdirection, STDOUT_FILENO);
                                close(fdirection);
                                close(second_fd[1]);
                                dup2(second_fd[0], STDIN_FILENO);
                                close(second_fd[0]);
                                execvp(cmdPipe->pipe3->cmd, cmdPipe->pipe3->args);
                                perror("execvp");
                                exit(1);
                        } else { // No redirection
                                close(second_fd[1]);
                                dup2(second_fd[0], STDIN_FILENO);
                                close(second_fd[0]);
                                execvp(cmdPipe->pipe3->cmd, cmdPipe->pipe3->args);
                                perror("execvp");
                                exit(1);
                        }
                }else if (pipe3Pid != 0){
                        close(second_fd[0]);
                        close(second_fd[1]);
                        waitpid(pipe3Pid, &status[2],0);
                        exitStatus[2] = WEXITSTATUS(status[2]);
                }
                
        } else if(totalPipeSiganl == 3){
                pipe(first_fd);
                pipe(second_fd);
                pipe(third_fd);
                pipe1Pid = fork();
                if(pipe1Pid == 0){
                        //Child 1
                        // printf("processing left\n");
                        close(first_fd[0]);
                        dup2(first_fd[1], STDOUT_FILENO);
                        close(first_fd[1]);
                        // close(fd[1]);
                        // printf("left cmd :%s\n", cmdPipe->pipe1->cmd);
                        // write(fd[1], send, 7);
                        execvp(cmdPipe->pipe1->cmd, cmdPipe->pipe1->args);
                        perror("execvp");
                        exit(1);
                } else if(pipe1Pid != 0){
                        //parent
                        waitpid(pipe1Pid, &status[0],0);
                        exitStatus[0] = WEXITSTATUS(status[0]);
                }
                
                pipe2Pid = fork();
                if(pipe2Pid == 0){
                        close(first_fd[1]);
                        dup2(first_fd[0], STDIN_FILENO);
                        close(first_fd[0]);

                        close(second_fd[0]);
                        dup2(second_fd[1], STDOUT_FILENO);
                        close(second_fd[1]);
                        execvp(cmdPipe->pipe2->cmd, cmdPipe->pipe2->args);
                        perror("execvp");
                        exit(1);
                } else if (pipe2Pid != 0){
                        // waitpid(pipe1Pid, &status[0],0);
                        close(first_fd[0]);
                        close(first_fd[1]);
                        waitpid(pipe2Pid, &status[1],0);
                        // exitStatus[0] = WEXITSTATUS(status[0]);
                        exitStatus[1] = WEXITSTATUS(status[1]);
                }

                pipe3Pid = fork();
                if(pipe3Pid == 0){
                        close(second_fd[1]);
                        dup2(second_fd[0], STDIN_FILENO);
                        close(second_fd[0]);

                        close(third_fd[0]);
                        dup2(third_fd[1],STDOUT_FILENO);
                        close(third_fd[1]);
                        execvp(cmdPipe->pipe3->cmd, cmdPipe->pipe3->args);
                        perror("execvp");
                        exit(1);
                }else if (pipe3Pid != 0){
                        close(second_fd[0]);
                        close(second_fd[1]);
                        waitpid(pipe3Pid, &status[2],0);
                        exitStatus[2] = WEXITSTATUS(status[2]);
                }
                pipe4Pid = fork();
                if(pipe4Pid == 0){
                        if (redirectionFlag) { // If there is redirection
                                close(third_fd[1]);
                                dup2(third_fd[0], STDIN_FILENO);
                                close(third_fd[0]);

                                if (cmdPipe->pipe4->filename == NULL) {
                                        fprintf(stderr, "Error: no output file\n");
                                        exit(1);
                                }

                                int fdirection;
                                fdirection = open(cmdPipe->pipe4->filename, O_WRONLY | O_CREAT | O_TRUNC, 0644);
                                if (fdirection == -1) {
                                        fprintf(stderr, "Error: cannot open output file\n");
                                        exit(1);
                                }
                                dup2(fdirection, STDOUT_FILENO);
                                close(fdirection);
                                close(third_fd[1]);
                                dup2(third_fd[0], STDIN_FILENO);
                                close(third_fd[0]);
                                execvp(cmdPipe->pipe4->cmd, cmdPipe->pipe4->args);
                                perror("execvp");
                                exit(1);
                        } else { // No redirection
                                close(third_fd[1]);
                                dup2(third_fd[0], STDIN_FILENO);
                                close(third_fd[0]);
                                execvp(cmdPipe->pipe4->cmd, cmdPipe->pipe4->args);
                                perror("execvp");
                                exit(1);
                        }
                } else if(pipe4Pid != 0){
                        close(third_fd[0]);
                        close(third_fd[1]);
                        waitpid(pipe4Pid, &status[3],0);
                        exitStatus[3] = WEXITSTATUS(status[3]);
                }
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
        fprintf(stdout, "%s\n", workingDirectory);
        return 0;
}



void print_completation(char* cmdCopy, int returnVal){
        fprintf(stderr, "+ completed '%s' [%d]\n",
                        cmdCopy, returnVal);
}
void print_pipe_completation(char* cmdCopy, int* returnVal, int pipe_index){
        if(pipe_index == 1){
                fprintf(stderr, "+ completed '%s' [%d][%d]\n",
                        cmdCopy, returnVal[0], returnVal[1]);
        } else if(pipe_index == 2){
                fprintf(stderr, "+ completed '%s' [%d][%d][%d]\n",
                        cmdCopy, returnVal[0], returnVal[1], returnVal[2]);
        } else if(pipe_index == 3){
                fprintf(stderr, "+ completed '%s' [%d][%d][%d][%d]\n",
                        cmdCopy, returnVal[0], returnVal[1], returnVal[2], returnVal[3]);
        }
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
                pipe->pipe1->filename = malloc(32 * sizeof(char));


                pipe->pipe2 = malloc(sizeof(Command));
                pipe->pipe2->cmd = malloc(32 * sizeof(char));
                pipe->pipe2->args = malloc(16 * sizeof(char*));
                for (int i = 0; i < 16; i++) {
                    pipe->pipe2->args[i] = malloc(32 * sizeof(char));
                }
                pipe->pipe2->filename = malloc(32 * sizeof(char));

                pipe->pipe3 = malloc(sizeof(Command));
                pipe->pipe3->cmd = malloc(32 * sizeof(char));
                pipe->pipe3->args = malloc(16 * sizeof(char*));
                for (int i = 0; i < 16; i++) {
                    pipe->pipe3->args[i] = malloc(32 * sizeof(char));
                }
                pipe->pipe3->filename = malloc(32 * sizeof(char));

                pipe->pipe4 = malloc(sizeof(Command));
                pipe->pipe4->cmd = malloc(32 * sizeof(char));
                pipe->pipe4->args = malloc(16 * sizeof(char*));
                for (int i = 0; i < 16; i++) {
                    pipe->pipe4->args[i] = malloc(32 * sizeof(char));
                }
                pipe->pipe4->filename = malloc(32 * sizeof(char));
                
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

                // if(strcmp(pipe->pipe1->cmd, "NULLPIPE") == 0 || 
                //    strcmp(pipe->pipe2->cmd, "NULLPIPE") == 0 || 
                //    strcmp(pipe->pipe3->cmd, "NULLPIPE") == 0 ||
                //    strcmp(pipe->pipe4->cmd, "NULLPIPE") == 0 ){
                //         fprintf(stderr, "Error: missing command\n");
                //         continue;
                // }

                if(strcmp(pipe->pipe1->cmd, "NULLPIPE") == 0){
                        fprintf(stderr, "Error: missing command\n");
                        continue;
                }
                if(strcmp(pipe->pipe2->cmd, "NULLPIPE") == 0){
                        fprintf(stderr, "Error: missing command\n");
                        continue;
                }
                if(pipe_index == 2){
                        if(strcmp(pipe->pipe1->cmd, "NULLPIPE") == 0 || 
                           strcmp(pipe->pipe2->cmd, "NULLPIPE") == 0 || 
                           strcmp(pipe->pipe3->cmd, "NULLPIPE") == 0){
                                        fprintf(stderr, "Error: missing command\n");
                                        continue;
                           }
                }
                if(pipe_index == 3){
                        if(strcmp(pipe->pipe1->cmd, "NULLPIPE") == 0 || 
                           strcmp(pipe->pipe2->cmd, "NULLPIPE") == 0 || 
                           strcmp(pipe->pipe3->cmd, "NULLPIPE") == 0 || 
                           strcmp(pipe->pipe4->cmd, "NULLPIPE") == 0){
                                        fprintf(stderr, "Error: missing command\n");
                                        continue;
                           }
                }


                if(strcmp(com->cmd, "NULLCMD") == 0){
                        fprintf(stderr, "Error: missing command\n");
                        continue;
                }
                if(mislocatedOut == 1){
                        mislocatedOut = 0;
                        fprintf(stderr, "Error: mislocated output redirection\n");
                        continue;
                }

                


                if(!strcmp(com->cmd, "cd")){
                        // Change directory --- 'cd'
                        retval = builtin_cd(com);
                        print_completation(cmd, retval);
                }else if(!strcmp(com->cmd, "pwd")){
                        // Print working directory command --- 'pwd'
                        char working_dir[80];
                        getcwd(working_dir, sizeof(working_dir));
                        retval = builtin_pwd(working_dir);
                        print_completation(cmd,retval);

                }else if (!strcmp(com->cmd, "exit")) {
                        /* Builtin command */
                        // Exit command --- 'exit'
                        retval = builtin_exit();
                        print_completation(cmd, retval);
                        break;
                }else {
                        if(pipeFlag){
                                int* retarr;
                                retarr = sshellSystem_pipe(pipe, redirectionFlag);
                                print_pipe_completation(cmd, retarr, pipe_index);
                        } else {
                                retval = sshellSystem(com, redirectionFlag);
                                if (retval == 0) { // If there was no error
                                        print_completation(cmd,retval);
                                }
                        }
                        
                        // If retval returns 1, it means there was error
                        
                }

                // Free the memorry
                free(com);
                free(cmdCopy);
                free(pipe);
                
                pipe_index = 0;
                
        }

        return EXIT_SUCCESS;
}
