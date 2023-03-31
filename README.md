
The shell should operate in a basic way: when you type in a command (in
response to its prompt), the shell creates a child process that executes the
command you entered and then prompts for more user input when it has
finished. 
This simple linux shell built in C displays a prompt where the user can enter the
commands to execute.//
The shell can be run in two modes : interactive or batch. In interactive mode, the
user types a command at the prompt and the output will be instantly displayed
there.
In batch mode, your shell is started by specifying a batch file on its command
line; the batch file contains the list of commands that should be executed. In
batch mode, you should echo each line you read from the batch file back to the
user before executing it.
In each mode, your shell stops accepting new commands when it sees the quit
command on a line or reaches the end of the input stream (i.e., the end of the
batch file).
