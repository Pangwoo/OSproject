/**************************************************************
* Class:  CSC-415-0# Fall 2023
* Name: GWANGWOO LEE
* Student ID: 922784535
* GitHub ID: pangwoo
* Project: Assignment 3 – Simple Shell
*
* File: LEE_GWANGWOO_HW3_main.c
*
* Description: This project implements a **simple Linux shell** written in C.  
* The shell runs on top of the standard Linux command-line interpreter and is capable of
* executing user commands, handling command-line arguments, supporting pipes, and displaying
* process execution status.
**************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>
#include <errno.h>
#include <glob.h>

#define BUFFER_SIZE 187
#define PRINT_STR(str) printf("%s\n", str)
#define PRINT_CHAR(char) printf("%c\n", char)
#define PRINT_INT(int) printf("%d\n", int)

char** simpleSplitor(char* commands);
char** applyGlob(char** args);
char*** pipeSplitor(char* cmds, int cnt);
int cntPipe(char* cmd);
void freeCmd(char** cmd);

int main(int argc, char* argv[]){
    while(1){
        // limit the max len of cmd
        char* buffer = malloc(BUFFER_SIZE);
        
        if(argc > 1){
            printf("%s", argv[1]);
        }else{
            printf("> ");
        }

        fgets(buffer, sizeof(char) * BUFFER_SIZE, stdin);

        // check EOF
        if( *(buffer) == '\0'){
            printf("No more commands goodbye!\n");
            exit(EXIT_SUCCESS);
        }

        // check empty cmd
        if( *(buffer) == '\n' || *(buffer) == ' ' || *(buffer) == '\t'){
            printf("Enter commands again!\n");
            continue;
        }
        
        // split the cmd by pipes and then spilt the cmd by options
        int cnt = cntPipe(buffer) + 1;
        int fdCnt = cnt-1;
        char*** mulCmdArr = pipeSplitor(buffer, cnt);
        
        // check terminator
        if(strcmp(mulCmdArr[0][0], "exit") == 0){
            printf("goodbye!\n");
            return 0;
        }

        // using pipe to connect with each process
        int fd[fdCnt][2];
        pid_t pid;
        // check pipe init error
        for(int i = 0; i < fdCnt; i++){
            if(pipe(fd[i]) == -1){
                printf("pipe error\n");
                exit(EXIT_FAILURE);
            }
        }

        // create processes as much as number of cmds
        for(int i = 0; i < cnt; i++){
            pid = fork();
            
            switch(pid){
                case -1:
                    printf("fork Error\n");
                    exit(EXIT_FAILURE);
                // child process handle
                // each child process take the cmd and then send result
                // to next child process using pipe out
                case 0:
                    int cur = -1;
                    int prev = -1;

                    if(i < fdCnt){
                        cur = i;
                    }

                    if(i > 0){
                        prev = i -1;
                    }

                    if(prev >= 0){
                        dup2(fd[prev][0], STDIN_FILENO);
                    }

                    if(cur >= 0){
                        dup2(fd[cur][1], STDOUT_FILENO);
                    }

                    for(int i = 0; i < fdCnt; i++){
                        close(fd[i][0]);
                        close(fd[i][1]);
                    }

                    char** cmd = applyGlob(mulCmdArr[i]);

                    if(execvp(cmd[0], cmd) == -1){
                        printf("%dth cmd error\n", i);
                        freeCmd(cmd);
                        exit(EXIT_FAILURE);
                    }
            }
        }
        
        // close all the pipes that not used
        for(int i = 0; i < fdCnt; i++){
            close(fd[i][0]);
            close(fd[i][1]);
        }

        // parent process wait until child process dead(finished)
        // then print info of the child process
        for(int i = 0; i < cnt; i++){
            int status;
            wait(&status);
            printf("Child %d, exit with %d\n", pid, status);
        }
    }
    return 0;
}

// helper function that split string by delim and return string array
char** simpleSplitor(char* commands){
    if(!commands){
        return NULL;
    }
    // init return value
    char** stringArray;
    stringArray = malloc( sizeof(char*) * (strlen(commands)/2 + 1));
    
    int idx = 0;
    int quoteFlag = 0;
    char* optPtr = NULL;
    char* ptr = commands;

    while(*ptr){
        if(*ptr == '"'){
            if(quoteFlag > 0){
                *(ptr) = '\0';
                quoteFlag = 0;
            }else{
                quoteFlag++;
                optPtr = ptr+1;
            }
        }
        else if(quoteFlag < 1 && *ptr == ' ' || *ptr == '\t' || *ptr == '\n'){
            if(optPtr){
                *ptr = '\0';
            }
        }else{
            if(!optPtr){
                optPtr = ptr;
            }
        }

        //
        if(!(*ptr) && optPtr){
            stringArray[idx++] = optPtr;
            optPtr = NULL;
        }

        ptr++;
    }

    return stringArray;
}

// helper function to handle wildcard '*' in command" 
char** applyGlob(char** args){
    char** final = malloc(sizeof(char*) * 256);
    int idx = 0;

    for(int i=0; args[i]; i++){
        if(strchr(args[i], '*') != NULL){
            glob_t g;
            glob(args[i], 0, NULL, &g);

            for(size_t j=0; j < g.gl_pathc; j++)
                final[idx++] = strdup(g.gl_pathv[j]);

            globfree(&g);
        } else {
            final[idx++] = strdup(args[i]);
        }
    }

    final[idx] = NULL;
    return final;
}

// helper function to free the memory 
void freeCmd(char** cmdArr){
    for(char** p = cmdArr; p; p++){
        free(p);
    }
    free(cmdArr);
}

// helper function to count the pipes | in commands
int cntPipe(char* cmd){
    int cnt = 0;
    for(char* p = cmd; *p; p++){
        if(*p == '|'){
            cnt++;
        }
    }
    return cnt;
}

// helper function to split the command by pipes | and 
// return array of cmd (cmd is array of string)
char*** pipeSplitor(char* cmds, int cnt){
    char*** result = malloc(sizeof(char***) * (cnt+1));
    int idx = 0;
    char* startPtr = NULL;
    char* ptr = cmds;
    while(*ptr){
        if(*ptr == '|'){
            *ptr = '\0';
            result[idx++] = simpleSplitor(startPtr);
            startPtr = NULL;
        }

        if(*ptr && !startPtr){
            startPtr = ptr;
        }
        ptr++;
    }

    if(startPtr){
        result[idx++] = simpleSplitor(startPtr);
    }
    result[idx++] = NULL;

    return result;
}