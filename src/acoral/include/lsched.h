/**
 * @file lsched.h
 * @author 王彬浩 (SPGGOGOGO@outlook.com)
 * @brief kernel层，aCoral调度机制头文件
 * @note 之所以要叫lsched.h，而不叫sched.h，是因为linux有一个头文件叫sched.h，怕重名会有问题
 * @version 1.1
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
#ifndef SCHED_H
#define SCHED_H

#include "thread.h"

extern unsigned char acoral_need_sched; 
extern unsigned char sched_lock;
extern acoral_thread_t *acoral_cur_thread,*acoral_ready_thread;

///就绪队列中的优先级位图的大小，目前等于2，算法就是优先级数目除以32向上取整
#define PRIO_BITMAP_SIZE ((ACORAL_MAX_PRIO_NUM+31)/32) 
#define acoral_set_ready_thread(thread) (acoral_ready_thread=(thread))

/**
 * @brief aCoral就绪队列
 * 
 */
typedef struct{
	unsigned int num;							///<就绪的线程数
	unsigned int bitmap[PRIO_BITMAP_SIZE];		///<优先级位图，每一位对应一个优先级，为1表示这个优先级有就绪线程
	acoral_list_t queue[ACORAL_MAX_PRIO_NUM];	///<每一个优先级都有独立的队列
}acoral_rdy_queue_t;

void acoral_set_running_thread(acoral_thread_t *thread);
void acoral_thread_runqueue_init(void);

/**
 * @brief 将某个线程挂载到就绪队列上
 * 
 * @param new 将被挂载的线程
 */
void acoral_rdyqueue_add(acoral_thread_t *new);

/**
 * @brief 从就绪队列上删除一个线程
 * 
 * @param old 要删除的线程
 */
void acoral_rdyqueue_del(acoral_thread_t *old);


/**
 * @brief 从就绪队列中选出优先级最高的线程，设为acoral_ready_thread
 * 
 */
void acoral_select_thread(void);
void acoral_sched(void);
void acoral_real_sched();
void acoral_real_intr_sched();
#endif
