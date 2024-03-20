/**
 * @file autocfg.h
 * @author 王彬浩 (SPGGOGOGO@outlook.com)
 * @brief aCoral一些配置头文件，属于历史遗留，以后还要删改
 * @version 1.1
 * @date 2023-04-19
 * @copyright Copyright (c) 2023
 * @revisionHistory 
 *  <table> 
 *   <tr><th> 版本 <th>作者 <th>日期 <th>修改内容 
 *   <tr><td> 0.1 <td>jivin <td>2010-03-08 <td>Created 
 *   <tr><td> 1.0 <td>王彬浩 <td> 2022-07-08 <td>Standardized 
 *   <tr><td> 1.1 <td>王彬浩 <td> 2023-04-19 <td>
 *  </table>
 */
#ifndef AUTOCFG_H
#define AUTOCFG_H

/*
 * SOC select
 */
#define CFG_K210 1
#undef CFG_S3C2440

#ifdef CFG_K210
#define CFG_SMP 
#endif

/*
 * kernel configuration
 */
#define CFG_MEM2 1 ///<任意大小内存分配系统是否启用
#define CFG_MEM2_SIZE (102400) ///<任意大小内存分配系统的大小，是从伙伴系统管理的堆内存中拿出一部分

#define CFG_THRD_PERIOD 1

#define CFG_THRD_DAG 1 ///<启用DAG调度
#define CFG_DAG_SIZE 10 ///<全局DAG图节点数量上限


#define CFG_HARD_RT_PRIO_NUM (0) ///<硬实时任务的专属优先级个数

#define CFG_MAX_THREAD (40) ///<///最多40个线程

#define CFG_MIN_STACK_SIZE (10240) ///<线程最小拥有10240字节的栈

#define CFG_EVT_SEM 1

#define CFG_MSG 1

#define CFG_TICKS_PER_SEC (100) ///<acoral每秒的ticks数

/*
 * User configuration
 */
#define CFG_SHELL 1 ///<启用shell

/*
 * System hacking
 */
#define CFG_BAUD_RATE (115200)
#define CFG_DEBUG_INFO 1

#undef ARCH_ACORAL_FPU /*//SPG中断保护现场那里*/

#undef EN_ASSERT //开启断言需要define
#ifdef EN_ASSERT
#define Assert(cond, format, ...) \
  do { \
    if (!(cond)) { \
      fprintf(stderr, format "\n" , ## __VA_ARGS__); \
      assert(cond); \
    } \
  } while (0)

#else
#define Assert(cond, format, ...)
#endif

#endif