/**
 * @file bitops.h
 * @author 王彬浩 (SPGGOGOGO@outlook.com)
 * @brief lib层
 * @version 1.0
 * @date 2023-04-19
 * @copyright Copyright (c) 2023
 * @revisionHistory 
 *  <table> 
 *   <tr><th> 版本 <th>作者 <th>日期 <th>修改内容 
 *   <tr><td> 0.1 <td>jivin <td>2010-03-08 <td>Created 
 *   <tr><td> 1.0 <td>王彬浩 <td> 2023-04-19 <td>Standardized 
 *  </table>
 */


#ifndef ACORAL_BITOPS_H
#define ACORAL_BITOPS_H

unsigned int acoral_find_first_bit(const unsigned int *b,unsigned int length);
void acoral_set_bit(int nr,unsigned int *p);
void acoral_clear_bit(int nr,unsigned int *p);
unsigned int acoral_ffs(unsigned int word);
unsigned int acoral_get_bit(int nr,unsigned int *bitmap);
#endif
