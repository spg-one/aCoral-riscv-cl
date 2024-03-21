#include "hal_timer.h"
#include "autocfg.h"

int hal_timer_init(int ticks_per_sec){

#ifdef CFG_K210
#include "clint.h"
extern void acoral_ticks_entry(int vector);
	clint_timer_init();                           	/*这个主要用于将用于ticks的时钟初始化*/
	clint_timer_register(acoral_ticks_entry,NULL);	//SPG 这里不应该直接使用acoral_ticks_entry，因为这是kernel层函数，应该将其作为参数传进来
    return clint_timer_start(1000/ticks_per_sec,0);
#endif

#ifdef CFG_2440
    return 0;
#endif
    return -1;
}
