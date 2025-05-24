#include "../basic/testing.h"
#include <stdint.h>
#include <stdio.h>
#include <time.h>

//#define DWT_CTRL (*(volatile uint32_t*)0xE0001000)
//#define DWT_CYCCNT (*(volatile uint32_t*)0xE0001004)

uint32_t test_once() {
	struct timespec start, end;
	
	clock_gettime(CLOCK_REALTIME, &start);
	
	// 执行任务
	uint8_t buffer[1024];
	for (int i=0; i<10000; i++) {
	    for (int j=0; j<1024; j++) {
	        buffer[j] = i & 0xFF; // 写操作
	    }
	}
	
	clock_gettime(CLOCK_REALTIME, &end);
	long duration = (end.tv_sec - start.tv_sec) * 1e9 + (end.tv_nsec - start.tv_nsec); //精确到纳秒
	printf("Duration: %ld ns\n", duration);
	return 0;
}

void test_task_running_time() {
	for(int i = 0;i < 100;i++)
	{
		test_once();
	}
}
