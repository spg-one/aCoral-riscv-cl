/**
 * @file mutex.c
 * @author 王彬浩 (SPGGOGOGO@outlook.com)
 * @brief kernel层，互斥量机制
 * @version 1.0
 * @date 2023-05-08
 * @copyright Copyright (c) 2023
 * @revisionHistory
 *  <table>
 *   <tr><th> 版本 <th>作者 <th>日期 <th>修改内容
 *   <tr><td> 0.1 <td>jivin <td> 2010-03-08 <td>Created
 *   <tr><td> 1.0 <td>王彬浩 <td> 2023-05-08 <td>Standardized
 *  </table>
 */

#include "event.h"
#include "hal.h"
#include "thread.h"
#include "lsched.h"
#include "int.h"
#include "timer.h"
#include "mutex.h"
#include <stdio.h>

extern void acoral_evt_queue_del(acoral_thread_t *thread);
extern void acoral_evt_queue_add(acoral_evt_t *evt, acoral_thread_t *new);

acoral_thread_t *acoral_evt_high_thread(acoral_evt_t *evt);

acoralMutexRetVal acoral_mutex_init(acoral_evt_t *evt, unsigned char prio)
{
	if ((acoral_evt_t *)0 == evt)
	{
		return MUTEX_ERR_NULL;
	}
	evt->count = (prio << 16) | MUTEX_AVAI;
	evt->type = ACORAL_EVENT_MUTEX;
	evt->data = NULL;
	acoral_evt_init(evt);
	return MUTEX_SUCCED;
}

acoral_evt_t *acoral_mutex_create(unsigned char prio, unsigned int *err) // SPG这个和上面那个保留一个
{
	acoral_evt_t *evt;

	/* 是否大于最小优先线，和是否该优先级已占用*/
	evt = acoral_alloc_evt(); /* alloc a RAM for the event*/
	if (NULL == evt)
	{
		*err = MUTEX_ERR_NULL;
		return NULL;
	}
	evt->count = (prio << 16) | MUTEX_AVAI | MUTEX_U_MASK;
	evt->type = ACORAL_EVENT_MUTEX;
	evt->data = NULL;
	acoral_evt_init(evt);
	return evt;
}

acoralMutexRetVal acoral_mutex_del(acoral_evt_t *evt, unsigned int opt)
{
	/* 参数检测 */
	if (NULL == evt)
	{
		return MUTEX_ERR_NULL; /*error*/
	}

	if (ACORAL_EVENT_MUTEX != evt->type)
	{
		return MUTEX_ERR_TYPE; /*error*/
	}

	/* 是否有任务等待*/
	acoral_enter_critical();
	if (acoral_evt_queue_empty(evt))
	{
		/*无等待任务删除*/
		acoral_exit_critical();
		return MUTEX_SUCCED;
	}
	else
	{
		/*有等待任务*/
		acoral_exit_critical();
		return MUTEX_ERR_TASK_EXIST;
	}
}

acoralMutexRetVal acoral_mutex_trypend(acoral_evt_t *evt)
{
	acoral_thread_t *cur;

	if (acoral_intr_nesting > 0)
		return MUTEX_ERR_INTR;

	cur = acoral_cur_thread;

	acoral_enter_critical();
	if (NULL == evt)
	{
		acoral_exit_critical();
		return MUTEX_ERR_NULL;
	}

	if ((unsigned char)(evt->count & MUTEX_L_MASK) == MUTEX_AVAI)
	{
		/* 申请成功*/
		evt->count &= MUTEX_U_MASK;
		evt->count |= cur->prio;
		evt->data = (void *)cur;
		acoral_exit_critical();
		return MUTEX_SUCCED;
	}

	acoral_exit_critical();
	return MUTEX_ERR_TIMEOUT;
}

acoralMutexRetVal acoral_mutex_pend(acoral_evt_t *evt, unsigned int timeout)
{
	unsigned char highPrio;
	acoral_thread_t *thread;
	acoral_thread_t *cur;

	if (acoral_intr_nesting > 0)
		return MUTEX_ERR_INTR;

	cur = acoral_cur_thread;

	acoral_enter_critical();
	if (NULL == evt)
	{
		acoral_exit_critical();
		return MUTEX_ERR_NULL;
	}

	if ((unsigned char)(evt->count & MUTEX_L_MASK) == MUTEX_AVAI)
	{
		/* 申请成功*/
		evt->count &= MUTEX_U_MASK;
		evt->count |= cur->prio;
		evt->data = (void *)cur;
		acoral_exit_critical();
		return MUTEX_SUCCED;
	}

	/* 互斥量已被占有*/
	highPrio = (unsigned char)(evt->count >> 8);
	thread = (acoral_thread_t *)evt->data;

	/*有可能优先级反转，继承最高优先级*/
	if (thread->prio > cur->prio)
	{
		if (cur->prio < highPrio)
		{
			highPrio = cur->prio;
			evt->count &= ~MUTEX_U_MASK;
			evt->count |= highPrio << 8;
		}
		acoral_thread_change_prio(thread, highPrio);
	}
	/*不需要或不能提高优先级*/
	acoral_unrdy_thread(cur);
	acoral_evt_queue_add(evt, cur);
	if (timeout > 0)
	{
		/*加载到超时队列*/
		cur->delay = time_to_ticks(timeout);
		timeout_queue_add(cur);
	}
	acoral_exit_critical();
	acoral_sched();
	acoral_enter_critical();
	if (evt->data != cur && timeout > 0 && cur->delay <= 0)
	{
		printf("Time Out Return\n");
		acoral_evt_queue_del(cur);
		acoral_exit_critical();
		return MUTEX_ERR_TIMEOUT;
	}

	//---------------
	// modify by pegasus 0804: timeout_queue_del [+]
	timeout_queue_del(cur);

	if (evt->data != cur)
	{
		printf("Err Ready Return\n");
		acoral_evt_queue_del(cur);
		acoral_exit_critical();
		return MUTEX_ERR_RDY;
	}

	return MUTEX_SUCCED;
}

acoralMutexRetVal acoral_mutex_pend2(acoral_evt_t *evt, unsigned int timeout)
{
	acoral_thread_t *cur;

	if (acoral_intr_nesting > 0)
		return MUTEX_ERR_INTR;

	cur = acoral_cur_thread;

	acoral_enter_critical();
	if (NULL == evt)
	{
		acoral_exit_critical();
		return MUTEX_ERR_NULL;
	}

	if ((unsigned char)(evt->count & MUTEX_L_MASK) == MUTEX_AVAI)
	{
		/* 申请成功*/
		evt->count &= MUTEX_U_MASK;
		evt->count |= cur->prio;
		evt->data = (void *)cur;

		/*提升至天花板优先级*/
		cur->prio = (evt->count & MUTEX_CEILING_MASK) >> 16;
		acoral_exit_critical();
		return MUTEX_SUCCED;
	}

	/* 互斥量已被占有*/
	acoral_unrdy_thread(cur);
	acoral_evt_queue_add(evt, cur);
	if (timeout > 0)
	{
		/*加载到超时队列*/
		cur->delay = time_to_ticks(timeout);
		timeout_queue_add(cur);
	}
	acoral_exit_critical();

	/*触发调度*/
	acoral_sched();

	acoral_enter_critical();

	/*超时时间内未获得互斥量*/
	if (evt->data != cur && timeout > 0 && cur->delay <= 0)
	{
		printf("Time Out Return\n");
		acoral_evt_queue_del(cur);
		acoral_exit_critical();
		return MUTEX_ERR_TIMEOUT;
	}

	/*超时时间内获得了互斥量*/
	timeout_queue_del(cur);

	if (evt->data != cur)
	{
		printf("Err Ready Return\n");
		acoral_evt_queue_del(cur);
		acoral_exit_critical();
		return MUTEX_ERR_RDY;
	}

	return MUTEX_SUCCED;
}

acoralMutexRetVal acoral_mutex_post(acoral_evt_t *evt)
{
	unsigned char ownerPrio;
	unsigned char highPrio;
	acoral_thread_t *thread;
	acoral_thread_t *cur;

	acoral_enter_critical();

	if (NULL == evt)
	{
		printf("mutex NULL\n");
		acoral_exit_critical();
		return MUTEX_ERR_NULL; /*error*/
	}

	highPrio = (unsigned char)(evt->count >> 8);
	ownerPrio = (unsigned char)(evt->count & MUTEX_L_MASK);
	cur = acoral_cur_thread;
	if (highPrio != 0 && cur->prio != highPrio && cur->prio != ownerPrio)
	{
		printf("mutex prio err\n");
		acoral_exit_critical();
		return MUTEX_ERR_UNDEF;
	}
	cur->evt = NULL;
	if (cur->prio != ownerPrio)
	{
		/* 提升过优先级，进行优先级复原*/
		acoral_change_prio_self(ownerPrio);
	}

	thread = acoral_evt_high_thread(evt);
	if (thread == NULL)
	{
		evt->count |= MUTEX_AVAI;
		evt->data = NULL;
		acoral_exit_critical();
		return MUTEX_SUCCED;
	}
	timeout_queue_del(thread);
	acoral_evt_queue_del(thread);
	evt->count &= MUTEX_U_MASK;
	evt->count |= thread->prio;
	evt->data = thread;
	acoral_rdy_thread(thread);
	acoral_exit_critical();
	acoral_sched();
	return MUTEX_SUCCED;
}
