/**
 * @file thread.h
 * @author 王彬浩 (SPGGOGOGO@outlook.com)
 * @brief kernel层，线程优先级、控制块定义，线程管理函数声明
 * @version 1.1
 * @date 2023-04-19
 * @copyright Copyright (c) 2023
 * @revisionHistory 
 *  <table> 
 *   <tr><th> 版本 <th>作者 <th>日期 <th>修改内容 
 *   <tr><td> 0.1 <td>pegasus <td>2010-07-19 <td>增加timeout链表，用来处理超时，挂g_timeout_queue
 *   <tr><td> 1.0 <td>王彬浩 <td> 2022-06-24 <td>Standardized 
 * 	 <tr><td> 1.1 <td>王彬浩 <td> 2023-04-19 <td>use enum 
 *  </table>
 */

#ifndef ACORAL_THREAD_H
#define ACORAL_THREAD_H
#include "autocfg.h"
#include "list.h"
#include "mem.h"
#include "event.h"
#include "policy.h"

#define ACORAL_MAX_PRIO_NUM ((CFG_MAX_THREAD + 1) & 0xff) ///<41。总共有40个线程，就有0~40共41个优先级
#define ACORAL_MINI_PRIO CFG_MAX_THREAD ///<aCoral最低优先级40

extern acoral_list_t acoral_threads_queue;

typedef enum{
	ACORAL_INIT_PRIO,	///<init线程独有的0优先级
	ACORAL_MAX_PRIO,	///<aCoral系统中允许的最高优先级
	ACORAL_HARD_RT_PRIO_MAX,	///<硬实时任务最高优先级
	ACORAL_HARD_RT_PRIO_MIN = ACORAL_HARD_RT_PRIO_MAX+CFG_HARD_RT_PRIO_NUM,	///<硬实时任务最低优先级
	ACORAL_NONHARD_RT_PRIO_MAX,	///<非硬实时任务最高优先级

	ACORAL_DAEMON_PRIO = ACORAL_MINI_PRIO-2,	///<daemon回收线程专用优先级
	ACORAL_NONHARD_RT_PRIO_MIN,	///<非硬实时任务最低优先级
	ACORAL_IDLE_PRIO	///<idle线程专用优先级，也是系统最低优先级ACORAL_MINI_PRIO
}acoralPrioEnum;

typedef enum{
	ACORAL_NONHARD_PRIO, ///<非硬实时任务的优先级，会将tcb中的prio加上ACORAL_NONHARD_RT_PRIO_MAX
	ACORAL_HARD_PRIO	 ///<硬实时任务优先级，会将tcb中的prio加上ACORAL_HARD_RT_PRIO_MAX
}acoralPrioTypeEnum;

typedef enum{
	ACORAL_THREAD_STATE_READY = 1,			///表示线程已经被挂载到acoral_ready_queues，就绪状态
	ACORAL_THREAD_STATE_SUSPEND = 1<<1,		///表示线程已经被从acoral_ready_queues取下，挂起状态
	ACORAL_THREAD_STATE_RUNNING = 1<<2, 	///表示线程正在运行
	ACORAL_THREAD_STATE_EXIT = 1<<3,		///表示线程已经运行完毕，等待被切换彻底送走
	ACORAL_THREAD_STATE_RELEASE = 1<<4,		///表示线程已经运行完毕并已经被切换，等待daem回收TCB和堆栈资源
	ACORAL_THREAD_STATE_DELAY = 1<<5,		///表示线程将在一段时间之后被重新唤醒并挂载到acoral_ready_queues上，对于普通线程来说，就是调用了delay接口，对于周期线程来说，就是周期
}acoralThreadStateEnum;

typedef enum{
    ACORAL_ERR_THREAD,
    ACORAL_ERR_THREAD_DELAY,
    ACORAL_ERR_THREAD_NO_STACK  ///<线程栈指针为空
}acoralThreadErrorEnum;

/**
 * 
 *  @struct acoral_thread_t
 *  @brief 线程控制块TCB
 * 
 * 
 */
typedef struct acoral_thread_tcb{//SPG加注释
  	acoral_res_t res;	///<资源id，线程创建后作为线程id
	acoralThreadStateEnum state;
	unsigned char prio;
	acoralSchedPolicyEnum policy;
	acoral_list_t ready;	///<用于挂载到全局就绪队列
	acoral_list_t timeout;
	acoral_list_t waiting;
	acoral_list_t global_list;
	acoral_evt_t* evt;
	unsigned int *stack;
	unsigned int *stack_buttom;
	unsigned int stack_size;
	int delay;
	char *name;
	int console_id; ///<deprecated
	void*	private_data;
	void*	data;
}acoral_thread_t;

/**
 * @brief 这个函数只是在线程执行完毕或者被杀死的时候，让线程进入ACORAL_THREAD_STATE_EXIT状态。
 * 		  虽然这个时候线程已经结束嘞，但是知道发送上下文切换彻底送走这个死掉的线程之前，TCB和堆栈还有用处，比如函数调用还用得到堆栈，所以这个时候还不能被daem回收释放。
 * 		  所以进入ACORAL_THREAD_STATE_EXIT状态的的线程要等到被切换到新线程的上下文之后，才会变成ACORAL_THREAD_STATE_RELEASE状态，这个状态下的线程才会被daem释放。
 * 		  详见绿书P98.
 * 		  
 * 
 * @param thread 线程指针
 */
void acoral_release_thread1(acoral_thread_t *thread);

/**
 * @brief daem线程用来释放tcb空间的
 * 
 * @param thread 线程指针
 */
void acoral_release_thread(acoral_res_t *thread);
void acoral_suspend_thread(acoral_thread_t *thread);
void acoral_resume_thread(acoral_thread_t *thread);
void acoral_kill_thread(acoral_thread_t *thread);
unsigned int acoral_thread_init(acoral_thread_t *thread,void (*route)(void *args),void (*exit)(void),void *args);
acoral_thread_t *acoral_alloc_thread(void);
void acoral_thread_pool_init(void);
void acoral_thread_sys_init(void);
void acoral_unrdy_thread(acoral_thread_t *thread);
void acoral_rdy_thread(acoral_thread_t *thread);
void acoral_thread_move2_tail_by_id(int thread_id);
void acoral_thread_move2_tail(acoral_thread_t *thread);
void acoral_thread_change_prio(acoral_thread_t* thread, unsigned int prio);

/***************线程控制API****************/

/**
 * @brief 创建一个线程
 * 
 * @param route 线程函数
 * @param stack_size 线程栈大小
 * @param args 线程函数参数
 * @param name 线程名字
 * @param stack 线程栈指针
 * @param sched_policy 线程调度策略
 * @param data 线程策略数据
 * @return int 返回线程id
 */
int acoral_create_thread(void (*route)(void *args),unsigned int stack_size,void *args,char *name,void *stack,acoralSchedPolicyEnum sched_policy,void *data);

/**
 * @brief 挂起当前线程
 * 
 */
void acoral_suspend_self(void);

/**
 * @brief 挂起某个线程
 * 
 * @param thread_id 要挂起的线程id
 */
void acoral_suspend_thread_by_id(unsigned int thread_id);

/**
 * @brief 唤醒某个线程
 * 
 * @param thread_id 要唤醒的线程id
 */
void acoral_resume_thread_by_id(unsigned int thread_id);

/**
 * @brief 将当前线程延时
 * 
 * @param time 延时时间（毫秒）
 */
void acoral_delay_self(unsigned int time);

/**
 * @brief 干掉某个线程
 * 
 * @param id 要干掉的线程id
 */
void acoral_kill_thread_by_id(int id);

/**
 * @brief 结束当前线程
 * 
 */
void acoral_thread_exit(void);

/**
 * @brief 改变当前线程优先级
 * 
 * @param prio 目标优先级
 */
void acoral_change_prio_self(unsigned int prio);

/**
 * @brief 改变某个线程优先级
 * 
 * @param thread_id 线程id
 * @param prio 目标优先级
 */
void acoral_thread_change_prio_by_id(unsigned int thread_id, unsigned int prio);

#endif

