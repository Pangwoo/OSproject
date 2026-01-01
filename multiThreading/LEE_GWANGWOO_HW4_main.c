/**************************************************************
* Class:  CSC-415-0# Spring 2023
* Name: Gwangwoo Lee
* Student ID: 922784535
* GitHub UserID: Pangwoo
* Project: Assignment 4 – Word Blast
*
* File: LEE_GWANGWOO_HW4_main.c
*
* Description: A multithreaded C program that analyzes a large text file
(*War and Peace*) and counts the frequency of words that are **six or more
characters long**.
**************************************************************/
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <pthread.h>
#include <string.h>

#define DICT_INIT_SIZE 20
#define DICT_MAX_SIZE 10240

typedef struct Element{
    char* key;
    int value;
}Element;

typedef struct Dictionary {
    int size;
    Element** elements;
    int count;
    int* order;
}Dictionary;

pthread_mutex_t mutex1 = PTHREAD_MUTEX_INITIALIZER;

int fd, chunk, mod;
char* filename;
int threadCnt;
int count = 0;
Dictionary* globalDictionary;

Element* elem_init(char* ptr);
Dictionary* dict_init();
int increase_mem(Dictionary* d);
Element* dup_element(Element* e);
int search_word(Dictionary* d, char* ptr);
int insert_word(Dictionary* d, char* ptr);
int increase_word(Dictionary* d, int idx, int val);
int insert_element(Dictionary* d, Element* e);
int merge_dictionary(Dictionary* d1, Dictionary* d2);
int free_dictionary(Dictionary* d);
int free_element(Element* e);
int search_order();
int insert_order();

//thread function
void* word_count(void *ptr);

// You may find this Useful
char * delim = "\"\'.“”‘’?:;-,—*($%)! \t\n\x0A\r";

int main (int argc, char *argv[])
{   
    // check # of args these must be 3
    // #1: object file name
    // #2: fileName
    // #3: # of threads
    if(argc != 3){
        printf("error: wrong args. please enter filename and number of threads\n");
        return -1;
    }

    // init all the varialbes to run threads
    filename = argv[1];
    threadCnt = atoi(argv[2]);
    globalDictionary = dict_init();

    fd = open(filename, O_RDONLY);
    off_t fileSize = lseek(fd, 0, SEEK_END);
    chunk = fileSize / threadCnt;
    mod = fileSize % threadCnt;
    lseek(fd, 0, SEEK_SET);

    pthread_t threads[threadCnt];
    pthread_attr_t attr;
    int rcs[threadCnt];
    int idxs[threadCnt];

    //**************************************************************
    // DO NOT CHANGE THIS BLOCK
    //Time stamp start
    struct timespec startTime;
    struct timespec endTime;

    clock_gettime(CLOCK_REALTIME, &startTime);
    //**************************************************************
    // thread processing
    pthread_attr_init(&attr);

    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

    for(int i = 0; i < threadCnt; i++){
        idxs[i] = i+1;
        rcs[i] = pthread_create(&threads[i], &attr, word_count, (void *) &idxs[i]);
    }
    // wait for the threads to finish
    for(int i = 0; i < threadCnt; i++){
        pthread_join( threads[i], NULL );
    }
    // ***TO DO *** Process TOP 10 and display
    for(int i = 0; i < 10; i++){
        int idx = globalDictionary->order[i];
        Element* e = globalDictionary->elements[idx]; 
        printf("%d: word: %s count: %d\n", i+1, e->key, e->value);
    }

    //**************************************************************
    // DO NOT CHANGE THIS BLOCK
    // Clock output
    clock_gettime(CLOCK_REALTIME, &endTime);
    time_t sec = endTime.tv_sec - startTime.tv_sec;
    long n_sec = endTime.tv_nsec - startTime.tv_nsec;
    if (endTime.tv_nsec < startTime.tv_nsec)
        {
        --sec;
        n_sec = n_sec + 1000000000L;
        }

    printf("Total Time was %ld.%09ld seconds\n", sec, n_sec);
    //**************************************************************
    
    // cleanup memory
    int cl = close(fd);
    free_dictionary(globalDictionary);

    return 0;
}

// thread function
void* word_count(void* ptr){
    int i = *(int*)ptr;
    int bufSize = chunk;
    int fdOffset = (i - 1) * chunk;
    
    Dictionary* localDict = dict_init();

    if(i == threadCnt){
        bufSize += mod;
    }

    char* buffer = malloc(sizeof(char) * (bufSize + 1));
    if(!buffer) return NULL;
    ssize_t res = pread(fd, buffer, bufSize, fdOffset);
    
    if(res < 0){
        free(buffer);
        return NULL;
    }

    buffer[res] = '\0';

    char* save;
    char* token = strtok_r(buffer, delim, &save);

    while(token != NULL){
        if(strlen(token) < 6){
            token = strtok_r(NULL, delim, &save);
            continue;
        }

        int res = search_word(localDict, token);
        if(res < 0){
            insert_word(localDict, token);
        }else{
            increase_word(localDict, res, 1);
        }
        token = strtok_r(NULL, delim, &save);
    }
    free(buffer);
    pthread_mutex_lock( &mutex1);
    merge_dictionary(globalDictionary, localDict);
    pthread_mutex_unlock( &mutex1);

    
    free_dictionary(localDict);
}

Element* elem_init(char* ptr){
    Element *e = malloc(sizeof(Element));
    e->key = strdup(ptr);
    e->value = 1;
    return e;
}

// dictionary initiaize function
Dictionary* dict_init(){
    Dictionary *d = malloc(sizeof(Dictionary));
    d->size = DICT_INIT_SIZE;
    d->count = 0;
    d->elements = malloc(sizeof(Element*) * DICT_INIT_SIZE);
    d->order = malloc(sizeof(int) * DICT_INIT_SIZE);
    for(int i = 0; i < DICT_INIT_SIZE; i++){
        d->order[i] = -1;
    }
    return d;
}

// helper function that increase the memory of dictionary
int increase_mem(Dictionary *d){
    if(d->size >= DICT_MAX_SIZE){
        return -1;
    }
    int new_size = d->size * 2;
    if(new_size > DICT_MAX_SIZE){
        new_size = DICT_MAX_SIZE;
    }
    Element** new_mem = realloc(d->elements, sizeof(Element*) * new_size);
    int* new_order = realloc(d->order, sizeof(int) * new_size);
    if(new_mem == NULL){
        return -1;
    }

    if(new_order == NULL){
        return -1;
    }
    for(int i = d->size; i < new_size; i++){
        new_order[i] = -1;
    }

    d->size = new_size;
    d->elements = new_mem;
    d->order = new_order;
    return new_size;
}

// helper function that find ptr in d 
// it returns ptr's idx if it exists
// else return -1
int search_word(Dictionary* d, char* ptr){
    int cnt = d->count;
    for(int i = 0; i < cnt; i++){
        if(strcmp(ptr, d->elements[i]->key) == 0){
            return i;
        }
    }

    return -1;
}

// helper function that add new element in dictionary
int insert_word(Dictionary* d, char* ptr){
    int idx = d->count;

    // check capacity in dictionary and increase memory if it is needed
    if(d->size == d->count){
        int res = increase_mem(d);
        if( res < 0 ){
            return -1;
        }
    }

    // init new element
    Element* element = elem_init(ptr);

    //
    if(idx == 0){
        d->elements[idx] = element;
        d->order[idx] = idx;
        d->count++;
        return idx;
    }

    // loop to find position in lexically
    for(int i = 0; i < d->count; i++){
        if(strcmp(ptr, d->elements[i]->key) <= 0){
            idx = i;
            break;
        }
    }

    // push elements that come behind of new element
    if(idx < d->count){
        for(int i = d->count; i > idx; i--){
            d->elements[i] = d->elements[i-1];
        }
    }

    // change the value that was pushed by new element
    for(int i = 0; i < d->count; i++){
        if(d->order[i] >= idx){
            d->order[i]++;
        }
    }

    d->elements[idx] = element;
    d->order[d->count] = idx; 
    d->count++;
    
    return idx;    
}

int increase_word(Dictionary* d, int idx, int val){
    if(idx < 0 || d->count <= idx){
        return -1;
    }

    int order_idx = -1;
    for(int i = 0; i < d->count; i++){
        if(d->order[i] == idx){
            order_idx = i;
            break;
        }
    }

    if(order_idx < 0){
        return -1;
    }

    d->elements[idx]->value += val;
    int new_value = d->elements[idx]->value;

    while(order_idx > 0){
        int prev_idx = d->order[order_idx - 1];
        int prev_value = d->elements[prev_idx]->value;
        if(new_value <= prev_value){
            break;
        }
        d->order[order_idx] = prev_idx;
        d->order[order_idx - 1] = idx;
        order_idx--;
    }

    return 0;
}

int insert_element(Dictionary* d, Element* e){
    int idx = d->count;

    if(d->size == d->count){
        int res = increase_mem(d);

        if( res < 0 ){
            return -1;
        }
    }

    Element* element = dup_element(e);

    if(idx == 0){
        d->elements[idx] = element;
        d->order[idx] = idx;
        d->count++;
        return 0;
    }

    for(int i = 0; i < d->count; i++){
        if(strcmp(e->key, d->elements[i]->key) < 0){
            idx = i;
            break;
        }
    }

    if(idx < d->count){
        for(int i = d->count; i > idx; i--){
            d->elements[i] = d->elements[i-1];
        }
    }

    for(int i = 0; i < d->count; i++){
        if(d->order[i] >= idx){
            d->order[i]++;
        }
    }

    
    d->elements[idx] = element;
    d->order[d->count] = idx;
    d->count++;

    int order_idx = d->count-1;
    int cur_val = element->value;

    while(order_idx > 0){
        int prev_idx = d->order[order_idx - 1];
        int prev_val = d->elements[prev_idx]->value;

        if(prev_val >= cur_val){
            break;
        }

        d->order[order_idx] = prev_idx;
        d->order[order_idx - 1] = idx;

        order_idx--;
    }

    return idx;  
}

int merge_dictionary(Dictionary* d1, Dictionary* d2){
    for(int i = 0; i < d2->count; i++){
        int res = search_word(d1, d2->elements[i]->key);
        if(res >= 0){
            increase_word(d1, res, d2->elements[i]->value);
        }else{
            insert_element(d1, d2->elements[i]);
        }
    }

    return 0;
}

Element* dup_element(Element* e){
    Element* element = malloc(sizeof(Element));
    element->key = strdup(e->key);
    element->value = e->value;
    return element;
}

int free_element(Element* e){
    if(!e) return 0;
    free(e->key);
    free(e);
    return 0;
}

int free_dictionary(Dictionary* d){
    if(!d) return 0;
    for(int i = 0; i < d->count; i++){
        free_element(d->elements[i]);
    }
    free(d->elements);
    free(d->order);
    free(d);

    return 0;
}