#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#define CMDLINE_MAX 512

// Implement the system() function by using fork+exec+wait method
int sshellSystem(char *cmdString){
    pid_t pid;
    char *args[] = {"sh","-c",cmdString, NULL};
    int ret;
    pid = fork();
    if(pid == 0){
        execvp(args[0], args);
        perror("execvp");
        exit(1);
    }else if(pid > 0){
        int status;
        waitpid(pid, &status, 0);
        // printf("completed %d\n ", WEXITSTATUS(status));
    }else{
        perror("fork");
        exit(1);
    }
    return 0;
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
                retval = sshellSystem(cmd);
                fprintf(stdout, "+ completed '%s': [%d]\n",
                        cmd, retval);
        }

        return EXIT_SUCCESS;
}
