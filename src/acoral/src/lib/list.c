/**
 * @file list.c
 * @author 王彬浩 (SPGGOGOGO@outlook.com)
 * @brief lib层，aCoral队列机制
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

#include "list.h"
void acoral_list_add(acoral_list_t *new, acoral_list_t *head)
{
	new->prev=head;	
	new->next=head->next;	
	head->next->prev=new;	
	head->next=new;	
}

void acoral_list_add2_tail(acoral_list_t *new, acoral_list_t *head)
{
	new->prev=head->prev;
	new->next=head;
	head->prev->next=new;
	head->prev=new;
}

void acoral_list_del(acoral_list_t *entry)
{
	entry->prev->next= entry->next;
	entry->next->prev=entry->prev;
	entry->next = entry;
	entry->prev = entry;
}
