/**************************************************************
* Class:  CSC-415-0# Spring 2023
* Name: Gwangwoo Lee
* Student ID: 922784535
* GitHub UserID: Pangwoo
* Project: Assignment 2 – Stucture in Memory and Buffering
*
* File: LEE_GWANGWOO_HW2_main.c
*
* Description: This project focuses on low-level memory handling and data buffering in C.
* It involves allocating and populating structured data in memory, managing block-based buffering, 
* and interfacing with precompiled binary APIs to validate correctness.
**************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <assignment2.h>
#include <string.h>

int main(int argc, char** argv){
    // 
    personalInfo* pi = malloc(sizeof(personalInfo));
    pi->firstName = strdup(*(argv + 1));
    if(!pi->firstName){
        printf("strdup Error\n");
    }
    pi->lastName = strdup(*(argv + 2));
    if(!pi->lastName){
        printf("strdup Error\n");
    }
    pi->studentID = 922784535;
    pi->level = GRAD;

    int progLang = 0;

    pi->languages = KNOWLEDGE_OF_JAVA | KNOWLEDGE_OF_JAVASCRIPT | KNOWLEDGE_OF_PYTHON 
                | KNOWLEDGE_OF_CPLUSPLUS |KNOWLEDGE_OF_SQL | KNOWLEDGE_OF_HTML 
                | KNOWLEDGE_OF_MIPS_ASSEMBLER | KNOWLEDGE_OF_R;

    strncpy(pi->message, *(argv + 3), sizeof(pi->message));

    int res = writePersonalInfo(pi);
    if(res != 0){
        printf("Error %d\n", res);
    }
    
    free(pi->firstName);
    free(pi->lastName);
    free(pi);

    char* buffer = malloc(BLOCK_SIZE);
    int bufferIdx = 0;
    

    const char* curStr = getNext();
    
    // this loop take string by getNext and memcpy to the buffer by possible chunk
    // until getNext return NULL
    // check the curStr len and compare with the buffer's available space 
    // if buffer doesn't have enough space to take all curStr, then memcpy as available space
    // if buffer has enough space, then memcpy as much as curStr len 
    while(curStr != NULL){
        int availalbeSpace = BLOCK_SIZE - bufferIdx;
        int curLen = strlen(curStr);

        while(availalbeSpace < curLen){
            memcpy(buffer + bufferIdx, curStr, availalbeSpace);
            bufferIdx = 0;
            commitBlock(buffer);
            curStr = curStr + availalbeSpace;
            availalbeSpace = BLOCK_SIZE - bufferIdx;
            curLen = strlen(curStr);
        }
        memcpy(buffer + bufferIdx, curStr, curLen);
        bufferIdx += curLen;
        
        curStr = getNext();
    }

    if(bufferIdx > 0){
        commitBlock(buffer);
    }

    int checkRes = checkIt();   
    
    if(checkRes != 0){
        printf("CheckIt ERROR: %d\n", checkRes);
    }

    free(buffer);
    return 0;
;
}
