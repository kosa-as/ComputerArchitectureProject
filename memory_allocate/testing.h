#ifndef __TESTING_H__
#define __TESTING_H__
#define SYS64
#include <stdio.h>
#include <reworks/types.h>
#include <reworks/printk.h>
#include <reworksio.h>
#include <memory.h>
#include <string.h>
#include <irq.h>
#include <clock.h>
#include <udelay.h>
//#include <io_asm.h>
#include <semLib.h>
#include <vxWorks.h>
//#include <types/vxWind.h> //ajm
#include <wdLib.h>
#include <taskLib.h>
#include <msgQLib.h>
#include <event.h>
#include <semaphore.h>
#include <pthread.h>
#include <ioctl.h>

#define REWORKS_TEST
#define _PRODUCT_TEST

/* 测试平台定义 */

//#define HC_FMQL						//华创自研FMQL
//#define HC_ZYNQ                         //ZYNQ ld 2022-3-22
//#define TARGET_SBC8641D
//#define TARGET_SBC8640D
//#define TARGET_P1020
//#define TARGET_PPC4080

//#define TARGET_PENTIUM
//#define TARGET_PENTIUM64
//#define TARGET_LOOGSON3A
//#ifdef TARGET_LOOGSON3A
//#define TARGET_LOONGSON3A_R2
//#define TARGET_LOONGSON3A_R3		//3A3000 -ajm
//#endif
//#define TARGET_LOOGSON3A3000
//#define TARGET_LOONGSON2K
//#define TARGET_LOONGSON2H
//#define TARGET_LOONGSON2F
//#define TARGET_LOONGSON1B
//#define TARGET_HUARUI
//#define TARGET_SOPC80
//#define TARGET_LOOGSON3A_1000
//#define TARGET_LOOGSON2K1000
//#define TARGET_FT1500
//#define TARGET_CET32_FT1500
//#define TARGET_MPC7448
// #define TARGET_MPC8569
//#define S698PM_PLAT
//#define TARGET_ARM7_IMX6D_PLAT
//#define TARGET_ULTRASCALE
//#define TARGET_P4080
#define TARGET_FTE_2000
#ifdef HC_FMQL
#define ARMv7_FMQL_PLAT
#define __multi_core__
#define REWORKS_V6
/* 核的数量  */
#define CPU_MAX_NUM	 4
#define TEST_PLAT  "华创自研FMQL"
#define TEST_VERSION "ReDeV6.1.0-a.3+FP.32.R.64.200730"
#endif

#ifdef HC_ZYNQ                 //ld 2022-3-25
#define ARMv7_ZYNQ7K_PLAT
#define __multi_core__
#define REWORKS_V6
/* 核的数量  */
#define CPU_MAX_NUM	 2
#define TEST_PLAT  "zynq7015"
#define TEST_VERSION "ReDeV6.1.0-a.3+FP.32.R.64.200730"
#endif

#ifdef TARGET_LOOGSON3A
#define LOONGSON_3A_PLAT
#define __multi_core__
/* 核的数量 */
#define CPU_MAX_NUM	 4
#define TEST_PLAT  "龙芯3A"
//#define TEST_VERSION "ReDeV5.0RC2-LS3A-A150928"
#ifdef TARGET_LOONGSON3A_R3    //ajm
#define TEST_VERSION "ReDeV5.1.2-LS3A-a170309"     //ajm
#else                                              //ajm
#define TEST_VERSION "ReDeV5.0RC1-LS-A140624"
#endif
#endif

#ifdef TARGET_LOONGSON2K
#define LOONGSON_2K_PLAT
#define __multi_core__
/* 核的数量  */
#define CPU_MAX_NUM	 2
#define TEST_PLAT  "龙芯2K"
#define TEST_VERSION "ReDeV5.0RC1-LS-A140624"
#endif

#ifdef TARGET_LOONGSON1B
#define TEST_PLAT  "龙芯1B"
#define TEST_VERSION "ReDeV-LS1B"
#endif

#ifdef TARGET_PENTIUM
#define TEST_PLAT  "X86"
#define TEST_VERSION "ReDeV4.7.2_X86_A20140812"
#endif

#ifdef TARGET_PENTIUM64
#define PENTIUM_PLAT
#define __multi_core__
#define REWORKS_V6
#define SYS64
#define CPU_MAX_NUM	 8
#define TEST_PLAT  "X86 64位"
//#define TEST_VERSION "ReDeV4.7.2_X86_A20140812"
#define TEST_VERSION "ReDeV6.1.1+X86.64-A220526"
#endif

#ifdef REWORKS_TEST
#define TEST_SYSTEM "reworks"
#else
#define TEST_SYSTEM "vxworks"
#endif

#ifdef SYS64
//typedef long TASK_ID;
#else
typedef int TASK_ID;
#endif

typedef void (*ISRFUNC)(void *);


#define INST_NOP	0x60000000
#define INST_BLR	0x4e800020

#define TESTING_COUNT		1000

#define JITTER_ITERATOR_COMP		8000000
#define TEST_ITER		100000

#define SIZE_SMALL  (64)
#define SIZE_LARGE  (4096 + 128 )
#define NMEMB (1024)

#define user_timestamp_init sys_timestamp_init

#define TASK_STACK_SIZE 1024

extern double inter_statistical(char *,u64 *, u32 , u32 );
//#define REWORKS_V6
#endif
