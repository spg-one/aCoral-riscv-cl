/**
 * @file timer.h
 * @author 王彬浩 (SPGGOGOGO@outlook.com)
 * @brief kernel层，定时器相关头文件
 * @version 1.0
 * @date 2022-07-20
 * @copyright Copyright (c) 2023
 * @revisionHistory 
 *  <table> 
 *   <tr><th> 版本 <th>作者 <th>日期 <th>修改内容 
 *   <tr><td> 0.1 <td>jivin <td>2010-03-08 <td>Created 
 *   <tr><td> 1.0 <td>王彬浩 <td> 2022-07-20 <td>Standardized 
 *  </table>
 */

#ifndef ACORAL_TIMER_H
#define ACORAL_TIMER_H

#include "autocfg.h"
#include "core.h"
#include "thread.h"

int time_to_ticks(unsigned int time); 
extern acoral_list_t time_delay_queue;
extern acoral_list_t timeout_queue;

void acoral_time_sys_init();
void acoral_time_init(void);
int acoral_ticks_init(void);
void acoral_ticks_entry(int vector);
void time_delay_deal(void);

/**
 * @brief 将线程挂到延时队列上
 * 
 */
void acoral_delayqueue_add(acoral_list_t*, acoral_thread_t*);

/**
 * @brief 超时链表处理函数
 * 
 */
void timeout_delay_deal(void);

/**
 * @brief 将线程挂到超时队列上
 * 
 */
void timeout_queue_add(acoral_thread_t*);

/**
 * @brief 将线程从超时队列删除
 * 
 */
void timeout_queue_del(acoral_thread_t*);

/***************ticks相关API****************/

/**
 * @brief 设置aCoral心跳tick的值
 * 
 * @param time tick新值
 */
void acoral_set_ticks(unsigned int time);

/**
 * @brief 得到tick的值
 * 
 * @return tick的值
 */
unsigned int acoral_get_ticks();

#endif

