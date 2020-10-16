#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#define CMDLINE_MAX 512

/**
 * @param: args[x][y] -- variable to store the command after spliting
 * @author: Zhengyu Wu
 * This struct will store the arguments generated from user command
 * Due to professor's describtion, assume that:
 * A program has a maximum of 16 arguments
 * The maximum length of individual tokens never exceeds 32 characters
 * 
 * Set x = 16, y = 32;
 */
typedef struct{
        char agrs[16][32]; 
        char* comPtr;
}command;

/**
 * @param: com --- command structur variable to store the user command splitted by stytok()
 * @param: cmdString --- original user command;
 * @return: cmdStr --- a char* variable that store the generated user commnad (new command)
 * @author: Zhengyu Wu
 * @version: 2020.10.15 20:45 last edited.
 * This function will be called in main(), then make an array to store all non-empty arguments in a 
 * user command line.
 * Example: user input: "echo hello | date" will be generated as
 * "echo", "hello", "|", "date".
 * 
 */
char* split_command(command* com, char* cmdString){
        int index = 0;
        // set memory location to struct pointer
        com = (command *)malloc(16 * sizeof(command));
        char* cmdStr = NULL;
        // set memory location to string (char *)
        cmdStr = (char *) malloc(16 * sizeof(command));
        // set memory location to store token
        char* token = (char *) malloc(16 * sizeof(char));
        // split user command by " "
        token = strtok(cmdString, " ");
        // let the struct variable to store all tokens
        // then store in a char* variable for further use
        while(token != NULL){
                strcpy(com->agrs[index], token);
                // printf("%d---%s\n",index,com->agrs[index]); // for debug using only
                token = strtok(NULL, " ");
                strcat(cmdStr, com->agrs[index]);
                strcat(cmdStr, " ");
                index++;
                if(token == NULL){
                        strcpy(com->agrs[index], "\0");
                        strcat(cmdStr, "\0");
                        break;
                }
        }
        free(token);
        return cmdStr;

}

/**
 * @param: cmdString --- user command
 * @return: exitStatus --- check if this function exits corretlly
 * @author: Zhengyu Wu
 * @version: 2020.10.13 last edited
 * This function usese fork+exec+wait method to implement the system()
 */
int sshellSystem(char* cmdString){
        pid_t pid;
        char *args[] = {"sh","-c",cmdString, NULL}; 
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

                command* com = NULL;
                char *newCmd = split_command(com, cmd);
                
                
                retval = sshellSystem(newCmd);
                
                print_completation(cmdCopy,retval);
                // Free the memorry
                free(com); 
                free(newCmd);
                free(cmdCopy);
        }

        return EXIT_SUCCESS;
}
