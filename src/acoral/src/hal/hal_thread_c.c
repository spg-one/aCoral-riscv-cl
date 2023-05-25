/**
 * @file hal_thread_c.c
 * @author 王彬浩 (SPGGOGOGO@outlook.com)
 * @brief hal层，线程硬件相关代码
 * @version 1.0
 * @date 2022-07-22
 * @copyright Copyright (c) 2023
 * @revisionHistory 
 *  <table> 
 *   <tr><th> 版本 <th>作者 <th>日期 <th>修改内容 
 *   <tr><td> 0.1 <td>jivin <td>2010-03-08 <td>Created 
 *   <tr><td> 1.0 <td>王彬浩 <td> 2022-07-22 <td>Standardized 
 *  </table>
 */

#include "hal_thread.h"
#include <stdio.h>
/**
 * @brief 线程上下文初始化，用于线程被切换到cpu上运行后，替换之前的线程的上下文
 * 
 * @param stk 栈指针
 * @param route 线程运行函数指针
 * @param exit 线程退出函数指针
 * @param args 线程运行函数参数
 */

#define ACORAL_ALIGN_DOWN(size, align)      ((size) & ~((align) - 1))

unsigned int* hal_stack_init(unsigned int *stack, void *route, void *exit, void *args)
{
    hal_ctx_t *frame;
    unsigned int *stk;
    int i;
    
    stk  = stack + sizeof(unsigned int);
    stk  = (unsigned int *)ACORAL_ALIGN_DOWN((unsigned int)stk, 8);
    stk -= sizeof(hal_ctx_t);

    frame = (hal_ctx_t *)stk;

    for (i = 0; i < sizeof(hal_ctx_t) / sizeof(unsigned long); i++)
    {
        ((unsigned long *)frame)[i] = 0xdeadbeef;
    }
    frame->ra      = (unsigned long)exit;
    frame->a0      = (unsigned long)args;
    frame->epc     = (unsigned long)route;

    /* force to machine mode(MPP=11) and set MPIE to 1 */
    frame->mstatus = 0x00007880;
    
    return stk;
}
