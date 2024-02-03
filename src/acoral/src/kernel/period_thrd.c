/**
 * @file period_thrd.c
 * @author 王彬浩 (SPGGOGOGO@outlook.com)
 * @brief kernel层，周期线程
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

#include "thread.h"
#include "hal.h"
#include "policy.h"
#include "mem.h"
#include "timer.h"
#include "period_thrd.h"
#include "int.h"
#include <stdio.h>
#include "lsched.h"
#include "dag.h"

acoral_list_t period_delay_queue; ///<周期线程专用延时队列，只要是周期线程，就会被挂载到这个队列上，延时时间就是周期，每次周期过后重新挂载
int period_policy_thread_init(acoral_thread_t *thread,void (*route)(void *args),void *args,void *data){
	unsigned int prio;
	acoral_period_policy_data_t *policy_data;
	period_private_data_t *private_data;
	if(thread->policy==ACORAL_SCHED_POLICY_PERIOD){
		policy_data=(acoral_period_policy_data_t *)data;
		prio=policy_data->prio;
		if(policy_data->prio_type==ACORAL_NONHARD_PRIO){
			prio+=ACORAL_NONHARD_RT_PRIO_MAX;
			if(prio>=ACORAL_NONHARD_RT_PRIO_MIN)
				prio=ACORAL_NONHARD_RT_PRIO_MIN-1;
		}
		//SPG加上硬实时判断
		// else{
		// 	prio += ACORAL_HARD_RT_PRIO_MAX;
		// 	if(prio > ACORAL_HARD_RT_PRIO_MIN){
		// 		prio = ACORAL_HARD_RT_PRIO_MIN;
		// }
		// }
		thread->prio=prio;
		private_data=(period_private_data_t *)acoral_malloc(sizeof(period_private_data_t));
		if(private_data==NULL){
			printf("No level2 mem space for private_data:%s\n",thread->name);
			acoral_enter_critical();
			acoral_release_res((acoral_res_t *)thread);
			acoral_exit_critical();
			return -1;
		}
		private_data->time=policy_data->time;
		private_data->route=route;
		private_data->args=args;
		thread->private_data=private_data;
	}
	if(acoral_thread_init(thread,route,period_thread_exit,args)!=0){
		printf("No thread stack:%s\n",thread->name);
		acoral_enter_critical();
		acoral_release_res((acoral_res_t *)thread);
		acoral_exit_critical();
		return -1;
	}
        /*将线程就绪，并重新调度*/
	acoral_resume_thread(thread);
	acoral_enter_critical();
	period_thread_delay(thread,((period_private_data_t *)thread->private_data)->time);
	acoral_exit_critical();
	return thread->res.id;
}

void period_policy_thread_release(acoral_thread_t *thread){
	acoral_free2(thread->private_data);	
}

void acoral_periodqueue_add(acoral_thread_t *new){
	acoral_list_t   *tmp,*head;
	acoral_thread_t *thread;
	int  delay2;
	int  delay= new->delay;
	head=&period_delay_queue;
	new->state|=ACORAL_THREAD_STATE_DELAY;
	for (tmp=head->next;delay2=delay,tmp!=head; tmp=tmp->next){
		thread = list_entry (tmp, acoral_thread_t, waiting);
		delay  = delay-thread->delay;
		if (delay < 0)
			break;
	}
	new->delay = delay2;
	acoral_list_add(&new->waiting,tmp->prev);
	/* 插入等待任务后，后继等待任务时间处理*/
	if(tmp != head){
		thread = list_entry(tmp, acoral_thread_t, waiting);
		thread->delay-=delay2;
	}
}

void period_thread_delay(acoral_thread_t* thread,unsigned int time){
	thread->delay=time_to_ticks(time);
	acoral_periodqueue_add(thread);
}

void period_delay_deal(){
	// int need_re_sched= 0;
	acoral_list_t *tmp,*tmp1,*head;
	acoral_thread_t * thread;
	period_private_data_t * private_data;
	head=&period_delay_queue;
	if(acoral_list_empty(head))
	    	return;
	thread=list_entry(head->next,acoral_thread_t,waiting);
	thread->delay--;
	for(tmp=head->next;tmp!=head;){
		thread=list_entry(tmp,acoral_thread_t,waiting);
		if(thread->delay>0)
		    break;
		private_data=thread->private_data;
		/*防止add判断delay时取下thread*/
		tmp1=tmp->next;
		acoral_list_del(&thread->waiting);
		tmp=tmp1;
		if(thread->state&ACORAL_THREAD_STATE_SUSPEND){
			thread->stack=(unsigned int *)((char *)thread->stack_buttom+thread->stack_size-4);
			thread->stack = HAL_STACK_INIT(thread->stack,private_data->route,period_thread_exit,private_data->args);
			acoral_rdy_thread(thread);
			// need_re_sched = 1;
		}
		period_thread_delay(thread,private_data->time);
		// if(need_re_sched)
		// {
		// 	acoral_sched();//SPG我是小丑，时钟中断里不能调度的，应该在把中断退出函数加回来
		// }
	}
}

void period_thread_exit(){
	acoral_suspend_self();
}

acoral_sched_policy_t period_policy;
void period_policy_init(void){
	acoral_init_list(&period_delay_queue);
	period_policy.type=ACORAL_SCHED_POLICY_PERIOD;
	period_policy.policy_thread_init=period_policy_thread_init;
	period_policy.policy_thread_release=period_policy_thread_release;
	period_policy.delay_deal=period_delay_deal;
	period_policy.name="period";
	acoral_register_sched_policy(&period_policy);
}
