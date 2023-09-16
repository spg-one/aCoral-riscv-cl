/**
 * @file core.h
 * @author 王彬浩 (SPGGOGOGO@outlook.com)
 * @brief kernel层，core.c对应的头文件
 * @version 1.1
 * @date 2023-04-19
 * @copyright Copyright (c) 2023
 * @revisionHistory
 *  <table>
 *   <tr><th> 版本 <th>作者 <th>日期 <th>修改内容
 *   <tr><td> 0.1 <td>jivin <td>2010-03-08 <td>Created
 *   <tr><td> 1.0 <td>王彬浩 <td> 2022-07-23 <td>Standardized
 *   <tr><td> 1.1 <td>王彬浩 <td> 2023-04-19 <td>use enum
 *  </table>
 */
#ifndef ACORAL_CORE_H
#define ACORAL_CORE_H

#include "autocfg.h"

volatile extern unsigned int acoral_start_sched;

#define DAEM_STACK_SIZE 256
#define IDLE_STACK_SIZE 128

/**
 * @brief aCoral空闲守护线程idle函数
 *
 * @param args
 */
void idle(void *args);

/**
 * @brief aCoral资源回收线程daem函数
 *
 * @param args
 */
void daem(void *args);

/**
 * @brief aCoral初始化线程init函数
 *
 * @param args
 */
void init(void *args);

/**
 * @brief aCoral内核各模块初始化
 * 
 */
void acoral_module_init(void);

/**
 * @brief  c语言初始化入口函数
 * 
 */
void acoral_start(void);

/**
 * @brief CPU开始创建线程工作，创建idle线程、init线程
 * 
 */
void acoral_core_cpu_start(void);

/**
 * @brief aCoral最终启动
 * 
 */
void acoral_start_os(void);
#endif
