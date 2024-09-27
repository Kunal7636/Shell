// this is assignemnt 3
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h> // Include stdlib.h for exit() function
#include <sys/wait.h> // Include sys/wait.h for wait() function
#include <pwd.h> // Include pwd.h for getting user's home directory
#include <glob.h>
#include <ctype.h> // Include ctype.h for isspace() function

#define MAX_ARGS 20
#define MAX_SPECIAL_CHARS 20
int flag = 0;
#define MAX_PIPELINE_COMMANDS 10 // Define the maximum number of pipeline commands
#define MAX_COMMAND_ARGUMENTS 20 
#define MAX_PATH_LENGTH 1024
#define MAXI_COMMAND_ARGUMENTS 50


void parse_command(char* commd, char* argmts[MAXI_COMMAND_ARGUMENTS])  // Function to parse the command and extract arguments from it
{
    char* parsedToken = strtok(commd, " "); // Tokenize the command
    int v = 0; // Initialize argument index
    while (parsedToken != NULL && v < MAXI_COMMAND_ARGUMENTS) 
    {
        argmts[v] = parsedToken; // Store parsed argument in array of arguments
        parsedToken = strtok(NULL, " "); // Move to next token
        v++; // Increment the index arguemnet
    }
    
    if (v == MAXI_COMMAND_ARGUMENTS) // checking if the maximum number of arguments is reached
    {
        printf("Provide less than %d arguments\n", MAXI_COMMAND_ARGUMENTS - 1); // Display error message if provided argumennt are more
        argmts[0] = NULL; // Set the first argument as NULL
        return;
    }
    
    argmts[v] = NULL; // Set the last argument as NULL to show end of arguments
}


void execute_command_pipeline(char *commands[MAX_PIPELINE_COMMANDS], int num_commands) {

     char *home_dir = getenv("HOME");
    int pipes[MAX_PIPELINE_COMMANDS - 1][2]; // Array to store file descriptors of pipes

    for (int i = 0; i < num_commands - 1; i++) {
        if (pipe(pipes[i]) < 0) {
            printf("Pipe creation error"); // Display error message for pipe creation
            exit(EXIT_FAILURE); // Exit program with a failure status
        }
    }

    for (int i = 0; i < num_commands; i++) {
        pid_t pid = fork(); // Create a child process and get its process ID
        if (pid < 0) {
            printf("Fork error"); // Display error message for fork
            exit(EXIT_FAILURE); // Exit program with a failure status
        } else if (pid == 0) {
            // Child process
            if (i != 0) {
                dup2(pipes[i - 1][0], STDIN_FILENO); // Redirect standard input to read from previous pipe
                close(pipes[i - 1][0]); // Close read end of the previous pipe
            }
            if (i != num_commands - 1) {
                dup2(pipes[i][1], STDOUT_FILENO); // Redirect standard output to write to current pipe
                close(pipes[i][1]); // Close write end of current pipe
            }

            for (int j = 0; j < num_commands - 1; j++) {
                close(pipes[j][0]);
                close(pipes[j][1]);
            }

            char* command_arguments[MAX_COMMAND_ARGUMENTS]; // Array to store command arguments
            parse_command(commands[i], command_arguments); // Parse the current command

            for (int k = 0; command_arguments[k] != NULL; k++) {
                if(strstr(command_arguments[k], "~")) { // If argument contains '~' sign
                    char path[MAX_PATH_LENGTH]; // Buffer to store the new path
                    snprintf(path, sizeof(path), "%s%s", home_dir, command_arguments[k] + 1); // Replace '~' with home directory path
                    command_arguments[k] = path; // Update argument with new path
                }
            }

            execvp(command_arguments[0], command_arguments); // Execute the command with arguments
            printf("execvp"); // Display an error message if execvp returns
            exit(EXIT_FAILURE); // Exit child process with a failure status
        }
    }

    for (int i = 0; i < num_commands - 1; i++) {
        close(pipes[i][0]);
        close(pipes[i][1]);
    }

    for (int i = 0; i < num_commands; i++) {
        wait(NULL); // Wait for all child processes to finish
    }
}


void execute_command(char *command, char *args[MAX_ARGS], char *special_chars[MAX_SPECIAL_CHARS], int argc);

void remove_leading_whitespace(char *arguments[]) {
    for (int i = 0; arguments[i] != NULL; i++) {
        char *arg = arguments[i];
        // Find the first non-whitespace character
        while (isspace(*arg)) {
            arg++;
        }
        // Shift the argument to remove leading whitespace
        memmove(arguments[i], arg, strlen(arg) + 1);
    }
}
/***
The function parses the input string, separates arguments 
and special characters, and counts the number of arguments (arg_count)
*/
void parse_input(char *input, char *arguments[20], char *special_chars[20], int *arg_count) {
    char *token = strtok(input, " \n\t|"); // string tokenizer function in C, which splits a string into a series of tokens based on a set of delimiter characters
    int num_args = 0;
    int num_special_chars = 0;

    while (token != NULL) {
        if (num_args < MAX_ARGS) {
            arguments[num_args++] = token;
        } else {
            fprintf(stderr, "arguments are more than expected.\n");
            exit(EXIT_FAILURE);
        }

        // Check for special characters
        if (strchr("#|><&;", token[0]) != NULL) {
            if (num_special_chars < MAX_SPECIAL_CHARS) {
                special_chars[num_special_chars++] = token;
            } else {
                fprintf(stderr, "Too many special characters.\n");
                exit(EXIT_FAILURE);
            }
        }

        token = strtok(NULL, " \n\t|");
    }

         *arg_count = num_args;

        arguments[num_args] = NULL; // Terminate the arguments array at the last valid index
        special_chars[num_special_chars] = NULL; // Terminate the special_chars array at the last valid index
}

void addSpecialCharsarray(char *input,char *special_chars[20])
{
int specialCharCount = 0;

    // Tokenize the input string
    char *token = strtok(input, " ");

    while (token != NULL) {

        if (*token == '|' || (strcmp(token, "&&") || *token ==';' == 0)) { // Compare with string "&&"
            if (specialCharCount < MAX_SPECIAL_CHARS - 1) { // Avoid overflow
                special_chars[specialCharCount++] = token;
            } else {
                fprintf(stderr, "Too many special characters.\n");
                exit(EXIT_FAILURE);
            }
        }
        token = strtok(NULL, " "); // Move to the next token
    }

    special_chars[specialCharCount] = NULL; // Terminate the special_chars array at the last valid index
}
void parse_input_with_both(char *input, char *arguments[20], char *special_chars[20], int *arg_count)
{
char *token = strtok(input, "||"); // string tokenizer function in C, which splits a string into a series of tokens based on a set of delimiter characters
    int num_args = 0;
    int num_special_chars = 0;

    while (token != NULL) {
        if (num_args < MAX_ARGS) {
            arguments[num_args++] = token;
        } else {
            fprintf(stderr, "arguments are more than expected.\n");
            exit(EXIT_FAILURE);
        }

        // Check for special characters
        if (strchr("#|><&;", token[0]) != NULL) {
            if (num_special_chars < MAX_SPECIAL_CHARS) {
                special_chars[num_special_chars++] = token;
            } else {
                fprintf(stderr, "Too many special characters.\n");
                exit(EXIT_FAILURE);
            }
        }

        token = strtok(NULL, "||");
    }

         *arg_count = num_args;

        arguments[num_args] = NULL; // Terminate the arguments array at the last valid index
        special_chars[num_special_chars] = NULL; // Terminate the special_chars array at the last valid index
}

void parse_input_with_and(char *input, char *arguments[20], char *special_chars[20], int *arg_count)
{
    char *token = strtok(input, ";&&||"); // string tokenizer function in C, which splits a string into a series of tokens based on a set of delimiter characters
    int num_args = 0;
    int num_special_chars = 0;

    while (token != NULL) {
        if (num_args < MAX_ARGS) {
            arguments[num_args++] = token;
        } else {
            fprintf(stderr, "arguments are more than expected.\n");
            exit(EXIT_FAILURE);
        }

        // Check for special characters
        if (strchr("#|><&;", token[0]) != NULL) {
            if (num_special_chars < MAX_SPECIAL_CHARS) {
                special_chars[num_special_chars++] = token;
            } else {
                fprintf(stderr, "Too many special characters.\n");
                exit(EXIT_FAILURE);
            }
        }

        token = strtok(NULL, ";&&||");
    }
    *arg_count = num_args;

        arguments[num_args] = NULL; // Terminate the arguments array at the last valid index
        special_chars[num_special_chars] = NULL; // Terminate the special_chars array at the last valid index
}


void handlethenewcommand() {

    int status =  execlp("xterm", "xterm", "-e", "./shell24", NULL);
    if (status == -1) {
        perror("system command failed");
    }
    
}

void expand_tilde(char *args[]) {
    char *home_dir = getenv("HOME"); // Get the user's home directory
    if (home_dir == NULL) {
        struct passwd *pw = getpwuid(getuid());
        if (pw != NULL) {
            home_dir = pw->pw_dir;
        }
    }

    if (home_dir != NULL) {
        for (int i = 0; args[i] != NULL; i++) {
            if (args[i][0] == '~' && (args[i][1] == '\0' || args[i][1] == '/')) {
                // Replace ~ with the user's home directory path
                char *expanded_path = malloc(strlen(home_dir) + strlen(args[i]) + 1);
                if (expanded_path != NULL) {
                    strcpy(expanded_path, home_dir);
                    strcat(expanded_path, args[i] + 1); // Skip the ~ character
                    args[i] = expanded_path; // Replace the original argument with the expanded path
                } else {
                    fprintf(stderr, "Memory allocation failed.\n");
                    exit(EXIT_FAILURE);
                }
            }
        }
    }
}

void handle_special_characters(char *args[MAX_ARGS], char *special_chars[MAX_SPECIAL_CHARS]) {

      int concatenation_done = 0;
      for (int i = 0; special_chars[i] != NULL; i++) {
         if (special_chars[i][0] == '#'&& !concatenation_done) {
            char *str = "#";
             // File concatenation operation
             char *cat_args[MAX_ARGS]; // Arguments for cat command
             cat_args[0]="cat";
             int j = 0;
             int k =1;
             while (args[j] != NULL) {
                  if (strcmp(args[j], "#") == 0) {
                        j++;
                        continue;
                        }
                        else
                        {
                            cat_args[k] = args[j];
                            j++;
                            k++;
                        }
             }
             cat_args[k] = NULL; // Terminate the arguments array for cat
            pid_t pid = fork();
            if (pid < 0) {
                perror("fork");
                exit(EXIT_FAILURE);
            } else if (pid == 0) {
                // Child process
                execvp("cat", cat_args);
                perror("execvp");
                exit(EXIT_FAILURE); // Terminate if execvp fails
            } else {
                // Parent process
                wait(NULL); // Wait for the child process to complete
                concatenation_done = 1;
            }
        }
     }
}

void handle_special_characters_ls(char *args[MAX_ARGS]) {
    
    int status;
    char *delimiter = "<<"; // Delimiter to split the string
    char *filename;

    // Find the position of the delimiter in the string
    char *token = strtok(args[1], delimiter);

    // Traverse through the tokens to find the filename
    while (token != NULL) {
        filename = token; // Update the filename
        token = strtok(NULL, delimiter); // Get the next token
    }
    char *output = args[0];
    char *input = filename;
    
    char *arg[] = {"cat", NULL, NULL};
    pid_t pid;

    // Fork a child process
    pid = fork();

    if (pid < 0) {
        perror("Fork failed");
        exit(EXIT_FAILURE);
    } else if (pid == 0) {
        // Inside the child process
        // Redirect stdout to output file and append if required
            freopen(output, "a", stdout);

        // Set the arguments for cat command
        arg[1] = input;

        // Execute cat command with input file
        execvp(arg[0], arg);

        // If execvp returns, it means it failed
        perror("execvp failed");
        exit(EXIT_FAILURE);
    } else {
        // Inside the parent process
        // Wait for the child to finish
        waitpid(pid, &status, 0);

        if (WIFEXITED(status) && WEXITSTATUS(status) == EXIT_SUCCESS) {
            printf("Contents of %s appended to %s successfully.\n", input, output);
        } else {
            printf("Appending contents failed.\n");
        }
    }

}
void handle_special_characters_gt(char *args[MAX_ARGS]) {
    
    int status;
    char *input = args[0];
    char *delimiter = ">>"; // Delimiter to split the string
    char *filename;

    // Find the position of the delimiter in the string
    char *token = strtok(args[1], delimiter);

    // Traverse through the tokens to find the filename
    while (token != NULL) {
        filename = token; // Update the filename
        token = strtok(NULL, delimiter); // Get the next token
    }
    char *output = filename;
    
    char *arg[] = {"cat", NULL, NULL};
    pid_t pid;

    // Fork a child process
    pid = fork();

    if (pid < 0) {
        perror("Fork failed");
        exit(EXIT_FAILURE);
    } else if (pid == 0) {
        // Inside the child process
        // Redirect stdout to output file and append if required
            freopen(output, "a", stdout);

        // Set the arguments for cat command
        arg[1] = input;

        // Execute cat command with input file
        execvp(arg[0], arg);

        // If execvp returns, it means it failed
        perror("execvp failed");
        exit(EXIT_FAILURE);
    } else {
        // Inside the parent process
        // Wait for the child to finish
        waitpid(pid, &status, 0);

        if (WIFEXITED(status) && WEXITSTATUS(status) == EXIT_SUCCESS) {
            printf("Contents of %s appended to %s successfully.\n", input, output);
        } else {
            printf("Appending contents failed.\n");
        }
    }

}

void handle_special_characters_pipes(char *args[MAX_ARGS], char *special_chars[MAX_SPECIAL_CHARS], int argc)
{
 // dummy
}


void execute_command_and(char *command,char *args[MAX_ARGS], char *special_chars[MAX_SPECIAL_CHARS], int argc) {

 pid_t pid = fork();
        if (pid < 0) {
            perror("fork");
            exit(EXIT_FAILURE);
        } else if (pid == 0) {
            expand_tilde(args);
            execvp(args[0], args);
            perror("execvp");
            exit(EXIT_FAILURE);
        } else {
            // Parent process
            wait(NULL);
        }
}
void execute_command_or(char *command,char *args[MAX_ARGS], char *special_chars[MAX_SPECIAL_CHARS], int argc)
{
   
    pid_t pid = fork();
        if (pid < 0) {

            perror("fork");
            exit(EXIT_FAILURE);
        } else if (pid == 0) {
            expand_tilde(args);
            execvp(args[0], args);
            flag = 1;
        } else {
            // Parent process
            wait(NULL);
        }

}
void handle_special_characters_or(char *args[MAX_ARGS], int argc)
{
   int argc2;
   int count =0;
    char *special_chars2[20];
    char *arguments[MAX_ARGS];
    for (int i = 0; i < argc; i++) {
            parse_input(args[i],arguments,special_chars2, &argc2);
            if(flag ==0 && count == 0){
            execute_command_or(args[i],arguments, special_chars2, argc2);
            count++;
            }
            else if(flag == 1 && count > 0)
            {
                execute_command_or(args[i],arguments, special_chars2, argc2);
                count++;
            }
    }
}

void handle_special_characters_and(char *args[MAX_ARGS], int argc)
{
   // printf("I am here in special\n");
    int argc2;
    char *special_chars2[20];
    char *arguments[MAX_ARGS];
    for (int i = 0; i < argc; i++) {
            parse_input(args[i],arguments,special_chars2, &argc2);
            execute_command_and(args[i],arguments, special_chars2, argc2);
    }
}


void execute_command_both(char *command, char *args[MAX_ARGS], char *special_chars[MAX_SPECIAL_CHARS], int argc) {
    pid_t pid;
    int status;

    char *arguments1[20];
    char *arguments2[20];
    int argcForFirst;
    int argcForSecond;

    // Parse arguments for the first command
    parse_input_with_and(args[0], arguments1, special_chars, &argcForFirst);
    remove_leading_whitespace(arguments1);

    // Parse arguments for the second command
    parse_input_with_and(args[1], arguments2, special_chars, &argcForSecond);
    remove_leading_whitespace(arguments2);

    // Fork a child process for the first command
    pid = fork();
    if (pid < 0) {
        // Fork failed
        perror("fork");
        exit(EXIT_FAILURE);
    } else if (pid == 0) {
            int argc2;
            char *special_chars2[20];
            char *arguments[MAX_ARGS];
            for (int i = 0; i < 2; i++) {
                    parse_input(arguments1[i],arguments,special_chars2, &argc2);
                        expand_tilde(args);
                        execvp(arguments[0], arguments);
                        perror("execvp");
                }

                exit(EXIT_SUCCESS);
     }
        // Inside the child process for the first command
        //handle_special_characters_and(arguments1, argcForFirst);
         // Exit the child process after executing the command
         else {
        // Inside the parent process
        // Wait for the first command to finish
        waitpid(pid, &status, 0);

        // If the first command failed, execute the second set of commands
        if (WIFEXITED(status) && WEXITSTATUS(status) != 0) {
            // Fork another child process for the second command
            pid = fork();
            if (pid < 0) {
                // Fork failed
                perror("fork");
                exit(EXIT_FAILURE);
            } else if (pid == 0) {
                // Inside the child process for the second command
                handle_special_characters_and(arguments2, argcForSecond);
                exit(EXIT_SUCCESS); // Exit the child process after executing the command
            } else {
                // Inside the parent process
                // Wait for the second command to finish
                waitpid(pid, NULL, 0);
            }
        }
    }
}




void execute_command(char *command,char *args[MAX_ARGS], char *special_chars[MAX_SPECIAL_CHARS], int argc) {
    // Check if there are any special characters
    if (special_chars[0] == NULL) {
      // printf("I have entered this if\n");
        // No special characters, execute command directly
        pid_t pid = fork();
        if (pid < 0) {
            perror("fork");
            exit(EXIT_FAILURE);
        } else if (pid == 0) {
            // Child process
             //execlp("/bin/sh", "/bin/sh", "-c", command, NULL);
            expand_tilde(args);
            execvp(args[0], args);
            perror("execvp");
            exit(EXIT_FAILURE);
        } else {
            // Parent process
            wait(NULL);
        }
    } else {
        // Handle special characters
        // Implemented handlers for different special characters
        // Example: handle piping, redirection, background processing, etc.
        for (int i = 0; special_chars[i] != NULL; i++) {
            if (special_chars[i][0] == '#') {
                printf("%c",special_chars[i][0]);
                handle_special_characters(args, special_chars);
                break; // Exit the loop after handling #
            }
            else if(special_chars[i][0] == '|' && special_chars[i][1]=='|')
            {
                handle_special_characters_or(args,argc);
                break;
            }
            else if (special_chars[i][0] == '|') {
                handle_special_characters_pipes(args, special_chars,argc);
                break; 
            }
            else if (special_chars[i][0] == '>') {
                handle_special_characters_gt(args);
                break; 
            }
            else if(special_chars[i][0] == '<')
            {
                handle_special_characters_ls(args);
            }
            else if(special_chars[i][0] == '&' && special_chars[i][1]=='&')
            {
                handle_special_characters_and(args,argc);
                break;
            }
            else if(special_chars[i][0] == ';')
            {
                handle_special_characters_and(args,argc);
                break;
            }
        }
    }
}

void expand_wildcards(char *args[MAX_ARGS]) {
    glob_t glob_result;
    int flags = GLOB_NOCHECK | GLOB_TILDE;
    int new_arg_count = 0; // Counter for the new argument count after expansion

    for (int i = 0; args[i] != NULL; i++) {
        if (strstr(args[i], "*") != NULL || strstr(args[i], "?") != NULL) {
            if (glob(args[i], flags, NULL, &glob_result) == 0) {
                // Replace the wildcard argument with the expanded filenames
                for (size_t j = 0; j < glob_result.gl_pathc; j++) {
                    args[new_arg_count++] = strdup(glob_result.gl_pathv[j]);
                }
                globfree(&glob_result);
            } else {
                fprintf(stderr, "Failed to expand wildcard for argument: %s\n", args[i]);
                exit(EXIT_FAILURE);
            }
        } else {
            // If no wildcard found, keep the original argument
            args[new_arg_count++] = args[i];
        }
    }

    // Ensure the last argument in the expanded list is NULL
    args[new_arg_count] = NULL;
}


void background_process(char *command, char *args[MAX_ARGS], char *special_chars[MAX_SPECIAL_CHARS], int argc) {
    pid_t pid = fork();
    if (pid < 0) {
        perror("fork");
        exit(EXIT_FAILURE);
    } else if (pid == 0) {
        // Inside the child process
        printf("Background process with PID: %d\n", getpid());
        execute_command(command, args, special_chars, argc);
        exit(EXIT_SUCCESS);
    } else {
        // Inside the parent process
        printf("Background process started with PID: %d\n", pid);
    }
}


int main() {
    char command[1024];
    char *arguments[20];
    char *special_chars[20];
    char copied_command[1024]; 
    char copied2_command[1024];
    char copied3_command[1024];
    char *commands[5]; // Add this line to declare the commands array
    while(1)
    {
        printf("shell24$ ");
        fflush(stdout); // we used this so that user sees the promot imigiately without any delay or waiting for buffer to fill up

        // benefits of using fgets over scanf is that it helps in error and white space handeling and input parsing       
        if (fgets(command, sizeof(command), stdin) == NULL) {
            perror("fgets failed");
            continue;
        }

      // Remove newline character from the end of the command
        command[strcspn(command, "\n")] = '\0';
         strcpy(copied_command, command);
        command[strcspn(copied_command, "\n")] = '\0';
        strcpy(copied2_command, command);
        command[strcspn(copied2_command, "\n")] = '\0';
        strcpy(copied3_command, command);
        command[strcspn(copied3_command, "\n")] = '\0';

        // Check for builin commands
        if (strcmp(command, "newt") == 0) {
            handlethenewcommand();
            continue; // Skip the rest of the loop iteration
        }
        
        int argc;
        if(strstr(command, "|")!= NULL)
        {
                parse_input_with_both(command, arguments, special_chars, &argc);
                addSpecialCharsarray(copied_command,special_chars);
                remove_leading_whitespace(arguments);
        }
        else if (strstr(command, "&&") && strstr(command, "||")  != NULL)
        {
                parse_input_with_both(command, arguments, special_chars, &argc);

        }
        else if (strstr(command, "&&") || strstr(command, "||") || strstr(command, ";")  != NULL) {
            parse_input_with_and(command, arguments, special_chars, &argc);
            addSpecialCharsarray(copied_command,special_chars);
            remove_leading_whitespace(arguments);
        }
        else{
        parse_input(command, arguments, special_chars, &argc);
        }

// printf("Arguments after parsing:\n");
//  for (int i = 0; i < argc; i++) {
//             printf("Argument %d: %s\n", i, arguments[i]);
//         }
        
        // Check if argc satisfies Rule 2
        if (argc < 1 || argc > 20) {
            printf("Invalid number of arguments. argc must be between 1 and 5.\n");
            continue;
        } 

         expand_wildcards(arguments);
        // printf("Special car \n");
        //  for (int i = 0; special_chars[i] != NULL; i++) {
        //      printf("%c",special_chars[i][0]);
        //  }
        if (strstr(copied2_command, "&") != NULL && strstr(copied2_command, "&&")== NULL){
             background_process(command, arguments, special_chars, argc); // Added line
        } 
        else if (strstr(copied2_command, "&&") && strstr(copied2_command, "||")  != NULL)
        {
                execute_command_both(command,arguments, special_chars, argc);    
        }
        else if ((strstr(copied3_command, "|") != NULL)) {
                 execute_command_pipeline(arguments, argc);
        }
        else{
         execute_command(command,arguments, special_chars, argc);    
        }   
    }
    return 0;
}
