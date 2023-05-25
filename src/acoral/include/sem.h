/**
 * @file sem.h
 * @author 王彬浩 (SPGGOGOGO@outlook.com)
 * @brief kernel层，信号量相关头文件
 * @version 1.0
 * @date 2023-04-21
 * @copyright Copyright (c) 2023
 * @revisionHistory
 *  <table>
 *   <tr><th> 版本 <th>作者 <th>日期 <th>修改内容
 *   <tr><td> 0.1 <td>jivin <td>2010-03-08 <td>Created
 *   <tr><td> 1.0 <td>王彬浩 <td> 2023-04-21 <td>Standardized
 *  </table>
 */

#ifndef _ACORAL_SEM_H
#define _ACORAL_SEM_H

#include "event.h"

typedef enum
{
    SEM_SUCCED,
    SEM_THREAD_SUSPEND,
    SEM_ERR_NULL,
    SEM_ERR_RES_NUM,
    SEM_ERR_TYPE,
    SEM_ERR_TASK_EXIST,
    SEM_ERR_INTR,
    SEM_ERR_UNDEF,
    SEM_ERR_TIMEOUT
} acoralSemRetValEnum;

typedef enum
{
    SEM_RES_AVAI,
    SEM_RES_NOAVAI
}acoralSemResAvailabiltyEnum;

/***************信号量相关API****************/

/**
 * @brief 初始化信号量
 *
 * @param evt 信号量指针
 * @param semNum 信号量的初始值
 * @return enum acoralSemRetValEnum
 */
acoralSemRetValEnum acoral_sem_init(acoral_evt_t *evt, unsigned int semNum);

/**
 * @brief 创建并初始化信号量
 *
 * @param semNum 信号量初始值
 * @return acoral_evt_t* 返回信号量指针
 */
acoral_evt_t *acoral_sem_create(unsigned int semNum);

/**
 * @brief 删除信号量//SPG这个函数删了个寂寞？都没释放内存
 *
 * @param evt 信号量指针
 * @return enum acoralSemRetValEnum
 */
acoralSemRetValEnum acoral_sem_del(acoral_evt_t *evt);

/**
 * @brief 获取信号量(非阻塞)
 *  desp: count <= SEM_RES_AVAI  信号量有效 a++
 *        count >  SEM_RES_AVAI  信号量无效 a++ && thread suspend
 *
 * @param evt 信号量指针
 * @return acoralSemRetValEnum
 */
acoralSemRetValEnum acoral_sem_trypend(acoral_evt_t *evt);

/**
 * @brief 获取信号量(阻塞式)
 *  desp: count <= SEM_RES_AVAI  信号量有效 a++
 *        count >  SEM_RES_AVAI  信号量无效 a++ && thread suspend
 *
 * @param evt 信号量指针
 * @param timeout 超时时间
 * @return acoralSemRetValEnum
 */
acoralSemRetValEnum acoral_sem_pend(acoral_evt_t *evt, unsigned int timeout);

/**
 * @brief 释放信号量
 *  desp: count > SEM_RES_NOAVAI 有等待线程 a-- && resume waiting thread.
 *        count <= SEM_RES_NOAVAI 无等待线程 a--
 *
 * @param evt 信号量指针
 * @return acoralSemRetValEnum
 */
acoralSemRetValEnum acoral_sem_post(acoral_evt_t *evt);

/**
 * @brief 得到当前信号量值
 *
 * @param evt 信号量指针
 * @return int 信号量值
 */
int acoral_sem_getnum(acoral_evt_t *evt);

#endif
