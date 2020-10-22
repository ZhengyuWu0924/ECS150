#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <dirent.h>
#include <sys/stat.h>

#define CMDLINE_MAX 512

//Global Variable
int pipe_index = 0;
int mislocatedOut = 0; // indicate if there is output mislocate in pipe

/**
 * CMD structure
 * Store command entered by user
 */
typedef struct{
    char* cmd;
    char** args;
    char* filename;
}Command;

/**
 * Pipe command structure
 * Store commands when user enters pipe signals
 */
typedef struct{
        Command *pipeCommand1;
        Command *pipeCommand2;
        Command *pipeCommand3;
        Command *pipeCommand4;
}Pipecmd;

/**
 * Detect if a user command contains output
 * redirection signals
 * @return 1, if there is output redirection requirement
 */
int isRedirection(char* cmdString) {
        if (strchr(cmdString, '>') == NULL) {
                return 0;
        }
        return 1;
}

int isAppend(char* cmdString) {
    char* found;
    found = strchr(cmdString, '>');

    if (found == NULL) {
        return 0;
    } else {
        if (found[1] == '>') {
            return 1;
        }
    }
    return 0;
}
/**
 * Detect if a user command contains pipe signals
 * @return 1, if there is pipe signal
 */
int isPipe(char* cmdString){
        if(strchr(cmdString, '|') == NULL){
                return 0;
        }
        return 1;
}
/**
 * Split general command entered by user
 * Store the splitted command in the data structure for further use
 */
int split_command(char* cmdString, Command* com){
        char* commandline = malloc(512 * sizeof(char));
        strcpy(commandline, cmdString);

        char* spaceDelimiter = " "; // Split by space
        char* string;

        int index = 0;
        string = strtok(cmdString, spaceDelimiter);
        if (string != NULL) {
                strcpy(com->cmd, string);
        }

        while (string != NULL) {
                if (index >= 16) { // Too many arguments
                    fprintf(stderr, "Error: too many process arguments\n");
                    return 1;
                }
                strcpy(com->args[index], string);
                string = strtok(NULL, spaceDelimiter);
                index++;
        }
        while (index < 16) {
                com->args[index] = NULL;
                index++;
        }
        free(commandline);
        return 0;
}

/**
 * Split command with output redirection requirement
 * Store the splitted command in the command data structure for further use
 */
int split_command_redirection(char* cmdString, Command* com) {

        char* commandline = malloc(512 * sizeof(char));
        strcpy(commandline, cmdString);
        char* checkRedirection = malloc(512 * sizeof(char));
        strcpy(checkRedirection, cmdString);


        char* redirectionDelimiter = ">";
        char* spaceDelimiter = " ";
        char* string;
        char* beforeRedirection;
        char* afterRedirection;
        char* missingCommandCheck;

        if (isRedirection(checkRedirection)) { // Redirection is present

                // If the first thing on the command line is '>', then there's a command missing
                missingCommandCheck = strtok(commandline, spaceDelimiter);
                char check = missingCommandCheck[0];

                if (check ==  '>') { // If the first thing is '>', return error
                    fprintf(stderr, "Error: missing command\n");
                    return 1;
                }

                strcpy(commandline, cmdString); // To reset commandline string incase it was modified previously

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
        return 0;
}
/**
 * Split command with pipe signals.
 * Store the splitted commands to pipe command data structure for
 * furether use
 */
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
                // If the first thing in the command is '|'
                // return to main then print error message
                // "Error: missing command"
                strcpy(pipe->pipeCommand1->cmd, "NULLPIPE");
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
                                // First command is empty space
                                // Missing command
                                strcpy(pipe->pipeCommand1->cmd, "NULLPIPE");
                        }
                        string = strtok(NULL, pipeDelimiter);
                        if(string == NULL){
                                // Second command is empty
                                // Missing command
                                strcpy(pipe->pipeCommand2->cmd, "NULLPIPE");
                        } else {
                                strcpy(pipe2Str, string);
                                if(strcmp(pipe2Str, " ") == 0){
                                        // Second command is empty space
                                        // Missing command 
                                        strcpy(pipe->pipeCommand2->cmd, "NULLPIPE");
                                }
                                string = strtok(NULL, pipeDelimiter);
                        }
                }
        }

        while(string != NULL){
                if(pipe_index == 1){
                        if(string == NULL){
                                // Third command is empty
                                // Missing command
                                strcpy(pipe->pipeCommand3->cmd, "NULLPIPE");
                                break;
                        }
                        strcpy(pipe3Str, string);
                        pipe_index++;
                        if(isRedirection(pipe1Str) == 1 || isRedirection(pipe2Str) == 1){
                                mislocatedOut = 1;
                                break;
                        }
                        if(strcmp(pipe3Str, " ") == 0){
                                // Third command is empty space
                                // Missing command
                                strcpy(pipe->pipeCommand3->cmd, "NULLPIPE");
                                break;
                        } else {
                                has3rdStr = 1;
                                string =strtok(NULL, pipeDelimiter);
                                if(string == NULL){
                                        // Fouth command is empty
                                        // Missing command
                                        strcpy(pipe->pipeCommand4->cmd, "NULLPIPE");
                                        break;
                                }
                        }
                }
                if(pipe_index == 2){
                        pipe_index++;
                        if(isRedirection(pipe1Str) == 1 || isRedirection(pipe2Str) == 1 
                        || isRedirection(pipe3Str) == 1){
                                mislocatedOut = 1;
                                break;
                        }
                        strcpy(pipe4Str, string);
                        if(strcmp(pipe4Str, " ") == 0){
                                // Fouth command is empty space
                                // Missing command
                                strcpy(pipe->pipeCommand4->cmd, "NULLPIPE");
                                break;
                        } else {
                                has4thStr = 1;
                                break;
                        }
                }
        }
        split_command(pipe1Str, pipe->pipeCommand1);

        if(has3rdStr == 0){
                // Second command is the last, output redirection permitted
                split_command_redirection(pipe2Str, pipe->pipeCommand2);

        }
        if(has4thStr == 0 && has3rdStr == 1){
                split_command(pipe2Str, pipe->pipeCommand2);
                // Third command is the last, output redirection permitted
                split_command_redirection(pipe3Str, pipe->pipeCommand3);
        }

        if(has4thStr == 1){
                split_command(pipe2Str, pipe->pipeCommand2);
                split_command(pipe3Str, pipe->pipeCommand3);
                // Fouth command is the last, output redirection permitted
                split_command_redirection(pipe4Str, pipe->pipeCommand4);

        }

        free(commandline);
        free(pipe1Str);
        free(pipe2Str);
        free(pipe3Str);
        free(pipe4Str);
}

/**
 * Shell to run the general commands.
 * Implemented with fork() + exec() + wait() method
 */
int sshellSystem(Command* com, int redirectionFlag, int appendFlag){
        pid_t pid;
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
                    if (appendFlag) { // We have a >> in commandline. Append NOT truncate. Open file in append mode. Else, open in truncate mode.
                        fd = open(com->filename, O_WRONLY | O_CREAT | O_APPEND, 0644);
                    } else {
                        fd = open(com->filename, O_WRONLY | O_CREAT | O_TRUNC, 0644);
                    }
                    if (fd == -1) {
                        fprintf(stderr, "Error: cannot open output file\n");
                        exit(1);
                    }
                    dup2(fd, STDOUT_FILENO);
                    close(fd);
                    execvp(com->cmd, com->args);
                    fprintf(stderr, "Error: command not found\n");
                    exit(1);
                } else { // No redirection
                    execvp(com->cmd, com->args);
                    fprintf(stderr, "Error: command not found\n");
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
/**
 * Shell to run command with pipe signals
 * Implemented with fork() + exec() + wait() method
 */
int* sshellSystem_pipe(Pipecmd* cmdPipe, int redirectionFlag){
        // We assumen there is up to 3 pipes
        int first_fd[2];
        int second_fd[2];
        int third_fd[2];

        // 4 commands in 3 pipes
        pid_t pipeCommand1Pid;
        pid_t pipeCommand2Pid;
        pid_t pipeCommand3Pid;
        pid_t pipeCommand4Pid;

        static int exitStatus[4]; // return exit values
        int totalPipeSiganl = pipe_index;
        int status[4];

        if(totalPipeSiganl == 1){ // Exactly 1 pipe
                pipe(first_fd); // Create a pipe 
                pipeCommand1Pid = fork(); // Execute the first command in pipe
                if(pipeCommand1Pid == 0){
                        // Child
                        close(first_fd[0]); // Close the pipe input
                        dup2(first_fd[1], STDOUT_FILENO); // Output to pipe
                        close(first_fd[1]); // Close the pipe output
                        execvp(cmdPipe->pipeCommand1->cmd, cmdPipe->pipeCommand1->args);
                        perror("execvp");
                        exit(1);

                } else if(pipeCommand1Pid != 0){
                        // Parent
                        waitpid(pipeCommand1Pid, &status[0],0);
                        exitStatus[0] = WEXITSTATUS(status[0]);
                }
                pipeCommand2Pid = fork(); //Execute the second command in pipe
                if(pipeCommand2Pid == 0){
                        // Child
                        if (redirectionFlag) { // If there is redirection
                                close(first_fd[1]); // Close the pipe output
                                dup2(first_fd[0], STDIN_FILENO); // Input from pipe
                                close(first_fd[0]); // Close the pipe input
                                if (cmdPipe->pipeCommand2->filename == NULL) {
                                        fprintf(stderr, "Error: no output file\n");
                                        exit(1);
                                }
                                int fdirection;
                                fdirection = open(cmdPipe->pipeCommand2->filename, O_WRONLY | O_CREAT | O_TRUNC, 0644);
                                if (fdirection == -1) {
                                        fprintf(stderr, "Error: cannot open output file\n");
                                        exit(1);
                                }
                                dup2(fdirection, STDOUT_FILENO); // Output to file
                                close(fdirection); // Close file
                                close(first_fd[1]); // Close pipe output
                                dup2(first_fd[0], STDIN_FILENO); // Input from pipe
                                close(first_fd[0]); // Close pipe input
                                execvp(cmdPipe->pipeCommand2->cmd, cmdPipe->pipeCommand2->args);
                                perror("execvp");
                                exit(1);
                        } else { // No redirection
                                close(first_fd[1]); // Close the pipe output
                                dup2(first_fd[0], STDIN_FILENO); // Input from pipe
                                close(first_fd[0]); // Close the pipe input
                                execvp(cmdPipe->pipeCommand2->cmd, cmdPipe->pipeCommand2->args);
                                perror("execvp");
                                exit(1);
                        }
                } else if (pipeCommand2Pid != 0){
                        // Parent
                        // Totally close pipe
                        close(first_fd[0]);
                        close(first_fd[1]);
                        waitpid(pipeCommand2Pid, &status[1],0);
                        exitStatus[1] = WEXITSTATUS(status[1]);
                }

        } else if(totalPipeSiganl == 2){ // 2 Pipes
                pipe(first_fd); // First pipe
                pipe(second_fd); // Second pipe
                pipeCommand1Pid = fork();
                if(pipeCommand1Pid == 0){ // Execute the first command in pipe
                        // Child
                        close(first_fd[0]); // Close pipe1 input
                        dup2(first_fd[1], STDOUT_FILENO); // Output to pipe1
                        close(first_fd[1]); // Close pipe1 output
                        execvp(cmdPipe->pipeCommand1->cmd, cmdPipe->pipeCommand1->args);
                        perror("execvp");
                        exit(1);
                } else if(pipeCommand1Pid != 0){
                        // Parent
                        waitpid(pipeCommand1Pid, &status[0],0);
                        exitStatus[0] = WEXITSTATUS(status[0]);
                }
                pipeCommand2Pid = fork();
                if(pipeCommand2Pid == 0){ // Execute the second command in pipe
                        close(first_fd[1]); // Close pipe1 output
                        dup2(first_fd[0], STDIN_FILENO); // Input from pipe1
                        close(first_fd[0]); // Close pipe1 input
                        // Pipe 1 ends
                        // Pipe 2 start
                        close(second_fd[0]); // Close pipe2 input
                        dup2(second_fd[1], STDOUT_FILENO); // Output to pipe2
                        close(second_fd[1]);// Close pipe2 output
                        execvp(cmdPipe->pipeCommand2->cmd, cmdPipe->pipeCommand2->args);
                        perror("execvp");
                        exit(1);
                } else if (pipeCommand2Pid != 0){
                        // Parent
                        // Totally close pipe1
                        close(first_fd[0]);
                        close(first_fd[1]);
                        waitpid(pipeCommand2Pid, &status[1],0);
                        exitStatus[1] = WEXITSTATUS(status[1]);
                }
                pipeCommand3Pid = fork();
                if(pipeCommand3Pid == 0){ // Execute the third command in pipe
                        if (redirectionFlag) { // If there is redirection
                                close(second_fd[1]); // Close pipe2
                                dup2(second_fd[0], STDIN_FILENO); // Input from pipe2
                                close(second_fd[0]); // Close pipe2 input
                                if (cmdPipe->pipeCommand3->filename == NULL) {
                                        fprintf(stderr, "Error: no output file\n");
                                        exit(1);
                                }
                                int fdirection;
                                fdirection = open(cmdPipe->pipeCommand3->filename, O_WRONLY | O_CREAT | O_TRUNC, 0644);
                                if (fdirection == -1) {
                                        fprintf(stderr, "Error: cannot open output file\n");
                                        exit(1);
                                }
                                dup2(fdirection, STDOUT_FILENO); // Output to file
                                close(fdirection); // Close file
                                close(second_fd[1]); // Clode pipe2 output
                                dup2(second_fd[0], STDIN_FILENO); // Input from pipe2
                                close(second_fd[0]); // Close pipe2 input
                                execvp(cmdPipe->pipeCommand3->cmd, cmdPipe->pipeCommand3->args);
                                perror("execvp");
                                exit(1);
                        } else { // No redirection
                                close(second_fd[1]); // Close pipe2 output
                                dup2(second_fd[0], STDIN_FILENO); // Input from pipe2
                                close(second_fd[0]); // Close pipe2 input
                                execvp(cmdPipe->pipeCommand3->cmd, cmdPipe->pipeCommand3->args);
                                perror("execvp");
                                exit(1);
                        }
                }else if (pipeCommand3Pid != 0){
                        // Parent
                        // Totally close pipe2
                        close(second_fd[0]);
                        close(second_fd[1]);
                        waitpid(pipeCommand3Pid, &status[2],0);
                        exitStatus[2] = WEXITSTATUS(status[2]);
                }

        } else if(totalPipeSiganl == 3){ // 3 pipes
                pipe(first_fd); 
                pipe(second_fd);
                pipe(third_fd); // Third pipe
                pipeCommand1Pid = fork();
                if(pipeCommand1Pid == 0){ // Execute the first command in pipe
                        // Child
                        close(first_fd[0]); // Close pipe1 input
                        dup2(first_fd[1], STDOUT_FILENO); // Output to pipe1
                        close(first_fd[1]); // Close pipe1 output
                        execvp(cmdPipe->pipeCommand1->cmd, cmdPipe->pipeCommand1->args);
                        perror("execvp");
                        exit(1);
                } else if(pipeCommand1Pid != 0){
                        //Parent
                        waitpid(pipeCommand1Pid, &status[0],0);
                        exitStatus[0] = WEXITSTATUS(status[0]);
                }

                pipeCommand2Pid = fork();
                if(pipeCommand2Pid == 0){ // Execute the second command in pipe
                        close(first_fd[1]); // Close pipe1 output
                        dup2(first_fd[0], STDIN_FILENO); // Input from pipe1
                        close(first_fd[0]); // Close pipe1 input
                        // Pipe 1 ends
                        // Pipe 2 starts
                        close(second_fd[0]); // Close pipe2 input
                        dup2(second_fd[1], STDOUT_FILENO); // Output to pipe2
                        close(second_fd[1]); // Close pipe2 output
                        execvp(cmdPipe->pipeCommand2->cmd, cmdPipe->pipeCommand2->args);
                        perror("execvp");
                        exit(1);
                } else if (pipeCommand2Pid != 0){
                        // Parent
                        // Totally close pipe1
                        close(first_fd[0]);
                        close(first_fd[1]);
                        waitpid(pipeCommand2Pid, &status[1],0);
                        exitStatus[1] = WEXITSTATUS(status[1]);
                }

                pipeCommand3Pid = fork();
                if(pipeCommand3Pid == 0){ // Execute the third command in pipe
                        close(second_fd[1]); // Close pipe2 output
                        dup2(second_fd[0], STDIN_FILENO); // Input from pipe2
                        close(second_fd[0]); // Close pipe2 input
                        // Pipe2 ends
                        // Pipe2 starts
                        close(third_fd[0]); // Close pipe3 input
                        dup2(third_fd[1],STDOUT_FILENO); // Output to pipe3
                        close(third_fd[1]); // Close pipe3 output
                        execvp(cmdPipe->pipeCommand3->cmd, cmdPipe->pipeCommand3->args);
                        perror("execvp");
                        exit(1);
                }else if (pipeCommand3Pid != 0){
                        // Parent
                        // Totally close pipe2
                        close(second_fd[0]);
                        close(second_fd[1]);
                        waitpid(pipeCommand3Pid, &status[2],0);
                        exitStatus[2] = WEXITSTATUS(status[2]);
                }
                pipeCommand4Pid = fork();
                if(pipeCommand4Pid == 0){ // Execute the fouth command in pipe
                        if (redirectionFlag) { // If there is redirection
                                close(third_fd[1]); // Close pipe3 output
                                dup2(third_fd[0], STDIN_FILENO); // Input from pipe3
                                close(third_fd[0]); // Close pipe3 input

                                if (cmdPipe->pipeCommand4->filename == NULL) {
                                        fprintf(stderr, "Error: no output file\n");
                                        exit(1);
                                }

                                int fdirection;
                                fdirection = open(cmdPipe->pipeCommand4->filename, O_WRONLY | O_CREAT | O_TRUNC, 0644);
                                if (fdirection == -1) {
                                        fprintf(stderr, "Error: cannot open output file\n");
                                        exit(1);
                                }
                                dup2(fdirection, STDOUT_FILENO); // Output to file
                                close(fdirection); // Close file
                                close(third_fd[1]); // Close pipe3 output
                                dup2(third_fd[0], STDIN_FILENO); // Input from pipe3
                                close(third_fd[0]); // Close pipe3 input
                                execvp(cmdPipe->pipeCommand4->cmd, cmdPipe->pipeCommand4->args);
                                perror("execvp");
                                exit(1);
                        } else { // No redirection
                                close(third_fd[1]); // Close pipe3 output
                                dup2(third_fd[0], STDIN_FILENO); // Input from pipe3
                                close(third_fd[0]); // Close pipe3 input
                                execvp(cmdPipe->pipeCommand4->cmd, cmdPipe->pipeCommand4->args);
                                perror("execvp");
                                exit(1);
                        }
                } else if(pipeCommand4Pid != 0){
                        // Parent
                        // Totally close pipe3
                        close(third_fd[0]);
                        close(third_fd[1]);
                        waitpid(pipeCommand4Pid, &status[3],0);
                        exitStatus[3] = WEXITSTATUS(status[3]);
                }
        }

        return exitStatus;

}
/**
 * Builtin command --- exit
 */
int builtin_exit(){
        fprintf(stderr, "Bye...\n");
        return 0;
}

/**
 * Builtin command --- cd
 */
int builtin_cd(Command* com){
        // Assume exactly one argument for cd
        DIR* dirp;
        dirp = opendir(com->args[1]);
        if (dirp == NULL) {
            fprintf(stderr, "Error: cannot cd into directory\n");
            return 1;
        }
        chdir(com->args[1]);
        return 0;

}

/**
 * Builtin command --- pwd
 */
int builtin_pwd(char* workingDirectory){
        fprintf(stdout, "%s\n", workingDirectory);
        return 0;
}

int sls_command() {
    DIR* dirp;
    struct stat sb;
    struct dirent* dp;

    dirp = opendir(".");

    if (dirp == NULL) {
        printf("Error: cannot open directory\n");
        return 1;
    }

    while ((dp = readdir(dirp)) != NULL) {
        if (!strcmp(dp->d_name, ".") | !strcmp(dp->d_name, "..")) { // Ignore file names with . and ..
            continue;
        }
        stat(dp->d_name, &sb);
        printf("%s (", dp->d_name);
        printf("%llu bytes)\n", (long long)sb.st_size);
    }
    return 0;
}

/**
 * Print completation message after running user command
 * Display the exit value for the command
 */
void print_completation(char* cmdCopy, int returnVal){
        fprintf(stderr, "+ completed '%s' [%d]\n",
                        cmdCopy, returnVal);
}

/**
 * Print completation message after running pipe
 * Display all the exit value  of each command composing the pipe separately
 */
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

                /* Command structure initializing */
                Command* com = NULL;
                com = malloc(sizeof(Command));
                com->cmd = malloc(32 * sizeof(char));
                com->args = malloc(16 * sizeof(char*));
                for (int i = 0; i < 16; i++) {
                    com->args[i] = malloc(32 * sizeof(char));
                }
                com->filename = malloc(32 * sizeof(char));

                /* Pipe structure initializing */
                Pipecmd* pipe = NULL;
                pipe = malloc(sizeof(Pipecmd));
                pipe->pipeCommand1 = malloc(sizeof(Command));
                pipe->pipeCommand1->cmd = malloc(32 * sizeof(char));
                pipe->pipeCommand1->args = malloc(16 * sizeof(char*));
                for (int i = 0; i < 16; i++) {
                    pipe->pipeCommand1->args[i] = malloc(32 * sizeof(char));
                }
                pipe->pipeCommand1->filename = malloc(32 * sizeof(char));


                pipe->pipeCommand2 = malloc(sizeof(Command));
                pipe->pipeCommand2->cmd = malloc(32 * sizeof(char));
                pipe->pipeCommand2->args = malloc(16 * sizeof(char*));
                for (int i = 0; i < 16; i++) {
                    pipe->pipeCommand2->args[i] = malloc(32 * sizeof(char));
                }
                pipe->pipeCommand2->filename = malloc(32 * sizeof(char));

                pipe->pipeCommand3 = malloc(sizeof(Command));
                pipe->pipeCommand3->cmd = malloc(32 * sizeof(char));
                pipe->pipeCommand3->args = malloc(16 * sizeof(char*));
                for (int i = 0; i < 16; i++) {
                    pipe->pipeCommand3->args[i] = malloc(32 * sizeof(char));
                }
                pipe->pipeCommand3->filename = malloc(32 * sizeof(char));

                pipe->pipeCommand4 = malloc(sizeof(Command));
                pipe->pipeCommand4->cmd = malloc(32 * sizeof(char));
                pipe->pipeCommand4->args = malloc(16 * sizeof(char*));
                for (int i = 0; i < 16; i++) {
                    pipe->pipeCommand4->args[i] = malloc(32 * sizeof(char));
                }
                pipe->pipeCommand4->filename = malloc(32 * sizeof(char));


                /* Redirection signal detection */
                int redirectionFlag = 0;
                if(isRedirection(cmdCopy)){
                        redirectionFlag = 1;
                }

                strcpy(cmdCopy, cmd); // Reset cmdCopy incase it was modified

                int appendFlag = 0;
                if (isAppend(cmdCopy)) {
                    appendFlag = 1;
                }

                strcpy(cmdCopy, cmd); // Reset cmdCopy incase it was modified

                /* Pipe signal detection */
                int pipeFlag = 0;
                if(isPipe(cmdCopy)){
                        pipeFlag = 1;
                }

                strcpy(cmdCopy, cmd); // Reset cmdCopy incase it was modified

                if(pipeFlag == 1){ //If there is pipe signal
                        split_pipe(cmdCopy, pipe);
                } else if(redirectionFlag == 1 && pipeFlag == 0){
                        int success = split_command_redirection(cmdCopy, com);
                        if (success == 1) {
                            continue;
                        }
                } else if(pipeFlag == 0 && redirectionFlag == 0){
                        int success = split_command(cmdCopy, com); // If success is 0, its fine. If its 1, its BAD.
                        if (success == 1) {
                            continue;
                        }
                }

                /* Error management */
                
                // Single pipe signal
                if(strcmp(pipe->pipeCommand1->cmd, "NULLPIPE") == 0){ //Left comment is missing
                        fprintf(stderr, "Error: missing command\n");
                        continue;
                }
                if(strcmp(pipe->pipeCommand2->cmd, "NULLPIPE") == 0){ //Right comment is missing
                        fprintf(stderr, "Error: missing command\n");
                        continue;
                }
                // Multiple pipe signals
                if(pipe_index == 2){ // 2 pipe signals
                        if(strcmp(pipe->pipeCommand1->cmd, "NULLPIPE") == 0 ||
                           strcmp(pipe->pipeCommand2->cmd, "NULLPIPE") == 0 ||
                           strcmp(pipe->pipeCommand3->cmd, "NULLPIPE") == 0){
                                        fprintf(stderr, "Error: missing command\n");
                                        continue;
                           }
                }
                if(pipe_index == 3){ // 3 pipe signals
                        if(strcmp(pipe->pipeCommand1->cmd, "NULLPIPE") == 0 ||
                           strcmp(pipe->pipeCommand2->cmd, "NULLPIPE") == 0 ||
                           strcmp(pipe->pipeCommand3->cmd, "NULLPIPE") == 0 ||
                           strcmp(pipe->pipeCommand4->cmd, "NULLPIPE") == 0){
                                        fprintf(stderr, "Error: missing command\n");
                                        continue;
                           }
                }
                // Output redirection before the last comment in pipe
                if(mislocatedOut == 1){
                        mislocatedOut = 0;
                        fprintf(stderr, "Error: mislocated output redirection\n");
                        continue;
                }
                // Empty command
                if(strcmp(com->cmd, "NULLCMD") == 0){
                        fprintf(stderr, "Error: missing command\n");
                        continue;
                }
                

                /* Command Execution */
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
                        // Exit command --- 'exit'
                        retval = builtin_exit();
                        print_completation(cmd, retval);
                        break;
                }else if(!strcmp(com->cmd, "sls")) {
                        retval = sls_command();
                        print_completation(cmd, retval);
                }else {
                        if(pipeFlag){ // run command include pipe
                                int* retarr; // an array to store exit values
                                retarr = sshellSystem_pipe(pipe, redirectionFlag);
                                print_pipe_completation(cmd, retarr, pipe_index);
                        } else { // run command not include pipe
                                retval = sshellSystem(com, redirectionFlag, appendFlag);
                                if (retval == 0) { // If there was no error
                                        print_completation(cmd,retval);
                                }
                        }
                }

                // Free the memorry
                free(com);
                free(cmdCopy);
                free(pipe);
                // Reset the amount of pipe signal
                pipe_index = 0;

        }

        return EXIT_SUCCESS;
}
