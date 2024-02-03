/**
 * @file thread.c
 * @author 王彬浩 (SPGGOGOGO@outlook.com)
 * @brief kernel层，线程机制相关函数
 * @version 1.0
 * @date 2022-07-08
 * @copyright Copyright (c) 2023
 * @revisionHistory 
 *  <table> 
 *   <tr><th> 版本 <th>作者 <th>日期 <th>修改内容 
 *   <tr><td> 0.1 <td>jivin <td>2010-03-08 <td>Created 
 *   <tr><td> 1.0 <td>王彬浩 <td> 2022-07-08 <td>Standardized 
 *  </table>
 */


#include "hal.h"
#include "lsched.h"
#include "timer.h"
#include "mem.h"
#include "thread.h"
#include "int.h"
#include "policy.h"
#include <stdio.h>


extern acoral_list_t acoral_res_release_queue;
extern void acoral_evt_queue_del(acoral_thread_t *thread);
acoral_list_t acoral_threads_queue; ///<aCoral全局所有线程队列
acoral_pool_ctrl_t acoral_thread_pool_ctrl;

int acoral_create_thread(void (*route)(void *args),unsigned int stack_size,void *args,char *name,void *stack,acoralSchedPolicyEnum sched_policy,void *data){
	acoral_thread_t *thread;
        /*分配tcb数据块*/
	thread=acoral_alloc_thread();
	if(NULL==thread){
		printf("Alloc thread:%s fail\n",name);
		printf("No Mem Space or Beyond the max thread\n");
		return -1;
	}
	thread->name=name;
	stack_size=stack_size&(~3);
	thread->stack_size=stack_size;
	if(stack!=NULL)
		thread->stack_buttom=(unsigned int *)stack;
	else
		thread->stack_buttom=NULL;
	thread->policy=sched_policy;
	return acoral_policy_thread_init(sched_policy,thread,route,args,data);
}

extern int daemon_id;

void acoral_release_thread1(acoral_thread_t *thread){
	acoral_list_t *head;
	acoral_thread_t *daem;
	thread->state=ACORAL_THREAD_STATE_EXIT;
	head=&acoral_res_release_queue;
	acoral_list_add2_tail(&thread->waiting,head);

	daem=(acoral_thread_t *)acoral_get_res_by_id(daemon_id);
	acoral_rdy_thread(daem);
}

void acoral_release_thread(acoral_res_t *res){
	acoral_thread_t *thread;
	thread=(acoral_thread_t *)res;
	acoral_list_del(&thread->global_list);
	acoral_policy_thread_release(thread);
  	acoral_free((void *)thread->stack_buttom);
	acoral_release_res((acoral_res_t *)thread);
}

void acoral_suspend_thread(acoral_thread_t *thread){
	if(!(ACORAL_THREAD_STATE_READY&thread->state)) //SPG挂起一个线程等价于把线程从acoral_ready_queues上取下，这就意味着这个线程必须之前在acoral_ready_queues上，也就等价于必须是就绪状态的线程才能被挂起
		return;

	acoral_enter_critical();
	/**/
	acoral_rdyqueue_del(thread);
	acoral_exit_critical();
	/**/
	acoral_sched();
}

void acoral_suspend_self(){
	acoral_suspend_thread(acoral_cur_thread);
}

void acoral_suspend_thread_by_id(unsigned int thread_id){
	acoral_thread_t *thread=(acoral_thread_t *)acoral_get_res_by_id(thread_id);
	acoral_suspend_thread(thread);
}

void acoral_resume_thread_by_id(unsigned int thread_id){
	acoral_thread_t *thread=(acoral_thread_t *)acoral_get_res_by_id(thread_id);
	acoral_resume_thread(thread);
}

void acoral_resume_thread(acoral_thread_t *thread){
	if(!(thread->state&ACORAL_THREAD_STATE_SUSPEND))
		return;

	acoral_enter_critical();
	/**/
	acoral_rdyqueue_add(thread);
	acoral_exit_critical();
	/**/
	acoral_sched();
}

static void acoral_delay_thread(acoral_thread_t* thread,unsigned int time){
	unsigned int real_ticks;
	if(!acoral_list_empty(&thread->waiting)){
		return;	
	}

	/*timeticks*/
	/*real_ticks=time*CFG_TICKS_PER_SEC/1000;*/
	real_ticks = time_to_ticks(time);
	thread->delay=real_ticks;
	/**/
	acoral_delayqueue_add(&time_delay_queue,thread);
}

void acoral_delay_self(unsigned int time){
	acoral_delay_thread(acoral_cur_thread,time);
}

void acoral_kill_thread(acoral_thread_t *thread){
	acoral_evt_t *evt;
	acoral_enter_critical();
        /*	*/
        /*	*/
	if(thread->state&ACORAL_THREAD_STATE_SUSPEND){
		evt=thread->evt;
		/**/
		if(thread->state&ACORAL_THREAD_STATE_DELAY){
			acoral_list_del(&thread->waiting);
		}else
		{
			/**/
			if(evt!=NULL){
				acoral_evt_queue_del(thread);
			}
		}
	}
	acoral_unrdy_thread(thread);
	acoral_release_thread1(thread);
    acoral_exit_critical();
	acoral_sched();
}

void acoral_kill_thread_by_id(int id){
	acoral_thread_t *thread;
	thread=(acoral_thread_t *)acoral_get_res_by_id(id);
	acoral_kill_thread(thread);
}

void comm_thread_exit(){
        acoral_kill_thread(acoral_cur_thread);
}

void acoral_thread_change_prio(acoral_thread_t* thread, unsigned int prio){
	acoral_enter_critical();
	if(thread->state&ACORAL_THREAD_STATE_READY){
		acoral_rdyqueue_del(thread);
		thread->prio = prio;
		acoral_rdyqueue_add(thread);
	}else
		thread->prio = prio;
	acoral_exit_critical();
}

void acoral_change_prio_self(unsigned int prio){
	acoral_thread_change_prio(acoral_cur_thread, prio);
}

void acoral_thread_change_prio_by_id(unsigned int thread_id, unsigned int prio){
	acoral_thread_t *thread=(acoral_thread_t *)acoral_get_res_by_id(thread_id);
	acoral_thread_change_prio(thread, prio);
}

void acoral_rdy_thread(acoral_thread_t *thread){
	if(!(ACORAL_THREAD_STATE_SUSPEND&thread->state)) //SPG线程必须要不在就绪队列上（有suspend状态）才能被就绪（加入就绪队列）
		return;

	acoral_rdyqueue_add(thread);
}

void acoral_unrdy_thread(acoral_thread_t *thread){
	if(!(ACORAL_THREAD_STATE_READY&thread->state))
		return;

	acoral_rdyqueue_del(thread);
}

void acoral_thread_move2_tail(acoral_thread_t *thread){
	acoral_enter_critical();
	acoral_unrdy_thread(thread);
	acoral_rdy_thread(thread);
	acoral_exit_critical();
	acoral_sched();
}

void acoral_thread_move2_tail_by_id(int thread_id){
	acoral_thread_t *thread=(acoral_thread_t *)acoral_get_res_by_id(thread_id);
	acoral_thread_move2_tail(thread);
}

acoral_thread_t *acoral_alloc_thread(){
  	return (acoral_thread_t *)acoral_get_res(&acoral_thread_pool_ctrl);
}

unsigned int acoral_thread_init(acoral_thread_t *thread,void (*route)(void *args),void (*exit)(void),void *args){
	unsigned int stack_size=thread->stack_size;
	if(thread->stack_buttom==NULL){
		if(stack_size<CFG_MIN_STACK_SIZE)
			stack_size=CFG_MIN_STACK_SIZE;
		thread->stack_buttom=(unsigned int *)acoral_malloc(stack_size);
		if(thread->stack_buttom==NULL)
			return ACORAL_ERR_THREAD_NO_STACK;
		thread->stack_size=stack_size;
	}
	thread->stack=(unsigned int *)((char *)thread->stack_buttom+stack_size-4);
	thread->stack = HAL_STACK_INIT(thread->stack,route,exit,args);
	
	thread->data=NULL;
	thread->state=ACORAL_THREAD_STATE_SUSPEND;
	
	acoral_init_list(&thread->waiting);
	acoral_init_list(&thread->ready);
	acoral_init_list(&thread->timeout);
	acoral_init_list(&thread->global_list);

	acoral_enter_critical();
	acoral_list_add2_tail(&thread->global_list,&acoral_threads_queue);
	acoral_exit_critical();
	return 0;
}

void acoral_thread_pool_init(){
	acoral_thread_pool_ctrl.type=ACORAL_RES_THREAD;
	acoral_thread_pool_ctrl.size=sizeof(acoral_thread_t);
	if(CFG_MAX_THREAD>20)
		acoral_thread_pool_ctrl.num_per_pool=20;
	else
		acoral_thread_pool_ctrl.num_per_pool=CFG_MAX_THREAD;
	acoral_thread_pool_ctrl.max_pools=CFG_MAX_THREAD/acoral_thread_pool_ctrl.num_per_pool;
	acoral_pool_ctrl_init(&acoral_thread_pool_ctrl);
}

void acoral_sched_mechanism_init(){
	acoral_thread_pool_init();
	acoral_thread_runqueue_init();
	acoral_init_list(&acoral_threads_queue);
}

void acoral_thread_sys_init(){
	acoral_sched_mechanism_init();
	acoral_sched_policy_init();
}
