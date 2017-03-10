/*
 Bailey Kane
 February 17, 2017
 
 HW05 - Unix Shell
 
 Unix shell similar to "bash," called "nush."
 
 Usage:
    1.
    ./nush
    nush$ {command}
    [output]
    nush$ {command} {argument}
    [output]
    etc...
    
    2.
    ./nush {file}
    [output]
*/

// Includes for standard I/O, error handling, string processing, and general
//   functions.
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>

int exec_commands_std(char** commands);
int exec_commands(char* in, char* out, char** commands);
int op_select(char input[]);
void op_exec(int op, char input[]);
int run_command_set(char* input);
char** parse_char_arr(char* input, char delimiter[]);
// char* rm_spec_char(char* input);

int main(int argc, char *argv[]) {
    char input[256];
    
    int op;
    
    if(argc == 1) { // if receiving input from stdin
        printf("nush$ ");
        fflush(stdout);
        
        // 1. read input
        while (fgets(input, 256, stdin)) {
            
            if(strlen(input) > 1) {
                // 2. reset operator selection
                op = op_select(input);
                
                // 3. run input commands
                if(op != 0) { // If we have operators to deal with
                    op_exec(op, input);
                } else { // If no funny business, execute as planned
                    run_command_set(input);
                }
            }
            
            printf("nush$ ");
            fflush(stdout);
        }
    } else if(argc == 2) { // if receiving input from a file
        FILE* input_file = fopen(argv[1], "r"); // input file, read-only
        
        // 1. read input
        while (fgets(input, 256, input_file)) {
            
            if(strlen(input) > 1) {
                // 2. reset operator selection
                op = op_select(input);
                
                // 3. run input commands
                if(op != 0) { // If we have operators to deal with
                    op_exec(op, input);
                } else { // If no funny business, execute as planned
                    run_command_set(input);
                }
            }
            
            fflush(stdout);
        }
        
        fclose(input_file); // close input file
    } else { // otherwise, improper usage.
        printf("Improper usage.\nnush shell requires 0 or 1 arguments.\n");
    }
    
    return(0);
}

// convenience function to call exec_commands with stdin and stdout
int exec_commands_std(char** commands) {
    return exec_commands("", "", commands);
}

// create a process from command array, run it, wait until it completes, move on
int exec_commands(char* in, char* out, char** commands) {
    int status, fd_in, fd_out;
    int cpid = fork();
    
    switch (cpid) {
    case 0: // Child process
        if(strcmp(in, "") != 0) {
            fd_in = open(in, O_RDWR);
            dup2(fd_in, STDIN_FILENO);
            close(fd_in);
        }
        
        if(strcmp(out, "") != 0) {
            fd_out = open(out, O_RDWR|O_CREAT, S_IRWXU);
            dup2(fd_out, STDOUT_FILENO);
            close(fd_out);
        }
        
        execvp(commands[0], commands);
        perror("execvp");
        exit(1); // if reached, error
        break;
    case -1: // error
        perror("fork");
        exit(1);
        break;
    default: // Parent process
        waitpid(cpid, &status, 0);
        break;
    }
    
    return WEXITSTATUS(status);
}

int exec_pipe_commands(char** commands) {
    char** pipe_cmd1;
    char** pipe_cmd2;
    
    int status, pipes[2];
    int cpid = fork();
    int ppid;
        
    pipe_cmd1 = parse_char_arr(commands[0], " \n");
    pipe_cmd2 = parse_char_arr(commands[1], " \n");
    
    pipe(pipes);
    
    if (!cpid) {
        ppid = fork();
        if (!ppid) { // Child process
            close(pipes[1]);
            dup2(pipes[0], STDIN_FILENO);
            
            if (pipe_cmd2[0]) {
                execvp(pipe_cmd2[0], pipe_cmd2);
            }
        } else { // Parent process
            close(pipes[0]);
            dup2(pipes[1], STDOUT_FILENO);
            if (pipe_cmd1[0]) {
                execvp(pipe_cmd1[0], pipe_cmd1);
            }
        }
    } else {
        waitpid(cpid, &status, 0);
    }
    
    close(pipes[0]);
    close(pipes[1]);
    
    free(pipe_cmd1);
    free(pipe_cmd2);
    
    return WEXITSTATUS(status);
}

// run the input commands, either built-ins or programs
int run_command_set(char* input) {
    char** commands;
    int status;
    
    // 1. parse input
    commands = parse_char_arr(input, " \n");
    
    // 2. determine the execution strategy for input commands
    if(!commands) { // if error on parsing input
        return 1;
    } else if(strcmp(commands[0], "exit") == 0) { // handle "exit"
        free(commands);
        exit(0);
    } else if(strcmp(commands[0], "cd") == 0) { // handle "cd"
        status = chdir(commands[1]);
    } else { // otherwise, default behavior
        status = exec_commands_std(commands);
    }
    
    free(commands);
    
    return status;
}

// Identify operator for the input commands
int op_select(char input[]) {
    int op;

    // Check for all possible operators
    if(strstr(input, " < ")) {
        op = 1;
    } else if(strstr(input, " > ")) {
        op = 2;
    } else if(strstr(input, " | ")) {
        op = 3;
    } else if(strstr(input, " &\n")) {
        op = 4;
    } else if(strstr(input, " && ")) {
        op = 5;
    } else if(strstr(input, " || ")) {
        op = 6;
    } else if(strstr(input, ";")) {
        op = 7;
    } else { // Otherwise, no operator, default behavior
        op = 0;
    }
    
    return op;
}

// Executes functionality based on input operators
void op_exec(int op, char input[]) {
    int i;
    char** commands;
    
    switch(op) { // switch case based on selected operator
        case 1: // Redirect input <
            commands = parse_char_arr(input, " <\n");
            
            // increment i to the end of commands
            for(i = 0; commands[i]; i++);
            
            // execute commands
            // commands[i-1] is the last command and the input target
            exec_commands(commands[i-1], "", commands);
            
            free(commands);
            break;
        case 2: // Redirect output >
            commands = parse_char_arr(input, " >\n");
            
            // increment i to the end of commands
            for(i = 0; commands[i]; i++);
            
            // execute commands
            // commands[i-1] is the last command and the output target
            exec_commands("", commands[i-1], commands);
            
            free(commands);
            break;
        case 3: // Pipe |
            commands = parse_char_arr(input, "|\n");
            
            exec_pipe_commands(commands);
            
            free(commands);
            break;
        case 4: // Background &
            system(input); // send commands to be processed in the background
            break;
        case 5: // And &&
            commands = parse_char_arr(input, "&&\n");
            
            for(i = 0; commands[i]; i++) {
                if(run_command_set(commands[i]) > 0) { // if failure status
                    break;
                }
            }
            
            free(commands);
            break;
        case 6: // Or ||
            commands = parse_char_arr(input, "||\n");
            
            for(i = 0; commands[i]; i++) {
                if(run_command_set(commands[i]) == 0) { // if success status
                    break;
                }
            }
            
            free(commands);
            break;
        case 7: // Semicolon ;
            commands = parse_char_arr(input, ";\n");
            
            // run each ';'-separated command set
            for(i = 0; commands[i]; i++) {
                run_command_set(commands[i]);
            }
            
            free(commands);
            break;
    }
}

// parse the input string using the input delimiter
char** parse_char_arr(char* input, char delimiter[]) {
    char** commands = (char**)malloc(strlen(input) * sizeof(char*));
    char* cmd;
    int i = 0;
    
    // separate input by delimiter, store in commands array
    for(cmd = strtok(input, delimiter); cmd; i++) {
        commands[i] = cmd;
        cmd = strtok(NULL, delimiter);
    }
    
    commands[i] = NULL;

    return commands;
}

// // removes all characters with ASCII values <= 32 from the input char*
// char* rm_spec_char(char* input) {
//     int i, j;
//     char* output = (char*)malloc(strlen(input) * sizeof(char));
    
//     for(i = 0, j = 0; input[i]; i++) {
//         if(input[i] > 32) { // if no special character, copy
//             output[j] = input[i];
//             j++;
//         } else { // if special character, skip
//             continue;
//         }
//     }
    
//     return output;
// }