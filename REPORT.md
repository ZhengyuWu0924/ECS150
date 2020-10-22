# ECS 150 Project 1

Akash Arun Malode, 914706364 

Zhengyu Wu, 916951023

## Purpose
To create a shell (sshell) which implements builtin commands,

output redirection, pipeline commands, and extra features

(appending and displaying file sizes),

using forked processes and system calls.

## Design Choices
We decided to create two structures- one for the command and

its arguments, and another for piping. The command structure 

includes a command, an array of potential arguments, and a

filename slot (in case redirection is needed). The pipe 

structure uses the command structure to ensure each pipe segment

has its own command and arguments.

## Execution Procedure

+ Except for commands ```cd```, ```pwd```, ```exit```, and

  ```sls```, all other commands first require a fork to make
  
  a child, which then performs the necessary command executions.

+ Check if there is special requirements in user command

  We have three functions: ``` isRedirection() ```, ```isPipe()```,  

  and ```isAppend()``` to detect if the original command entered by 

  user include any output redirection signal, pipe signal, or append 

  signals. 
  
  + Output redirection 

    If there is output redirection signal in a command, the program 

    will make a special split of commands later. This is done by 
    
    first splitting the original command by the redirection symbol
    
    ```>```, and then further splitting the left side into command
    
    and arguments. The right side is put into the filename slot of 
    
    the command structure.

  + Pipe 

    If pipe signal has been detected, the program will enter a 

    special command split later.

  + Append 

    Appending to a file exists only if there first is a redirection
    
    signal. First check if redirection exists in the original command.
    
    If the next character is also a ```>```, then we open the file in 
    
    append mode, and not in truncate mode.

+ Check error in command 

  According to the project requirement, there are three types of 

  errors, as the shell will terminate itself when a library 

  function failed,

  there are only two types of errors we have to deal with.
  + Parsing error 

    If a incorrect command entered by user, the shell will return

    an error message like ```"Error: missing command```, and 

    ```"Error: no output file```. 

    Then run nothing but waiting next input. 

    *The shell will not die.*

  + Launching error 

    This error occurs when a folder cannot be accessed/opened,
    
    or if the folder does not exist. If we request a non-existent
    
    folder to be opened (as per ```opendir()```), then we display
    
    the error ```"Error: cannot cd into directory"```.



+ Redirect Output
  + We first detect if the output redirection symbol ```>```
    
    exists in the original command. We then check if a double
    
    ```>>``` exists.  
      
    + If appending is required, we open a file
    
      with ```O_APPEND``` option.
    
    + If its plain redirection
    
      (we want to truncate the file), we use the ```O_TRUNC```
    
      option.



+ Execute commands with pipe 
  + Detect amount of pipes 

    We detect the amount of pipes when splitting the user command 

    via the function ```split_pipe()```. The pipe amount will affect 

    how we connect pipes and how we execute commands.  

    ***According to requirement, there is up to three pipes.***

  + Connect and execute commands around pipe 

    Before execute commands, the shell will connect those commands

    based on the amount of pipes. 

    + Single pipe 

      If there is only one pipe, the command on the left will 

      run first, then the right.

    + Multiple pipes 

      If there are multiple pipes, the shell will run the commands

      one by one, from left to right.

  + Transit data between pipes 

    As a pipe receives the output from the command before the pipe

    sign and make is as the input to the command after pipe sign. 

    

    The shell will ```fork()``` a single child process to execute 

    one command at a time, from left to right. The command takes 

    previous output from pipe as its input unless it is the first 

    one, then store its output to next pipe unless it is the last

    one. 

    

    After executing current command, return from the child process 

    back to parent process, then repeat the above operation until 

    the last command done. 

    

    In the parent process, we record the commands exit values from

    child processes, and use it later on printing out the command

    completion message.
   
+ Built-in commands
  + The remaining built-in commands such as ```cat``` and ```grep```
  
    are executed using ```execvp()```. We use ```execvp()``` to 
    
    automatically search for programs in ```$PATH```.



## Test

Extensive manual testing has been done for each part.

We also used ```tester.sh``` provided by professor Porquet to test 

our program on CSIF environment. 

We've passed all the test cases.

Some errors such as printing the ```pwd``` result to ```stderr```

instead of ```stdout``` were detected after testing with

```tester.sh```.

## Challenges




## Source

[GNU C Library](https://www.gnu.org/software/libc/manual/html_mono/libc.html)

[Markdown Cheatsheet](https://github.com/adam-p/markdown-here/wiki/Markdown-Cheatsheet)
