#include "../basic/testing.h"
#include <irq.h>
#include <clock.h>
#include <clock_arch.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdlib.h>
#include <time.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>

#define NUM_TASKS 10
#define NUM_ROUNDS 1000
#define STACK_SIZE (32 << 10)

// Priority range
#define HIGH_PRIORITY_BASE 120
#define MID_PRIORITY_BASE 100
#define LOW_PRIORITY_BASE 80

extern u32 get_cntfrq();

static sem_t *task_semid;

// Statistics
typedef struct {
    int total_inversions;
    u64 total_delay_ns;
    u64 max_delay_ns;
    int severe_inversions;  // Severe priority inversions (>4ms)
    // Preemption response time statistics
    int total_preemptions;     // Total preemptions
    u64 total_preempt_ns;      // Total preemption response time
    u64 max_preempt_ns;        // Maximum preemption response time
    u64 min_preempt_ns;        // Minimum preemption response time
    int slow_preemptions;      // Slow preemptions (>1ms)
} statistics_t;

statistics_t stats = {0};

typedef struct {
    u64 ready_time;  // Task ready time
} task_args_t;

// Competing tasks: acquire lock, then execute
void* mutex_task(void* arg) {
    task_args_t* args = (task_args_t*)arg;
    u64 exec_start = sys_timestamp();  // Task actual execution start time
    u32 freq = get_cntfrq();
    if(args != NULL) {
        // Calculate preemption response time
        u64 preempt_response_ns = (exec_start - args->ready_time) * 1000000000 / freq;
        
        // Update preemption response time statistics
        __sync_fetch_and_add(&stats.total_preemptions, 1);
        __sync_add_and_fetch(&stats.total_preempt_ns, preempt_response_ns);
        
        // Update maximum preemption response time
        u64 current_max;
        do {
            current_max = stats.max_preempt_ns;
            if (preempt_response_ns <= current_max) break;
        } while (!__sync_bool_compare_and_swap(&stats.max_preempt_ns, current_max, preempt_response_ns));
        
        // Update minimum preemption response time
        u64 current_min;
        do {
            current_min = stats.min_preempt_ns;
            if (current_min != 0 && preempt_response_ns >= current_min) break;
        } while (!__sync_bool_compare_and_swap(&stats.min_preempt_ns, current_min, preempt_response_ns));
        
        // Record slow preemption
        if (preempt_response_ns > 1000000) { // Preemption response over 1ms
            __sync_fetch_and_add(&stats.slow_preemptions, 1);
            // printk("[Warning] Slow preemption response! Response time: %.3f ms\n", (double)preempt_response_ns / 1000000.0);
        }
    }
    u64 lock_start;
    if(args != NULL){
        lock_start = sys_timestamp();
    }
    // Acquire lock
    int ret = sem_wait(task_semid);
    if (ret != 0) {
        sem_close(task_semid);
        printk("[Error] Low priority task cannot acquire mutex: %s\n", strerror(ret));
        return (void*)(intptr_t)ret;
    }
    if(args != NULL){
        u64 lock_end = sys_timestamp();
        // freq = get_cntfrq();
        u64 delay_ns = (lock_end - lock_start) * 1000000000 / freq;
        if (delay_ns > 1500) {// Over 1.5ms indicates blocking
            __sync_fetch_and_add(&stats.total_inversions, 1);
            __sync_add_and_fetch(&stats.total_delay_ns, delay_ns);
            
            // Update maximum delay time
            u64 current_max;
            do {
                current_max = stats.max_delay_ns;
                if (delay_ns <= current_max) break;
            } while (!__sync_bool_compare_and_swap(&stats.max_delay_ns, current_max, delay_ns));
            
            // Record severe inversion
            if (delay_ns > 4000) { // Inversion over 4ms
                __sync_fetch_and_add(&stats.severe_inversions, 1);
                printk("[Warning] Severe priority inversion! Blocking time: %lu ns\n", delay_ns);
            }
        }
    }

    udelay(1000);
    
    ret = sem_post(task_semid);
    if (ret != 0) {
        sem_close(task_semid);
        printk("[Error] Low priority task cannot release mutex: %s\n", strerror(ret));
        return (void*)(intptr_t)ret;
    }
    return NULL;
}


// Non-competing tasks: queue jumping execution
void* no_mutex_task(void* arg) {
    // Simulate random workload
    udelay(1000);
    return NULL;
}


int run_one_round() {
    pthread_t high_priority_mutex_tid[2];
    pthread_t low_priority_mutex_tid[2];
    pthread_t no_mutex_tid[6];
    task_args_t high_args[2];
    int ret;    
    for(int i = 0; i < 2; i++){
        high_args[i].ready_time = sys_timestamp();
        ret = pthread_create2(&high_priority_mutex_tid[i], "high_priority_mutex_tid", HIGH_PRIORITY_BASE, 0, STACK_SIZE, mutex_task, &high_args[i]);
        pthread_suspend(high_priority_mutex_tid[i]);
        if(ret != 0){
            sem_close(task_semid);
            printk("[Error] Failed to create high priority mutex task: %s\n", strerror(ret));
            return ret;
        }
    }
    for(int i = 0; i < 2; i++){
        ret = pthread_create2(&low_priority_mutex_tid[i], "low_priority_mutex_tid", LOW_PRIORITY_BASE, 0, STACK_SIZE, mutex_task, NULL);
        pthread_suspend(low_priority_mutex_tid[i]);
        if(ret != 0){
            sem_close(task_semid);
            printk("[Error] Failed to create low priority mutex task: %s\n", strerror(ret));
            return ret;
        }
    }
    for(int i = 0; i < 6; i++){
        ret = pthread_create2(&no_mutex_tid[i], "no_mutex_tid", MID_PRIORITY_BASE, 0, STACK_SIZE, no_mutex_task, NULL);
        pthread_suspend(no_mutex_tid[i]);
        if(ret != 0){
            sem_close(task_semid);
            printk("[Error] Failed to create non-mutex task: %s\n", strerror(ret));
            return ret;
        }
    }

    // ensure each task is resumed once
    int task_order[NUM_TASKS];
    for(int i = 0; i < NUM_TASKS; i++){
        task_order[i] = i;
    }
    
    // Shuffle task order
    for(int i = NUM_TASKS - 1; i > 0; i--){
        int j = rand() % (i + 1);
        int temp = task_order[i];
        task_order[i] = task_order[j];
        task_order[j] = temp;
    }
    
    // Resume tasks in shuffled order
    for(int i = 0; i < NUM_TASKS; i++){
        int task_idx = task_order[i];
        if(task_idx < 2){
            high_args[task_idx].ready_time = sys_timestamp();
            pthread_resume(high_priority_mutex_tid[task_idx]);
        }else if(task_idx < 4){
            pthread_resume(low_priority_mutex_tid[task_idx - 2]);
        }else{
            pthread_resume(no_mutex_tid[task_idx - 4]);
        }
        usleep(300);
    }

    for(int i = 0; i < 2; i++){
        pthread_join(high_priority_mutex_tid[i], NULL);
    }
    for(int i = 0; i < 2; i++){
        pthread_join(low_priority_mutex_tid[i], NULL);
    }
    for(int i = 0; i < 6; i++){
        pthread_join(no_mutex_tid[i], NULL);
    }
    return 0;
}

void test_task_preempt() {
    srand(time(NULL));
    // Reset statistics
    memset(&stats, 0, sizeof(stats));
    stats.min_preempt_ns = UINT64_MAX;  // Initialize minimum value
    
    int success_rounds = 0;
    for (int i = 0; i < NUM_ROUNDS; i++) {
        
        task_semid = sem_open("task_semid",O_CREAT, 0777, rand() % 3 + 1, SEM_COUNTING, PTHREAD_WAITQ_PRIO);
        if (task_semid == SEM_FAILED)
        {
            printk("Failed to create semaphore\n");
            return NULL;
        }
        if (i % 100 == 0) {
            printk("Running test round %d/%d...\n", i, NUM_ROUNDS);
        }
        
        if (run_one_round() == 0) {
            success_rounds++;
        }
        sem_close(task_semid);
    }
    
    // Output detailed statistics
    printk("=========== Priority Inversion and Preemption Response Test Report ===========\n");
    printk("Test completion: %d/%d rounds successful\n", success_rounds, NUM_ROUNDS);
    
    printk("\n--- Priority Inversion Statistics ---\n");
    printk("Total priority inversions: %d\n", stats.total_inversions);
    if (stats.total_inversions > 0) {
        printk("Severe inversions (>4ms): %d\n", stats.severe_inversions);
        printk("Average blocking time: %10d ns\n", (int)(stats.total_delay_ns / stats.total_inversions / 1000.0));
        printk("Maximum blocking time: %10d ns\n", (int)(stats.max_delay_ns / 1000.0));
        printk("Priority inversion rate: %10d%%\n", (int)(stats.total_inversions / success_rounds * 100.0));
    }
    
    printk("\n--- Preemption Response Time Statistics ---\n");
    printk("Total preemptions: %d\n", stats.total_preemptions);
    if (stats.total_preemptions > 0) {
        printk("Slow preemptions (>1ms): %d\n", stats.slow_preemptions);
        printk("Average preemption response time: %10d ns\n", (int)(stats.total_preempt_ns / stats.total_preemptions / 1000.0));
        printk("Maximum preemption response time: %10d ns\n", (int)(stats.max_preempt_ns / 1000.0));
        printk("Minimum preemption response time: %10d ns\n", (int)(stats.min_preempt_ns / 1000.0));
        printk("Slow preemption rate: %d%%\n", (int)(stats.slow_preemptions / stats.total_preemptions * 100.0));
    }
    printk("================================================\n");
    sem_unlink("task_semid");
}