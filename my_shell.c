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

int isRedirection(char* cmdString) {
    char* found;
    found = strchr(cmdString, '>');
    if (found == NULL) {
        return 0;
    }
    return 1;
}

/**
 * @param: args[x][y] -- variable to store the command after spliting
 * @author: Zhengyu Wu, Akash
 * This struct will store the arguments generated from user command
 * It will also store the filename incase there is redirection
 */
typedef struct{
    char* cmd;
    char** args;
    char* filename;
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

void split_command(char* cmdString, Command* com) {
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
    }
    else { // NO redirection
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

/**
 * @param: com --- struct that has command and arguments
 * @param: redirectionFlag --- if commandline has redirection, it is set to 1
 * @return: exitStatus --- check if this function exits corretlly
 * @author: Zhengyu Wu, Aksh
 * @version: 2020.10.13 last edited
 * This function uses fork+exec+wait method to implement the system()
 */
int sshellSystem(Command* com, int redirectionFlag, int appendFlag){
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
/**
 *      Built-in Command --- 'exit'
 *      Moved from main() (skeleton code given by professor)
 *      As assignment prompy mentioned, this command will never be called
 *      with incorrect argument.
 *      'no argument for exit'
 */

int builtin_exit(){
        fprintf(stderr, "Bye...\n");
        return 0;
}
/**
 *      Built-in Command --- 'cd'
 *      Change working directory to the directory user enter
 *      'exactly one argument for cd'
 */
int builtin_cd(Command* com){
        // Assume exactly one argument for cd
        chdir(com->args[1]);
        return 0;

}
/**
 *      Built-in Command --- 'pwd'
 *      Print out current working directory
 *      'no argument for pwd'
 */
int builtin_pwd(char* workingDirectory){
        fprintf(stderr, "%s\n", workingDirectory);
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

                split_command(cmdCopy, com);

                int redirectionFlag = 0;
                if (isRedirection(cmdCopy)) {
                    redirectionFlag = 1;
                }

                strcpy(cmdCopy, cmd); // Reset cmdCopy incase it was modified

                int appendFlag = 0;
                if (isAppend(cmdCopy)) {
                    appendFlag = 1;
                }

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

                }else if(!strcmp(com->cmd, "sls")) {
                        retval = sls_command();
                        print_completation(cmd, retval);
                }else{
                        retval = sshellSystem(com, redirectionFlag, appendFlag);
                        // If retval returns 1, it means there was error
                        if (retval == 0) { // If there was no error
                            print_completation(cmd,retval);
                        }
                }

                // Free the memorry
                free(com);
                free(cmdCopy);
        }

        return EXIT_SUCCESS;
}
