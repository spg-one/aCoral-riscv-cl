/**
 * @file int.c
 * @author 王彬浩 (SPGGOGOGO@outlook.com)
 * @brief kernel层，中断相关函数
 * @version 1.0
 * @date 2022-07-24
 * @copyright Copyright (c) 2023
 * @revisionHistory
 *  <table>
 *   <tr><th> 版本 <th>作者 <th>日期 <th>修改内容
 *   <tr><td> 0.1 <td>jivin <td>2010-03-08 <td>Created
 *   <tr><td> 1.0 <td>王彬浩 <td> 2022-07-24 <td>Standardized
 *  </table>
 */

#include "hal.h"
#include "lsched.h"
#include "int.h"
#include "plic.h"
#include <stdio.h>

void acoral_intr_sys_init()
{
	/*关中断*/
	acoral_intr_disable();

	/*中断嵌套标志初始化*/
	HAL_INTR_NESTING_INIT();

	/*中断底层初始化函数*/
	plic_init();
}

int acoral_intr_attach(int vector,void (*isr)(int)){
	plic_irq_register(vector,isr,NULL);
	return 0;
}

int acoral_intr_detach(int vector){
	plic_irq_deregister(vector);
	plic_irq_register(vector,acoral_default_isr,NULL);
	return 0;
}

int acoral_intr_unmask(int vector){
	return plic_irq_enable(vector);
}

int acoral_intr_mask(int vector){
	return plic_irq_disable(vector);
}

void acoral_default_isr(int vector){
	printf("in acoral_default_isr");
}