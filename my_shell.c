#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#define CMDLINE_MAX 512

typedef struct {
    char* cmd;
    char** args;
}Command;

void split_cmd_args(char** tokens, Command* com) {
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

// Implement the system() function by using fork+exec+wait method
int sshellSystem(Command* com){
    pid_t pid;
    //char *args[] = {"sh","-c",cmdString, NULL}; // Chris's
    //char *args[] = {cmdString, "Hello", NULL}; // Mine
    int exitStatus;

    pid = fork();

    if(pid == 0){
        execvp(com->cmd, com->args);
        perror("execvp");
        exit(1);
    }else if(pid > 0){
        int status;
        waitpid(pid, &status, 0);
        exitStatus = WEXITSTATUS(status);
    }else{
        perror("fork");
        exit(1);
    }
    return exitStatus;
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

                /* Regular command */
                // retval = system(cmd);

                // ****** Allocating space for tokens 2d array *******

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
                split_cmd_args(tokens, com);

                // At this point, com should have the command and arguments stored.

                retval = sshellSystem(com);
                fprintf(stderr, "+ completed '%s': [%d]\n",
                        cmd, retval);
        }

        return EXIT_SUCCESS;
}
