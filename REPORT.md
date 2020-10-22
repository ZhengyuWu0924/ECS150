# ECS 150 Project 1

Akash Arun Malode, 914706364 

Zhengyu Wu, 916951023

## Execution Procedure



+ Check if there is special requirements in user command

  We have three functions: ``` isRedirection() ```, ```isPipe()```,  

  and ```isAppend()``` to detect if the original command entered by 

  user include any output redirection signal, pipe signal, or append 

  signals. 
  + Output redirection 

    If there is output redirection signal in a command, the program 

    will make a special split of commands later.

  + Pipe 

    If pipe signal has been detected, the program will enter a 

    special command split later.

  + Append 

    (EDIT HERE)

+ Check error in command 

  According to project requirement, there are three types of 

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

    (EDIT HERE)



+ Redirect Output
  + （edit here）
    + （edit here）



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



## Test

Manually test has been done for each part. 

We also use ```tester.sh``` provided by professor on Canvas to test 

our program on CSIF environment. 

Fully point received.



## Source

[GNU C Library](https://www.gnu.org/software/libc/manual/html_mono/libc.html)

[Markdown Cheatsheet](https://github.com/adam-p/markdown-here/wiki/Markdown-Cheatsheet)
