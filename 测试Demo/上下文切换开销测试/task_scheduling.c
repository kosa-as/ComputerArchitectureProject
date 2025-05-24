#include "../basic/testing.h"
#include <irq.h>
#include <clock.h>
#include <clock_arch.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <limits.h>
#include <stdint.h>

#define TASK_TEST_COUNT 100000
#define HIGH_PRIORITY 120
#define LOW_PRIORITY 100

extern u32 sys_clk_rate_get(void);
extern double inter_statistical(char *testname, u64 *array, u32 num, u32 flag);
extern int cpu_id_get(void);
extern u64 get_cntp_ct();
extern u64 get_cntp_cval();
extern void set_cntp_cval(u64);
extern u32 get_cntp_tval();
extern void set_cntp_tval(u32);
extern u32 get_cntp_ctl();
extern void set_cntp_ctl(u32);
extern void gic_reg_test();
extern u32 get_cntfrq();
extern void aarch64_sysclk_ack(void *arg);
extern void aarch64_sysclk_reset(void);
extern u64 cpu_cycles_per_tick;
extern void clock_tick_per_cpu(int);

// 全局变量用于任务调度时间测试
static sem_t *task_semid;
static u64 task_time_diff[TASK_TEST_COUNT];
static u64 task_max_time = 0;
static u64 task_min_time = ULLONG_MAX;
static double task_average_time = 0;
static u64 task_sum_time = 0;
static int task_test_count = 0;

// 时间记录变量
static u64 task_time1, task_time2;
// 获取频率
u32 freq;

// 任务A：高优先级任务，等待信号量并记录时间
void* taskA_func(void* arg)
{
    while (task_test_count < TASK_TEST_COUNT)
    {
        sem_wait(task_semid); // 等待信号量
        freq = get_cntfrq();

        // 记录获取信号量的时间
        task_time2 = sys_timestamp();
//        printk("task_test_count is %d task_time2 is %10d\n",task_test_count,(int)task_time2);
                
        task_time_diff[task_test_count] = task_time2 - task_time1;
        printk("test count: %d, task_scheduling_time: %d\n", task_test_count, (int)(task_time_diff[task_test_count] * 1000000000.0 / freq));
//        printk("task_time_diff[%d] is %10d\n",task_test_count,(int)(task_time_diff[task_test_count]* 1000000.0 / freq));
        

        // 更新统计数据
        task_sum_time += task_time_diff[task_test_count];
        if (task_time_diff[task_test_count] > task_max_time)
        {
            task_max_time = task_time_diff[task_test_count];
        }
        if (task_time_diff[task_test_count] < task_min_time)
        {
            task_min_time = task_time_diff[task_test_count];
        }

        task_test_count++;
    }
    return NULL;
}

// 任务B：低优先级任务，释放信号量并记录时间
void* taskB_func(void* arg)
{
    while (task_test_count < TASK_TEST_COUNT)
    {
        // 记录释放信号量的时间
        task_time1 = sys_timestamp();
        sem_post(task_semid); // 释放信号量

    }
    return NULL;
}

// 任务调度时间测试内部函数
void test_task_scheduling()
{
    pthread_t taskA, taskB;
    int ret;

    // 初始化信号量，初始值为0
    task_semid = sem_open("task_semid",O_CREAT, 0777, 0, SEM_BINARY, PTHREAD_WAITQ_PRIO);
    if (task_semid == SEM_FAILED)
    {
        printk("Failed to create semaphore\n");
        return NULL;
    }

    // 创建高优先级任务A
    ret = pthread_create2(&taskA, "taskA", HIGH_PRIORITY, 0, 32 << 10, 
                         taskA_func, NULL);
    if (ret != 0)
    {
        printk("Creating Task A failed\n");
        sem_close(task_semid);
        return NULL;
    }

    // 创建低优先级任务B
    ret = pthread_create2(&taskB, "taskB", LOW_PRIORITY, 0, 32 << 10, 
                         taskB_func, NULL);
    if (ret != 0)
    {
        printk("Creating Task B failed\n");
        pthread_cancel(taskA);
        sem_close(task_semid);
        return NULL;
    }

    // 等待任务A和任务B完成
    pthread_join(taskA, NULL);
    pthread_join(taskB, NULL);

    // 计算平均值
    task_average_time = (double)task_sum_time / TASK_TEST_COUNT;

    freq = get_cntfrq();

    // 输出统计结果，转换为微秒
    printk("task scheduling time!\n");
    printk("| Min:%10d ns | Max:%10d ns | Average:%10d ns |\n",
           (int)(task_min_time * 1000000000.0 / freq),
           (int)(task_max_time * 1000000000.0 / freq),
           (int)(task_average_time * 1000000000.0 / freq));

    sem_close(task_semid);
    sem_unlink("task_semid");
}

