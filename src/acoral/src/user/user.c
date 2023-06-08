#include <stdio.h>
#include "acoral.h"
#include <assert.h>

void p1(){
    printf("in p1 %s\n",__FILE__);
    int i = 0;
    i++;
    printf("out p1\n");
    return;
}

void user_main(){
    // acoral_period_policy_data_t* p1data = acoral_malloc(sizeof(acoral_period_policy_data_t));
    // p1data->time = 2000;
    // p1data->prio = 20;
    // p1data->prio_type = ACORAL_HARD_PRIO;
    acoral_period_policy_data_t p1data={
        .time=2000,
        .prio=21,
        .prio_type=ACORAL_HARD_PRIO
    };

#ifndef NDEBUG
    // Assert(p1data.prio>30,"priority need to be greater than 30");
#endif
    acoral_create_thread(p1,0,NULL,"p1",NULL,ACORAL_SCHED_POLICY_PERIOD,&p1data);
}