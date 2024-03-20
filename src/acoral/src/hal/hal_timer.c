#include "hal_timer.h"
#include "autocfg.h"

int hal_timer_init(int ticks_per_sec){

#ifdef CFG_K210
#include "clint.h"
extern void acoral_ticks_entry(int vector);
	clint_timer_init();                           	/*这个主要用于将用于ticks的时钟初始化*/
	clint_timer_register(acoral_ticks_entry,NULL);	/*这个用于注册ticks的处理函数*/
    return clint_timer_start(1000/ticks_per_sec,0);
#endif

#ifdef CFG_2440
    return 0;
#endif
    return -1;
}
