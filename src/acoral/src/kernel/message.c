/**
 * @file message.c
 * @author 王彬浩 (SPGGOGOGO@outlook.com)
 * @brief kernel层，消息机制
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
#include "message.h"
#include <stdio.h>

acoral_pool_ctrl_t acoral_msgctr_pool_ctrl;
acoral_pool_ctrl_t acoral_msg_pool_ctrl;
acoral_list_t g_msgctr_header; ///< 全局-用来串系统中所有的acoral_msgctr_t块，在中断函数中处理ttl和timeout，create acoral_msgctr_t 时加到该链表中

void acoral_msg_sys_init()
{
	/*初始化全局事件列表头*/
	acoral_init_list(&(g_msgctr_header));
	acoral_msgctr_pool_ctrl.type = ACORAL_RES_MST;
	acoral_msgctr_pool_ctrl.size = sizeof(acoral_msgctr_t);
	acoral_msgctr_pool_ctrl.num_per_pool = 10;
	acoral_msgctr_pool_ctrl.max_pools = 4;

	acoral_msg_pool_ctrl.type = ACORAL_RES_MSG;
	acoral_msg_pool_ctrl.size = sizeof(acoral_msg_t);
	acoral_msg_pool_ctrl.num_per_pool = 10;
	acoral_msg_pool_ctrl.max_pools = 4;

	acoral_pool_ctrl_init(&acoral_msgctr_pool_ctrl);
	acoral_pool_ctrl_init(&acoral_msg_pool_ctrl);
}

acoral_msgctr_t *acoral_alloc_msgctr()
{
	return (acoral_msgctr_t *)acoral_get_res(&acoral_msgctr_pool_ctrl);
}

acoral_msg_t *acoral_alloc_msg()
{
	return (acoral_msg_t *)acoral_get_res(&acoral_msg_pool_ctrl);
}

void acoral_msgctr_queue_add(acoral_msgctr_t *msgctr,
							 acoral_thread_t *thread)
{ /*需按优先级排序*/
	/*acoral_list_add2_tail (&thread->waiting, &msgctr->waiting);*/
	acoral_list_t *p, *q;
	acoral_thread_t *ptd;

	p = &msgctr->waiting;
	q = p->next;
	for (; p != q; q = q->next)
	{
		ptd = list_entry(q, acoral_thread_t, waiting);
		if (ptd->prio > thread->prio)
			break;
	}
	acoral_list_add(&thread->waiting, q->prev);
}

acoral_msgctr_t *acoral_msgctr_create()
{
	acoral_msgctr_t *msgctr;

	msgctr = acoral_alloc_msgctr();

	if (msgctr == NULL)
		return NULL;

	msgctr->name = NULL;
	msgctr->count = 0;
	msgctr->wait_thread_num = 0;

	acoral_init_list(&msgctr->msgctr_list);
	acoral_init_list(&msgctr->msglist);
	acoral_init_list(&msgctr->waiting);

	acoral_list_add2_tail(&msgctr->msgctr_list, &(g_msgctr_header));
	return msgctr;
}

acoral_msg_t *acoral_msg_create(
	unsigned int count, unsigned int id,
	unsigned int nTtl /* = 0*/, void *dat /*= NULL*/)
{
	acoral_msg_t *msg;

	msg = acoral_alloc_msg();

	if (msg == NULL)
		return NULL;

	msg->id = id;	 /*消息标识*/
	msg->count = count;		 /*消息被接收次数*/
	msg->ttl = nTtl; /*消息生存周期*/
	msg->data = dat; /*消息指针*/
	acoral_init_list(&msg->msglist);
	return msg;
}

unsigned int acoral_msg_send(acoral_msgctr_t *msgctr, acoral_msg_t *msg)
{
	/*	if (acoral_intr_nesting > 0)
			return MST_ERR_INTR;
	*/
	acoral_enter_critical();

	if (NULL == msgctr)
	{
		acoral_exit_critical();
		return MST_ERR_NULL;
	}

	if (NULL == msg)
	{
		acoral_exit_critical();
		return MSG_ERR_NULL;
	}

	/*----------------*/
	/*   消息数限制*/
	/*----------------*/
	if (ACORAL_MESSAGE_MAX_COUNT <= msgctr->count)
	{
		acoral_exit_critical();
		return MSG_ERR_COUNT;
	}

	/*----------------*/
	/*   增加消息*/
	/*----------------*/
	msgctr->count++;
	msg->ttl += acoral_get_ticks();
	acoral_list_add2_tail(&msg->msglist, &msgctr->msglist);

	/*----------------*/
	/*   唤醒等待*/
	/*----------------*/
	if (msgctr->wait_thread_num > 0)
	{
		/* 此处将最高优先级唤醒*/
		wake_up_thread(&msgctr->waiting);
		msgctr->wait_thread_num--;
	}
	acoral_exit_critical();
	acoral_sched();
	return MSGCTR_SUCCED;
}

void *acoral_msg_recv(acoral_msgctr_t *msgctr,
					  unsigned int id,
					  unsigned int timeout,
					  unsigned int *err)
{
	void *dat;
	acoral_list_t *p, *q;
	acoral_msg_t *pmsg;
	acoral_thread_t *cur;

	if (acoral_intr_nesting > 0)
	{
		*err = MST_ERR_INTR;
		return NULL;
	}
	if (NULL == msgctr)
	{
		*err = MST_ERR_NULL;
		return NULL;
	}

	cur = acoral_cur_thread;

	acoral_enter_critical();
	if (timeout > 0)
	{
		cur->delay = time_to_ticks(timeout);
		timeout_queue_add(cur);
	}
	while (1)
	{
		p = &msgctr->msglist;
		q = p->next;
		for (; p != q; q = q->next)
		{
			pmsg = list_entry(q, acoral_msg_t, msglist);
			if ((pmsg->id == id) && (pmsg->count > 0))
			{
				/*-----------------*/
				/* 有接收消息*/
				/*-----------------*/
				pmsg->count--;
				/*-----------------*/
				/* 延时列表删除*/
				/*-----------------*/
				timeout_queue_del(cur);
				dat = pmsg->data;
				acoral_list_del(q);
				acoral_release_res((acoral_res_t *)pmsg);
				msgctr->count--;
				acoral_exit_critical();
				return dat;
			}
		}

		/*-----------------*/
		/*  没有接收消息*/
		/*-----------------*/
		msgctr->wait_thread_num++;
		acoral_msgctr_queue_add(msgctr, cur);
		acoral_unrdy_thread(cur);
		acoral_exit_critical();
		acoral_sched();
		/*-----------------*/
		/*  看有没有超时*/
		/*-----------------*/
		acoral_enter_critical();

		if (timeout > 0 && (int)cur->delay <= 0)
			break;
	}

	/*---------------*/
	/*  超时退出*/
	/*---------------*/
	//	timeout_queue_del(cur);
	if (msgctr->wait_thread_num > 0)
		msgctr->wait_thread_num--;
	acoral_list_del(&cur->waiting);
	acoral_exit_critical();
	*err = MST_ERR_TIMEOUT;
	return NULL;
}

unsigned int acoral_msgctr_del(acoral_msgctr_t *pmsgctr, unsigned int flag)
{
	acoral_list_t *p, *q;
	acoral_thread_t *thread;
	acoral_msg_t *pmsg;

	if (NULL == pmsgctr)
		return MST_ERR_NULL;
	if (flag == MST_DEL_UNFORCE)
	{
		if ((pmsgctr->count > 0) || (pmsgctr->wait_thread_num > 0))
			return MST_ERR_UNDEF;
		else
			acoral_release_res((acoral_res_t *)pmsgctr);
	}
	else
	{
		// 释放等待进程
		if (pmsgctr->wait_thread_num > 0)
		{
			p = &pmsgctr->waiting;
			q = p->next;
			for (; q != p; q = q->next)
			{
				thread = list_entry(q, acoral_thread_t, waiting);
				// acoral_list_del  (&thread->waiting);
				acoral_rdy_thread(thread);
			}
		}

		// 释放消息结构
		if (pmsgctr->count > 0)
		{
			p = &pmsgctr->msglist;
			q = p->next;
			for (; p != q; q = p->next)
			{
				pmsg = list_entry(q, acoral_msg_t, msglist);
				acoral_list_del(q);
				acoral_release_res((acoral_res_t *)pmsg);
			}
		}

		// 释放资源
		acoral_release_res((acoral_res_t *)pmsgctr);
	}
	return MSGCTR_SUCCED;
}

unsigned int acoral_msg_del(acoral_msg_t *pmsg)
{
	if (NULL != pmsg)
		acoral_release_res((acoral_res_t *)pmsg);
	return 0;
}

void wake_up_thread(acoral_list_t *head)
{
	acoral_list_t *p, *q;
	acoral_thread_t *thread;

	p = head;
	q = p->next;
	thread = list_entry(q, acoral_thread_t, waiting);
	acoral_list_del(&thread->waiting);
	acoral_rdy_thread(thread);
}

void acoral_print_all_msg(acoral_msgctr_t *msgctr)
{
	acoral_list_t *p, *q;
	acoral_msg_t *pmsg;

	p = &msgctr->msglist;
	q = p->next;
	for (; p != q; q = q->next)
	{
		pmsg = list_entry(q, acoral_msg_t, msglist);
		printf("\nid = %d", pmsg->id);
	}
}