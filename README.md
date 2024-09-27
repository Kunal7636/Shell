Custom Shell
This project is a custom shell written in C that supports various features like command execution, piping, conditional execution, tilde expansion, background processing, and more.

Features
Basic Command Execution: Execute common shell commands like ls, cat, etc.
Pipelining: Allows you to use multiple commands connected via pipes (|), where the output of one command is the input of the next.
Conditional Execution: Supports conditional operators like && and ||, allowing commands to run based on the success or failure of previous commands.
Tilde Expansion: Expands the ~ symbol to the home directory of the current user.
Special Characters: Handles special characters such as #, >, <, |, &&, and ||.
Background Processing: Allows commands to be executed in the background using &.
Wildcard Expansion: Supports wildcard expansion with * and ? using the glob library.
Redirection: Supports output redirection using > and >>.
Foreground/Background Execution: Executes commands in both foreground and background modes based on the input.

How to Use
Clone the repository and navigate to the project directory:

bash
Copy code
git clone <repository-url>
cd shell
Compile the program using the following command:

bash
Copy code
gcc -o shell24 shell.c
Run the shell:

bash
Copy code
./shell24
Type commands as you would in a normal shell. Some features to try:

Execute a command:
bash
Copy code
shell24$ ls
Use pipelines:
bash
Copy code
shell24$ ls | grep "file"
Use conditional execution:
bash
Copy code
shell24$ ls && echo "Listed files" || echo "Failed to list"
Run commands in the background:
bash
Copy code
shell24$ sleep 10 &
Expand wildcards:
bash
Copy code
shell24$ ls *.txt
Key Functions
parse_command
Parses a command string and extracts the arguments.

execute_command_pipeline
Executes commands in a pipeline. Handles the creation of pipes, redirection of input/output, and executes each command in a child process.

parse_input
Parses the input string, identifies arguments and special characters, and separates them for further processing.

expand_tilde
Expands the tilde (~) symbol to the user's home directory.

handle_special_characters
Handles special characters like #, >, <, &&, ||, and more for proper command execution.

background_process
Runs the command as a background process and prints the process ID.

expand_wildcards
Expands wildcards (*, ?) using the glob function.

Limitations
Maximum number of arguments for a command is set to 20 (MAX_ARGS).
Maximum number of commands in a pipeline is 10.
The shell does not support command history or job control like in bash.
Future Improvements
Add support for job control (fg, bg, jobs).
Implement command history and auto-completion.
Improve error handling for invalid commands or syntax.
