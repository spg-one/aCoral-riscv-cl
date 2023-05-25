/**
 * @file sem.h
 * @author 王彬浩 (SPGGOGOGO@outlook.com)
 * @brief kernel层，内核信号量相关函数
 * @version 1.0
 * @date 2023-04-21
 * @copyright Copyright (c) 2023
 * @revisionHistory
 *  <table>
 *   <tr><th> 版本 <th>作者 <th>日期 <th>修改内容
 *   <tr><td> 0.1 <td>jivin <td>2010-03-08 <td>Created
 *   <tr><td> 1.0 <td>王彬浩 <td> 2023-04-21 <td>Standardized
 *  </table>
 */

#include "event.h"
#include "thread.h"
#include "lsched.h"
#include "hal.h"
#include "int.h"
#include "timer.h"
#include "sem.h"
#include <stdio.h>

extern void acoral_evt_queue_del(acoral_thread_t *thread);
extern void acoral_evt_queue_add(acoral_evt_t *evt, acoral_thread_t *new);
acoral_thread_t *acoral_evt_high_thread(acoral_evt_t *evt);
acoralSemRetValEnum acoral_sem_init(acoral_evt_t *evt, unsigned int semNum)
{
	if (NULL == evt)
	{
		return SEM_ERR_NULL;
	}
	semNum = 1 - semNum; /* 拥有多个资源，0,一个  -1 两个， -2 三个 ....*/
	evt->count = semNum;
	evt->type = ACORAL_EVENT_SEM;
	evt->data = NULL;
	acoral_evt_init(evt);
	return SEM_SUCCED;
}

acoral_evt_t *acoral_sem_create(unsigned int semNum)
{
	acoral_evt_t *evt;
	evt = acoral_alloc_evt();
	if (NULL == evt)
	{
		return NULL;
	}
	semNum = 1 - semNum; /* 拥有多个资源，0,一个  -1 两个， -2 三个 ....*/
	evt->count = semNum;
	evt->type = ACORAL_EVENT_SEM;
	evt->data = NULL;
	acoral_evt_init(evt);
	return evt;
}

acoralSemRetValEnum acoral_sem_del(acoral_evt_t *evt)
{
	acoral_thread_t *thread;
	if (acoral_intr_nesting)
	{
		return SEM_ERR_INTR;
	}
	/* 参数检测*/
	if (NULL == evt)
		return SEM_ERR_NULL; /* error*/
	if (evt->type != ACORAL_EVENT_SEM)
		return SEM_ERR_TYPE; /* error*/

	acoral_enter_critical();
	thread = acoral_evt_high_thread(evt);
	if (thread == NULL)
	{
		/*队列上无等待任务*/
		acoral_exit_critical();
		evt = NULL;
		return SEM_ERR_UNDEF;
	}
	else
	{
		/*有等待任务*/
		acoral_exit_critical();
		return SEM_ERR_TASK_EXIST; /*error*/
	}
}

acoralSemRetValEnum acoral_sem_trypend(acoral_evt_t *evt)
{
	if (acoral_intr_nesting)
	{
		return SEM_ERR_INTR;
	}

	/* 参数检测 */
	if (NULL == evt)
	{
		return SEM_ERR_NULL; /*error*/
	}
	if (ACORAL_EVENT_SEM != evt->type)
	{
		return SEM_ERR_TYPE; /*error*/
	}

	/* 计算信号量处理*/
	acoral_enter_critical();
	if ((char)evt->count <= SEM_RES_AVAI)
	{ /* available*/
		evt->count++;
		acoral_exit_critical();
		return SEM_SUCCED;
	}
	acoral_exit_critical();
	return SEM_ERR_TIMEOUT;
}

acoralSemRetValEnum acoral_sem_pend(acoral_evt_t *evt, unsigned int timeout)
{
	acoral_thread_t *cur = acoral_cur_thread;

	if (acoral_intr_nesting)
	{
		return SEM_ERR_INTR;
	}

	/* 参数检测 */
	if (NULL == evt)
	{
		return SEM_ERR_NULL; /*error*/
	}
	if (ACORAL_EVENT_SEM != evt->type)
	{
		return SEM_ERR_TYPE; /*error*/
	}

	/* 计算信号量处理*/
	acoral_enter_critical();
	if ((char)evt->count <= SEM_RES_AVAI)
	{ /* available*/
		evt->count++;
		acoral_exit_critical();
		return SEM_SUCCED;
	}

	evt->count++;
	acoral_unrdy_thread(cur);
	if (timeout > 0)
	{
		cur->delay = time_to_ticks(timeout);
		timeout_queue_add(cur);
	}
	acoral_evt_queue_add(evt, cur);
	acoral_exit_critical();

	acoral_sched();

	acoral_enter_critical();
	if (timeout > 0 && cur->delay <= 0)
	{
		//--------------
		// modify by pegasus 0804: count-- [+]
		evt->count--;
		acoral_evt_queue_del(cur);
		acoral_exit_critical();
		return SEM_ERR_TIMEOUT;
	}

	//-------------------
	// modify by pegasus 0804: timeout_queue_del [+]
	timeout_queue_del(cur);
	acoral_exit_critical();
	return SEM_SUCCED;
}

acoralSemRetValEnum acoral_sem_post(acoral_evt_t *evt)
{
	acoral_thread_t *thread;

	/* 参数检测*/
	if (NULL == evt)
	{
		return SEM_ERR_NULL; /* error*/
	}
	if (ACORAL_EVENT_SEM != evt->type)
	{
		return SEM_ERR_TYPE;
	}

	acoral_enter_critical();

	/* 计算信号量的释放*/
	if ((char)evt->count <= SEM_RES_NOAVAI)
	{ /* no waiting thread*/
		evt->count--;
		acoral_exit_critical();
		return SEM_SUCCED;
	}
	/* 有等待线程*/
	evt->count--;
	thread = acoral_evt_high_thread(evt);
	if (thread == NULL)
	{
		/*应该有等待线程却没有找到*/
		printf("Err Sem post\n");
		acoral_exit_critical();
		return SEM_ERR_UNDEF;
	}
	timeout_queue_del(thread);
	/*释放等待任务*/
	acoral_evt_queue_del(thread);
	acoral_rdy_thread(thread);
	acoral_exit_critical();
	acoral_sched();
	return SEM_SUCCED;
}

int acoral_sem_getnum(acoral_evt_t *evt)
{
	int t;
	if (NULL == evt)
		return SEM_ERR_NULL;

	acoral_enter_critical();
	t = 1 - (int)evt->count;
	acoral_exit_critical();
	return t;
}
