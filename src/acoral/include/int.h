/**
 * @file int.h
 * @author 王彬浩 (SPGGOGOGO@outlook.com)
 * @brief kernel层，aCoral中断相关头文件
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
#ifndef ACORAL_INT_H
#define ACORAL_INT_H

/**
 * @brief 中断结构体
 * 
 */
typedef struct {
	unsigned char  type;				///<上面三种中断类型
	void (*isr)(int);		///<中断服务程序
	void (*enter)(int);	///<中断服务程序执行之前执行的操作，aCroal中为hal_intr_ack函数
	void (*exit)(int);	///<中断服务程序执行完成后的操作，比如置中断结束，目前aCoral中没有这个操作
	void (*mask)(int);	///<除能中断操作
	void (*unmask)(int);	///<使能中断操作
}acoral_intr_ctr_t;

/**
 * @brief aCoral全局中断打开
 * 
 */
#define acoral_intr_enable() HAL_INTR_ENABLE()

/**
 * @brief aCoral全局中断关闭
 * 
 */
#define acoral_intr_disable() HAL_INTR_DISABLE()

/**
 * @brief aCoral进入临界区（本质就是关中断）
 * 
 */
#define acoral_enter_critical() HAL_ENTER_CRITICAL()

/**
 * @brief aCoral退出临界区（本质就是开中断）
 * 
 */
#define acoral_exit_critical() HAL_EXIT_CRITICAL()

#define acoral_intr_nesting HAL_GET_INTR_NESTING()
#define acoral_intr_nesting_inc() HAL_INTR_NESTING_INC()
#define acoral_intr_nesting_dec() HAL_INTR_NESTING_DEC()


void acoral_default_isr(int vector);
void acoral_intr_sys_init();

/***************中断相关API****************/

/**
 * @brief 给某个plic中断绑定中断服务函数
 * 
 * @param vector 中断号
 * @param isr 中断服务函数
 * @return int 0 success
 */
int acoral_intr_attach(int vector,void (*isr)(int));

/**
 * @brief 给某个plic中断解绑中断服务函数，并换成aCoral默认的中断服务函数acoral_default_isr
 * 
 * @param vector 中断号
 * @return int 0 success
 */
int acoral_intr_detach(int vector);

/**
 * @brief 使能某个中断
 * 
 * @param vector 中断号
 * @return int 返回0成功，其它失败
 */
int acoral_intr_unmask(int vector);

/**
 * @brief 除能某个中断
 * 
 * @param vector 中断号
 * @return int 返回0成功，其它失败
 */
int acoral_intr_mask(int vector);

#endif
