#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#define CMDLINE_MAX 512

char** tokenizer(char* cmdString) {
    // We can assume that the command and arguments will be separated by a whitespace.

    char* delimiter = " ";
    char** tokens = malloc(17 * sizeof(char*)); // Prompt: maximum # of arguments is 16. So with the command, its 17.
    for (int i = 0; i < 17; i++) {
        tokens[i] = malloc(32 * sizeof(char)); // Prompt: max length of individual tokens is 32
    }
    char* string;

    string = strtok(cmdString, delimiter);

    // Keep strtok'ing until we get a NULL value
    int index = 0;
    while (string != NULL) {
        tokens[index] = string;
        index++;
        string = strtok(NULL, delimiter);
    }

    return tokens;
}

// Implement the system() function by using fork+exec+wait method
int sshellSystem(char *cmdString){
    pid_t pid;
    //char *args[] = {"sh","-c",cmdString, NULL}; // Chris's
    char *args[] = {cmdString, "Hello", NULL}; // Mine
    int exitStatus;

    pid = fork();

    if(pid == 0){
        execvp(args[0], args);
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

                char** tokens = malloc(sizeof(char*));
                tokens = tokenizer(cmd);

                retval = sshellSystem(cmd);
                fprintf(stderr, "+ completed '%s': [%d]\n",
                        cmd, retval);
        }

        return EXIT_SUCCESS;
}
