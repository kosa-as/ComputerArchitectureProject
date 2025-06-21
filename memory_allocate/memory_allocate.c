#include "../testing.h"
#include <memory.h>
#include <memory_show.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
/*
一些任务负责执行，一个任务负责一直记录
内部碎片：每次malloc的时候，调用钩子函数记录申请的大小和实际分配的大小（分配的时候，钩子函数还要去计算时间）
外部碎片：由负责记录的任务定时调用记录内存的情况（包括最大的可用块大小）
*/
#define MALLOC_NUM 5

typedef struct{
    u64 memory_malloc_size;
    u64 memory_allocate_size;
    u64 memory_malloc_cost;
    u64 memory_free_cost;
    double external_fragmentation_rate;
} statistics_t;  

typedef struct {
    statistics_t statistics;
    void* ptr;
} statistics_item_t;

typedef struct {
    int items[MALLOC_NUM];  // store the index of the statistics_item_t
    int top;  // the top pointer
} Stack;

// initialize the stack
void init_stack(Stack* stack) {
    stack->top = -1;  // the top pointer is initialized to -1, which means the stack is empty
}

// check if the stack is empty
int is_empty(Stack* stack) {
    return stack->top == -1;
}

// check if the stack is full
int is_full(Stack* stack) {
    return stack->top == MALLOC_NUM - 1;
}

// push the index into the stack
void push(Stack* stack, int index) {
    if (is_full(stack)) {
        printk("Stack is full, cannot push!\n");
        return;
    }
    stack->items[++stack->top] = index;  // push the index into the stack
}

// pop the pointer from the stack
int pop(Stack* stack) {
    if (is_empty(stack)) {
        printk("Stack is empty, cannot pop!\n");
        return -1;
    }
    return stack->items[stack->top--];  // pop the pointer from the stack
}

// peek the top pointer
int peek(Stack* stack) {
    if (is_empty(stack)) {
        printk("Stack is empty!\n");
        return -1;
    }
    return stack->items[stack->top];  // return the top pointer
}

statistics_item_t statistics[MALLOC_NUM];

int statistics_index = 0;
int free_index = 0;
meminfo_t mem_info;

int calculate_catalan_number(int n){
    if(n == 0){
        return 1;
    }
    return calculate_catalan_number(n - 1) * (4 * n - 2) / (n + 1);
}

void memory_hook_malloc(void* ptr, int size){
    memset(&mem_info, 0, sizeof(meminfo_t));
    u64 malloc_time = sys_timestamp();
    if(mem_getinfo(&mem_info) == 0){
        statistics[statistics_index].statistics.memory_allocate_size = mem_info.heapmemused - statistics[statistics_index].statistics.memory_allocate_size;
        statistics[statistics_index].statistics.memory_malloc_cost = malloc_time - statistics[statistics_index].statistics.memory_malloc_cost;
    }
}

void memory_hook_free(void* ptr){
    u64 free_time = sys_timestamp();
    statistics[free_index].statistics.memory_free_cost = free_time - statistics[free_index].statistics.memory_free_cost;
}

void generateMallocFreeSequence(int malloc_count, int free_count, char* sequence, int index, char** sequences, int* seq_index) {
    // if malloc_count == MALLOC_NUM && free_count == MALLOC_NUM, return
    if (malloc_count == MALLOC_NUM && free_count == MALLOC_NUM) {
        sequence[index] = '\0';  // end the string
        // save the valid sequence to the 2D array
        sequences[*seq_index] = (char*)malloc((index + 1) * sizeof(char));
        if (sequences[*seq_index] == NULL) {
            printk("Memory allocation failed!\n");
            return;
        }
        // copy the current valid sequence to the 2D array
        strcpy(sequences[*seq_index], sequence);
        (*seq_index)++;  // update the index of the sequence
        return;
    }

    // if not enough memory is allocated, do malloc
    if (malloc_count < MALLOC_NUM) {
        sequence[index] = 'M';  // 'M' means malloc operation
        generateMallocFreeSequence(malloc_count + 1, free_count, sequence, index + 1, sequences, seq_index);
    }

    // if not enough memory is freed and the number of allocated memory is greater than the number of freed memory, do free
    if (free_count < malloc_count) {
        sequence[index] = 'F';  // 'F' means free operation
        generateMallocFreeSequence(malloc_count, free_count + 1, sequence, index + 1, sequences, seq_index);
    }
}


void test_memory_allocate()
{
    srand(time(NULL));
    int catalan_number = calculate_catalan_number(MALLOC_NUM);
    char** sequences = (char**)malloc(catalan_number * sizeof(char*));
    // to store the valid malloc and free sequences
    if (sequences == NULL) {
        printk("Memory allocation failed!\n");
        return;
    }
    // store the maximum length of each sequence
    char sequence[2*MALLOC_NUM];
    int seq_index = 0;

    // call the recursive function to generate all valid sequences and save them
    generateMallocFreeSequence(0, 0, sequence, 0, sequences, &seq_index);

    u64 total_malloc_cost = 0;
    u64 total_free_cost = 0;
    u64 total_memory_malloc_size = 0;
    u64 total_memory_allocate_size = 0;
    double total_external_fragmentation_rate = 0;

    mem_stat_hook_add(memory_hook_malloc, memory_hook_free);

    for (int i = 0; i < seq_index; i++) {
        Stack stack;
        init_stack(&stack);
        char* sequence = sequences[i];
        memset(statistics, 0, sizeof(statistics_item_t) * MALLOC_NUM);
        statistics_index = 0;
        // execute the sequence
        for(int j = 0; j < 2*MALLOC_NUM; j++){
            if(sequence[j] == 'M'){
                int size = rand() % (1024 * 256) + 256;
                memset(&mem_info, 0, sizeof(meminfo_t));
                if(mem_getinfo(&mem_info) == 0){
                    statistics[statistics_index].statistics.memory_malloc_size = size * sizeof(int);
                    statistics[statistics_index].statistics.memory_allocate_size = mem_info.heapmemused;
                    int max_free_size = mem_findmax();
                    statistics[statistics_index].statistics.external_fragmentation_rate = (double)max_free_size / mem_info.totalheapmem;
                }
                statistics[statistics_index].statistics.memory_malloc_cost = sys_timestamp();
                int* ptr = (int*)malloc(size * sizeof(int));
                if(ptr == NULL){
                    printk("Memory allocation failed!\n");
                    return;
                }
                memset(ptr, 0, size * sizeof(int));
                statistics[statistics_index].ptr = ptr;
                push(&stack, statistics_index);
                statistics_index++;
            }else{
                int index = pop(&stack);
                free_index = index;
                statistics[index].statistics.memory_free_cost = sys_timestamp();
                free(statistics[index].ptr);
                statistics[index].ptr = NULL;
            }
        }
        printf("NO.%d : sequence: %s\n", i, sequence);
        for(int j = 0; j < MALLOC_NUM; j++){
            printf("malloc %d : memory_malloc_size: %d   ", j, statistics[j].statistics.memory_malloc_size);
            printf("malloc %d : memory_allocate_size: %d   ", j, statistics[j].statistics.memory_allocate_size);
            printf("malloc %d : memory_malloc_cost: %d   ", j, statistics[j].statistics.memory_malloc_cost);
            printf("malloc %d : memory_free_cost: %d    ", j, statistics[j].statistics.memory_free_cost);
            printf("malloc %d: external_fragmentation_rate: %f %%\n", j, (1-statistics[j].statistics.external_fragmentation_rate)*100);
            printf("================================================\n");
            total_malloc_cost += statistics[j].statistics.memory_malloc_cost;
            total_free_cost += statistics[j].statistics.memory_free_cost;
            total_memory_malloc_size += statistics[j].statistics.memory_malloc_size;
            total_memory_allocate_size += statistics[j].statistics.memory_allocate_size;
            total_external_fragmentation_rate += statistics[j].statistics.external_fragmentation_rate;
        }
    }
    // printf("total_malloc_cost: %d ns\n", total_malloc_cost);
    // printf("total_free_cost: %d ns\n", total_free_cost);
    printf("average_malloc_cost: %f ns\n", ((double)total_malloc_cost / (catalan_number*MALLOC_NUM)) );
    printf("average_free_cost: %f ns\n", ((double)total_free_cost / (catalan_number*MALLOC_NUM)) );
    printf("internal_fragmentation_rate: %f %%\n", ((double)(total_memory_allocate_size - total_memory_malloc_size) / total_memory_allocate_size)*100);
    printf("external_fragmentation_rate: %f %%\n", (1 - (double)(total_external_fragmentation_rate / (catalan_number*MALLOC_NUM)))*100);
    mem_stat_hook_delete();
    printf("================================================\n");
    mem_show();
    for(int i = 0; i < seq_index; i++){
        free(sequences[i]);
    }
    free(sequences);
}