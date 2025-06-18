#include "../testing.h"
#include <memory.h>
#include <memory_show.h>
/*
Memory Allocation Metric
    1. internal fragmentation rate
    2. external fragmentation rate
    3. time cost of allocation and deallocation
    4. max available memory block size
*/
#define HIGH_PRIORITY 120

int ret;

void* task(void* arg){
    struct pthread_meminfo* task_meminfo = (struct pthread_meminfo*)arg;
    if(task_meminfo == NULL){
        printk("task_meminfo is NULL\n");
        return NULL;
    }
    int size = 8000;
    char* string = (char*)malloc(size * sizeof(char));
    for(int i = 0; i < size; i++){
        string[i] = 'a';
    }
    printk("memory write done\n");
    pthread_t self = pthread_self();
    printk("[Thread 0x%p] memory write done\n", (void*)self);
    printk("before wirte: sigma_alloc_size: %u, alloc_cnt: %u, sigma_free_size: %u, free_cnt: %u\n", task_meminfo->sigma_alloc_size, task_meminfo->alloc_cnt, task_meminfo->sigma_free_size, task_meminfo->free_cnt);
    ret = pthread_get_meminfo(self, task_meminfo);
    printk("after write: sigma_alloc_size: %u, alloc_cnt: %u, sigma_free_size: %u, free_cnt: %u\n", task_meminfo->sigma_alloc_size, task_meminfo->alloc_cnt, task_meminfo->sigma_free_size, task_meminfo->free_cnt);
    if(ret != 0){
        printk("pthread_get_meminfo failed\n");
        return NULL;
    }
    free(string);
    return NULL;
}

void test_pthread_get_meminfo()
{
// ret = pthread_get_meminfo(pthread_self(), task_meminfo);
// struct pthread_meminfo
// {
//        u32 sigma_alloc_size; 
//        u32 alloc_cnt; 
//        u32 sigma_free_size; 
//        u32 free_cnt; 
// };
    pthread_t task_thread;
    struct pthread_meminfo task_meminfo;
    memset(&task_meminfo, 0, sizeof(struct pthread_meminfo));
    printk("init memory info done\n");
    ret = pthread_create2(&task_thread, "task_thread", HIGH_PRIORITY, 0, 32 << 10, task, &task_meminfo);
    if(ret != 0){
        printk("pthread_create2 failed\n");
        return;
    }
    pthread_join(task_thread, NULL);
    printk("task memory info:\n");
    printk("sigma_alloc_size: %u, alloc_cnt: %u, sigma_free_size: %u, free_cnt: %u\n", task_meminfo.sigma_alloc_size, task_meminfo.alloc_cnt, task_meminfo.sigma_free_size, task_meminfo.free_cnt);
    printk("final memory status:\n");
    mem_show();
}