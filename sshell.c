#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#define CMDLINE_MAX 512

/**
 * @param: args[x][y] -- variable to store the command after spliting
 * @author: Zhengyu Wu, Akash
 * This struct will store the arguments generated from user command
 */
typedef struct{
    char* cmd;
    char** args;
}Command;

/**
 * @param: com --- command structur variable to store the user command splitted by stytok()
 * @param: cmdString --- original user command;
 * @return: cmdStr --- a char* variable that store the generated user commnad (new command)
 * @author: Zhengyu Wu, Akash
 * @version: 2020.10.15 20:45 last edited.
 * This function will be called in main(), then make an array to store all non-empty arguments in a 
 * user command line.
 * Example: user input: "echo hello | date" will be generated as
 * "echo", "hello", "|", "date".
 * 
 */
void split_command(char** tokens, Command* com) {
    int index = 0;

    // Copy command into cmd first
    if (tokens[index] != NULL) {
        strcpy(com->cmd, tokens[index]);
    }
    index++;

    while (tokens[index] != NULL){
        strcpy(com->args[index], tokens[index]);
        index++;
    }
    while (index < 16) {
        com->args[index] = NULL;
        index++;
    }
}


void tokenizer(char** tokens, char* cmdString) {
    // We can assume that the command and arguments will be separated by a whitespace.

    char* delimiter = " ";
    char* string;

    string = strtok(cmdString, delimiter);

    // Keep strtok'ing until we get a NULL value
    int index = 0;
    while (string != NULL) {
        strcpy(tokens[index], string);
        index++;
        string = strtok(NULL, delimiter);
    }
    while (index < 16) {
        tokens[index] = NULL;
        index++;
    }
}

/**
 * @param: cmdString --- user command
 * @return: exitStatus --- check if this function exits corretlly
 * @author: Zhengyu Wu, Aksh
 * @version: 2020.10.13 last edited
 * This function usese fork+exec+wait method to implement the system()
 */
int sshellSystem(Command* com){
        pid_t pid;
        // char *args[] = {"sh","-c",cmdString, NULL}; 
        int exitStatus;

        pid = fork();

        if(pid == 0){
                // Child
                execvp(com->cmd, com->args);
                perror("execvp");
                exit(1);
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
    @param: cmdCopy --- a copy of user cmd
    @param: returnVal --- value return from sshellsystem, the fork+exec+wait method
    @author: Zhengyu Wu
    @version: 2020.10.14 last edited
    This Function will take the cmd ented by user, then make a copy
    It will allows the program to print out the original cmd in each completation message.
*/
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
                if (nl)
                        *nl = '\0';

                /* Builtin command */
                if (!strcmp(cmd, "exit")) {
                        fprintf(stderr, "Bye...\n");
                        break;
                }
                /*
                        Make a copy of cmd entered by user
                */
                int rawLen = strlen(cmd);
                char* cmdCopy = (char *) malloc(rawLen);
                strcpy(cmdCopy, cmd);


                /* Regular command */
                // retval = system(cmd);

                char** tokens = malloc(16 * sizeof(char*)); // Prompt: maximum # of arguments is 16. So with the command, its 17.
                for (int i = 0; i < 17; i++) {
                    tokens[i] = malloc(32 * sizeof(char)); // Prompt: max length of individual tokens is 32
                }

                tokenizer(tokens, cmd);
                Command* com = NULL;
                com = malloc(sizeof(Command));
                com->cmd = malloc(32 * sizeof(char));
                com->args = malloc(16 * sizeof(char*));
                for (int i = 0; i < 16; i++) {
                    com->args[i] = malloc(32 * sizeof(char));
                }



                split_command(tokens, com);
                retval = sshellSystem(com);
                print_completation(cmdCopy,retval);
                
                
                // Free the memorry
                free(com); 
                free(tokens);
                free(cmdCopy);
        }

        return EXIT_SUCCESS;
}
