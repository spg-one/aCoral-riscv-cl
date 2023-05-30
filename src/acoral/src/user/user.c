#include <stdio.h>
#include "acoral.h"

void p1(){
    printf("in p1\n");
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
        .prio=20,
        .prio_type=ACORAL_HARD_PRIO
    };
    acoral_create_thread(p1,0,NULL,"p1",NULL,ACORAL_SCHED_POLICY_PERIOD,&p1data);
}