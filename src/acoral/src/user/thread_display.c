#include "thread.h"
#include "policy.h"
#include "hal.h"
#include "shell.h"
#include <stdio.h>

void display_thread(int argc,char **argv){	
	acoral_list_t *head,*tmp;
	acoral_thread_t * thread;
	head=&acoral_threads_queue;
	printf("\t\tSystem Thread Information\r\n");
	printf("------------------------------------------------------\r\n");
	printf("Name\t\tType\t\tState\t\tPrio\r\n");
	HAL_ENTER_CRITICAL();

	for(tmp=head->next;tmp!=head;tmp=tmp->next){
		thread=list_entry(tmp,acoral_thread_t,global_list);
		printf("%s\t\t",thread->name);
		switch(thread->policy){
			case ACORAL_SCHED_POLICY_COMM:
				printf("Common\t\t");
				break;
			case ACORAL_SCHED_POLICY_PERIOD:
				printf("Period\t\t");
				break;
			default:
				break;
		}
		if(thread->state&ACORAL_THREAD_STATE_RUNNING)
				printf("Running\t\t");
		else if(thread->state&ACORAL_THREAD_STATE_READY)
				printf("Ready\t\t");
		else if(thread->state&ACORAL_THREAD_STATE_DELAY)
				printf("Delay\t\t");
		else if(thread->state&ACORAL_THREAD_STATE_SUSPEND)
				printf("Sleep\t\t");
		else if(thread->state==ACORAL_THREAD_STATE_EXIT)
				printf("Freeze\t\t");
		else
				printf("Error\t\t");
		
		printf("%d\t\t",thread->prio);
		printf("\r\n");
	}
	printf("------------------------------------------------------\r\n");

	HAL_EXIT_CRITICAL();
}

acoral_shell_cmd_t dt_cmd={
	"dt",
	(void*)display_thread,
	"View all thread info",
	NULL
};
