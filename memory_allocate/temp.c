#include "../testing.h"
#include <memory.h>
#include <memory_show.h>

/*
一些任务负责执行，一个任务负责一直记录
内部碎片：每次malloc的时候，调用钩子函数记录申请的大小和实际分配的大小（分配的时候，钩子函数还要去计算时间）
外部碎片：由负责记录的任务定时调用记录内存的情况（包括最大的可用块大小）

*/

typedef struct{
    u32 memory_malloc_size;
    u32 memory_allocate_size;
    u32 memory_malloc_cost;
    u32 memory_free_cost;
}

meminfo_t* mem_info;

void test_memory_allocate()
{

}