# Assignment 3 – Simple Shell

## Overview
This project implements a **simple Linux shell** written in C.  
The shell runs on top of the standard Linux command-line interpreter and is capable of
executing user commands, handling command-line arguments, supporting pipes, and displaying
process execution status.

The purpose of this assignment is to gain hands-on experience with **process creation,
command execution, inter-process communication (pipes), and low-level UNIX system calls**.

### Basic Shell Functionality
- Reads user input line by line (up to 187 bytes)
- Parses commands and arguments
- Creates child processes using `fork()`
- Executes commands using `execvp()`
- Waits for child processes to finish
- Displays child PID and exit status after execution

### Command Execution
- Supports both absolute paths (e.g., `/bin/ls`)
- Supports PATH-based command lookup (e.g., `ls`)
- Handles at least **four command-line arguments** per command

### Input Handling
- Prevents buffer overflow by limiting input size
- Gracefully exits on EOF (`Ctrl-D`)
- Reports errors for empty input lines
- Truncates input safely if it exceeds buffer size

### Pipe Support
- Supports piping using the `|` operator
- Redirects `stdout` of one command to `stdin` of the next
- Supports multiple chained pipes
- Uses `pipe()` and `dup2()` for file descriptor redirection

### Basic Shell Functionality
- Reads user input line by line (up to 187 bytes)
- Parses commands and arguments
- Creates child processes using `fork()`
- Executes commands using `execvp()`
- Waits for child processes to finish
- Displays child PID and exit status after execution

### Command Execution
- Supports both absolute paths (e.g., `/bin/ls`)
- Supports PATH-based command lookup (e.g., `ls`)
- Handles at least **four command-line arguments** per command

### Input Handling
- Prevents buffer overflow by limiting input size
- Gracefully exits on EOF (`Ctrl-D`)
- Reports errors for empty input lines
- Truncates input safely if it exceeds buffer size

### Pipe Support
- Supports piping using the `|` operator
- Redirects `stdout` of one command to `stdin` of the next
- Supports multiple chained pipes
- Uses `pipe()` and `dup2()` for file descriptor redirection

## Sample Execution

```bash
$ ./assn3 "prompt$ "
prompt$ ls -l
total 20
-rwxr-xr-x 1 student student 13216 Feb 23 13:44 assn3
-rw-r--r-- 1 student student  1583 Feb 23 13:44 assn3.c
Child 2124, exited with 0

prompt$ ls foo
ls: cannot access 'foo': No such file or directory
Child 2125, exited with 2

prompt$ exit
$