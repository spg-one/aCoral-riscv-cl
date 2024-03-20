/**
 * @file hal_timer.h
 * @author 王彬浩 (SPGGOGOGO@outlook.com)
 * @brief hal层，线程相关头文件
 * @version 1.0
 * @date 2024-03-20
 * @copyright Copyright (c) 2024
 * @revisionHistory 
 *  <table> 
 *   <tr><th> 版本 <th>作者 <th>日期 <th>修改内容 
 *   <tr><td> 1.0 <td>王彬浩 <td> 2022-07-17 <td>create
 *  </table>
 */

#ifndef HAL_TIMER_H
#define HAL_TIMER_H

/**
 * @brief 
 * 
 * @return result
 *     - 0      Success
 *     - Other  Fail 
 */

/**
 * @brief 配置ticks定时器的频率,打开其中断，并为其注册中断服务函数acoral_ticks_entry
 * 
 * @param ticks_per_sec 每秒的ticks中断数
 * @return result
 *     - 0      Success
 *     - Other  Fail 
 */
int hal_timer_init(int ticks_per_sec);

#define HAL_TIMER_INIT(ticks_per_sec) hal_timer_init(ticks_per_sec)

#endif