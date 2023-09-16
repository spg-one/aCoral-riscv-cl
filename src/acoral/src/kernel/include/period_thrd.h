/**
 * @file period_thrd.h
 * @author 王彬浩 (SPGGOGOGO@outlook.com)
 * @brief kernel层，周期策略头文件
 * @version 1.0
 * @date 2023-04-20
 * @copyright Copyright (c) 2023
 * @revisionHistory 
 *  <table> 
 *   <tr><th> 版本 <th>作者 <th>日期 <th>修改内容 
 *   <tr><td> 0.1 <td>jivin <td>2010-03-08 <td>Created 
 *   <tr><td> 1.0 <td>王彬浩 <td> 2022-07-08 <td>Standardized 
 *   <tr><td> 1.1 <td>王彬浩 <td> 2023-04-20 <td>optimized 
 *  </table>
 */
#ifndef PERIOD_THRD_H
#define PERIOD_THRD_H

#include "thread.h"

/**
 * @brief 周期策略数据块
 * 
 */
typedef struct{
	unsigned char prio; 		///<线程优先级
	acoralPrioTypeEnum prio_type; 	///<线程优先级类型，包括硬实时任务ACORAL_HARD_PRIO、非硬实时任务ACORAL_NONHARD_PRIO
	unsigned int time; 			///<线程周期，单位为毫秒
}acoral_period_policy_data_t;

/**
 * @brief 周期线程私有数据块，用于保存自己的周期、函数体，以便在新周期重新挂载线程时用
 * 
 */
typedef struct{
	unsigned int time; 			///<线程周期，单位为毫秒
	void (*route)(void *args); 	///<线程函数
	void *args; 				///<线程函数的参数
}period_private_data_t;

void period_thread_exit(void);
void period_thread_delay(acoral_thread_t* thread,unsigned int time);
void period_delay_deal(void);
int period_policy_thread_init(acoral_thread_t *thread,void (*route)(void *args),void *args,void *data);
void period_policy_init(void);
#endif
