/**
 * @file hal_int_c.c
 * @author 王彬浩 (SPGGOGOGO@outlook.com)
 * @brief hal层，中断二级入口、中断开关以及中断初始化c代码
 * @version 1.0
 * @date 2022-06-23
 * @copyright Copyright (c) 2023
 *
 * <table>
 * <tr><th> 版本 <th>作者 <th>日期 <th>修改内容
 * <tr><td> 0.1 <td>jivin <td>2010-3-8 <td>Created
 * <tr><td> 1.0 <td>王彬浩 <td>2022-06-23 <td>Standardized
 * </table>
 */

#include "hal_int.h"
#include "lsched.h"

///中断嵌套数。大于0表示正在中断中。大于1表示中断层数不止一层，即中断嵌套。
int acoral_intr_nesting = 0;

void hal_intr_unmask(int vector)
{
	// TODO riscv-加上
}

void hal_intr_mask(int vector)
{
	// TODO riscv-加上
}





void hal_intr_nesting_dec_comm()
{
	if (acoral_intr_nesting > 0)
		acoral_intr_nesting--;
}

void hal_intr_nesting_inc_comm()
{
	acoral_intr_nesting++;
}

void hal_sched_bridge_comm()
{
	HAL_ENTER_CRITICAL(); //SPG这里关了中断，线程切换后，新的线程不就默认是关中断的状态了么
	acoral_real_sched();
	HAL_EXIT_CRITICAL();
}

void hal_intr_exit_bridge_comm()
{

	HAL_ENTER_CRITICAL();
	acoral_real_intr_sched();
	HAL_EXIT_CRITICAL();
}
