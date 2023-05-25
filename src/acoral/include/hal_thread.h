/**
 * @file hal_thread.h
 * @author 王彬浩 (SPGGOGOGO@outlook.com)
 * @brief hal层，线程相关头文件
 * @version 1.1
 * @date 2023-04-120
 * @copyright Copyright (c) 2023
 * @revisionHistory 
 *  <table> 
 *   <tr><th> 版本 <th>作者 <th>日期 <th>修改内容 
 *   <tr><td> 0.1 <td>jivin <td>2010-03-08 <td>Created 
 *   <tr><td> 1.0 <td>王彬浩 <td> 2022-07-17 <td>Standardized 
 *   <tr><td> 1.1 <td>王彬浩 <td> 2023-04-20 <td>riscv
 *  </table>
 */

#ifndef HAL_THREAD_H
#define HAL_THREAD_H

/**
 * @brief aCoral线程上下文context在硬件层面的描述
 */
typedef struct {
  	unsigned long epc;        /* epc - epc    - program counter                     */
    unsigned long ra;         /* x1  - ra     - return address for jumps            */
    unsigned long mstatus;    /*              - machine status register             */
    unsigned long gp;         /* x3  - gp     - global pointer                      */
    unsigned long tp;         /* x4  - tp     - thread pointer                      */
    unsigned long t0;         /* x5  - t0     - temporary register 0                */
    unsigned long t1;         /* x6  - t1     - temporary register 1                */
    unsigned long t2;         /* x7  - t2     - temporary register 2                */
    unsigned long s0_fp;      /* x8  - s0/fp  - saved register 0 or frame pointer   */
    unsigned long s1;         /* x9  - s1     - saved register 1                    */
    unsigned long a0;         /* x10 - a0     - return value or function argument 0 */
    unsigned long a1;         /* x11 - a1     - return value or function argument 1 */
    unsigned long a2;         /* x12 - a2     - function argument 2                 */
    unsigned long a3;         /* x13 - a3     - function argument 3                 */
    unsigned long a4;         /* x14 - a4     - function argument 4                 */
    unsigned long a5;         /* x15 - a5     - function argument 5                 */
    unsigned long a6;         /* x16 - a6     - function argument 6                 */
    unsigned long a7;         /* x17 - s7     - function argument 7                 */
    unsigned long s2;         /* x18 - s2     - saved register 2                    */
    unsigned long s3;         /* x19 - s3     - saved register 3                    */
    unsigned long s4;         /* x20 - s4     - saved register 4                    */
    unsigned long s5;         /* x21 - s5     - saved register 5                    */
    unsigned long s6;         /* x22 - s6     - saved register 6                    */
    unsigned long s7;         /* x23 - s7     - saved register 7                    */
    unsigned long s8;         /* x24 - s8     - saved register 8                    */
    unsigned long s9;         /* x25 - s9     - saved register 9                    */
    unsigned long s10;        /* x26 - s10    - saved register 10                   */
    unsigned long s11;        /* x27 - s11    - saved register 11                   */
    unsigned long t3;         /* x28 - t3     - temporary register 3                */
    unsigned long t4;         /* x29 - t4     - temporary register 4                */
    unsigned long t5;         /* x30 - t5     - temporary register 5                */
    unsigned long t6;         /* x31 - t6     - temporary register 6                */
#ifdef ARCH_ACORAL_FPU
    unsigned int f0;      /* f0  */
    unsigned int f1;      /* f1  */
    unsigned int f2;      /* f2  */
    unsigned int f3;      /* f3  */
    unsigned int f4;      /* f4  */
    unsigned int f5;      /* f5  */
    unsigned int f6;      /* f6  */
    unsigned int f7;      /* f7  */
    unsigned int f8;      /* f8  */
    unsigned int f9;      /* f9  */
    unsigned int f10;     /* f10 */
    unsigned int f11;     /* f11 */
    unsigned int f12;     /* f12 */
    unsigned int f13;     /* f13 */
    unsigned int f14;     /* f14 */
    unsigned int f15;     /* f15 */
    unsigned int f16;     /* f16 */
    unsigned int f17;     /* f17 */
    unsigned int f18;     /* f18 */
    unsigned int f19;     /* f19 */
    unsigned int f20;     /* f20 */
    unsigned int f21;     /* f21 */
    unsigned int f22;     /* f22 */
    unsigned int f23;     /* f23 */
    unsigned int f24;     /* f24 */
    unsigned int f25;     /* f25 */
    unsigned int f26;     /* f26 */
    unsigned int f27;     /* f27 */
    unsigned int f28;     /* f28 */
    unsigned int f29;     /* f29 */
    unsigned int f30;     /* f30 */
    unsigned int f31;     /* f31 */
#endif
}hal_ctx_t;

void HAL_SWITCH_TO(unsigned int** next);
void HAL_CONTEXT_SWITCH(unsigned int **prev , unsigned int **next);
void HAL_INTR_SWITCH_TO(unsigned int **next);
void HAL_INTR_CTX_SWITCH(unsigned int **prev , unsigned int **next);
unsigned int* hal_stack_init(unsigned int *stack, void *route, void *exit, void *args);

//线程相关的硬件抽象接口 //TODO 全大写为了和汇编接口统一
#define HAL_STACK_INIT(stack,route,exit,args) hal_stack_init(stack, route,exit, args)

#endif