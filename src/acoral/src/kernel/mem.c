/**
 * @file mem.c
 * @author 王彬浩 (SPGGOGOGO@outlook.com)
 * @brief kernel层，整合了伙伴系统和资源池系统初始化的两级内存管理系统
 * @version 1.2
 * @date 2023-04-20
 * @copyright Copyright (c) 2023
 * @revisionHistory
 *  <table>
 *   <tr><th> 版本 <th>作者 <th>日期 <th>修改内容
 *   <tr><td> 0.1 <td>jivin <td>2010-03-08 <td>Created
 *   <tr><td> 1.0 <td>王彬浩 <td> 2022-07-04 <td>Standardized, add acoral_res_sys_init
 * 	 <tr><td> 1.1 <td>王彬浩 <td> 2022-07-06 <td>将resource.c 和 buddy.c放进来
 *   <tr><td> 1.2 <td>王彬浩 <td> 2023-04-20 <td>optimized
 *  </table>
 */

#include "autocfg.h"
#include "hal.h"
#include "mem.h"
#include "core.h"
#include "int.h"
#include "list.h"
#include <stdio.h>
#include "bitops.h"

extern int _heap_start; ///< 堆内存起始地址，定义于链接脚本
extern int _heap_end;	///< 堆内存结束地址，定义于链接脚本

void acoral_mem_sys_init()
{
	acoral_mem_init((unsigned int)&_heap_start, (unsigned int)&_heap_end); // 伙伴系统初始化
#ifdef CFG_MEM2
	acoral_mem_init2(); // 任意大小内存分配系统初始化
#endif
	acoral_res_sys_init(); // 资源池系统初始化
}

/*伙伴系统部分*/

acoral_block_ctr_t *acoral_mem_ctrl; ///< 内存控制块,只有一个
acoral_block_t *acoral_mem_blocks;	 ///< 这是一个数组，每个基本内存块对应一个

void buddy_scan()
{
	unsigned int i, k, n;
	unsigned int num_perlevel;
	unsigned int max_level = acoral_mem_ctrl->level;
	for (i = 0; i < max_level; i++)
	{
		printf("Level%d\r\n", i);
		printf("bitmap:");
		num_perlevel = acoral_mem_ctrl->num[i];
		for (k = 0; k < num_perlevel;)
		{
			for (n = 0; n < 8 && k < num_perlevel; n++, k++)
			{
				printf("%x ", acoral_mem_ctrl->bitmap[i][k]);
			}
			printf("\r\n");
		}
		printf("Free Block head:%d\r\n", acoral_mem_ctrl->free_cur[i]);
		printf("\r\n");
	}
	printf("Free Mem Block Number:%d\r\n", acoral_mem_ctrl->free_num);
	printf("\r\n");
}

unsigned int buddy_init(unsigned int start_adr, unsigned int end_adr)
{
	int i, k;
	unsigned int resize_size;
	unsigned int save_adr;
	unsigned int index;
	unsigned int num = 1;
	unsigned int adjust_level = 1;
	int level = 0;
	unsigned int max_num, o_num;
	unsigned int cur;
	start_adr += 3;
	start_adr &= ~(4 - 1); // 首地址四字节对齐
	end_adr &= ~(4 - 1);   // 尾地址四字节对齐
	resize_size = BASIC_BLOCK_SIZE;
	end_adr = end_adr - sizeof(acoral_block_ctr_t);	 // 减去内存控制块的大小，剩下的才是可分配内存
	end_adr &= ~(4 - 1);							 // 尾地址再进行一次四字节对齐
	acoral_mem_ctrl = (acoral_block_ctr_t *)end_adr; // 内存控制块的地址

	// 如果内存这么少，不适合分配
	if (start_adr > end_adr || end_adr - start_adr < BASIC_BLOCK_SIZE)
	{
		acoral_mem_ctrl->state = MEM_NO_ALLOC;
		return -1;
	}
	acoral_mem_ctrl->state = MEM_OK;

	while (1)
	{
		if (end_adr <= start_adr + resize_size)
			break;
		resize_size = resize_size << 1;
		num = num << 1; // 全分成基本内存块的数量
		adjust_level++;
	}
	acoral_mem_blocks = (acoral_block_t *)end_adr - num;
	save_adr = (unsigned int)acoral_mem_blocks;
	level = adjust_level; // 实际层数
	// 如果层数较小，则最大层用一块构成，如果层数较多，限制层数范围，最大层由多块构成
	if (adjust_level > LEVEL)
		level = LEVEL;
	num = num / 32; // 每32个基本内存块由一个32位位图表示
	for (i = 0; i < level - 1; i++)
	{
		num = num >> 1; // 除去最大层，其他每层的32位图都是64个块构成，所以要除以2
		if (num == 0)
		{
			num = 1; // 不足一个位图的，用一个位图表示
		}

		save_adr -= num * 4;								   // 每一个32位位图为4个字节
		save_adr &= ~(4 - 1);								   // 四字节对齐
		acoral_mem_ctrl->bitmap[i] = (unsigned int *)save_adr; // 当层bitmap地址
		acoral_mem_ctrl->num[i] = num;
		save_adr -= num * 4;							 // 每一个32位位图为4个字节
		save_adr &= ~(4 - 1);							 // 四字节对齐
		acoral_mem_ctrl->free_list[i] = (int *)save_adr; // 当层free_list地址
		for (k = 0; k < num; k++)
		{
			acoral_mem_ctrl->bitmap[i][k] = 0;	   // 初始化当层bitmap
			acoral_mem_ctrl->free_list[i][k] = -1; // 初始化当层free_list
		}
		acoral_mem_ctrl->free_cur[i] = -1; // 初始化当层free_cur
	}

	// 最大内存块层如果不足一个位图的，用一个位图表示
	if (num == 0)
	{
		num = 1;
	}
	save_adr -= num * 4;
	save_adr &= ~(4 - 1);
	acoral_mem_ctrl->bitmap[i] = (unsigned int *)save_adr;
	acoral_mem_ctrl->num[i] = num;
	save_adr -= num * 4;
	save_adr &= ~(4 - 1);
	acoral_mem_ctrl->free_list[i] = (int *)save_adr;
	for (k = 0; k < num; k++)
	{
		acoral_mem_ctrl->bitmap[i][k] = 0;
		;
		acoral_mem_ctrl->free_list[i][k] = -1;
	}
	acoral_mem_ctrl->free_cur[i] = -1;

	// 如果剩余内存大小不够形成现在的level
	if (save_adr <= (start_adr + (resize_size >> 1)))
		adjust_level--;
	if (adjust_level > LEVEL)
		level = LEVEL;

	////初始化内存控制块
	acoral_mem_ctrl->level = level;
	acoral_mem_ctrl->start_adr = start_adr;
	num = (save_adr - start_adr) >> BLOCK_SHIFT;
	acoral_mem_ctrl->end_adr = start_adr + (num << BLOCK_SHIFT);
	acoral_mem_ctrl->block_num = num;
	acoral_mem_ctrl->free_num = num;
	acoral_mem_ctrl->block_size = BASIC_BLOCK_SIZE;

	i = 0;
	max_num = (1 << level) - 1; // 最大内存块层的每块内存大小
	o_num = 0;

	if (num > 0)
	{ // 有内存块，则最大内存块层的free_cur为0
		acoral_mem_ctrl->free_cur[level - 1] = 0;
	}
	else
	{ // 无内存块，则最大内存块层的free_cur为-1
		acoral_mem_ctrl->free_cur[level - 1] = -1;
	}

	////整块内存优先分给最大内存块层
	// 计算当前可分配内存容量是否能直接形成一个最大内存块层的32位图
	while (num >= max_num * 32)
	{
		acoral_mem_ctrl->bitmap[level - 1][i] = -1;
		;
		acoral_mem_ctrl->free_list[level - 1][i] = i + 1;
		num -= max_num * 32;
		o_num += max_num * 32;
		i++;
	}
	if (num == 0)
	{ // 所有块正好分配到最大内存块层的32位图
		acoral_mem_ctrl->free_list[level - 1][i - 1] = -1;
	}

	// 计算当前可分配内存是否还能形成最大内存块层的一块
	while (num >= max_num)
	{
		index = (o_num >> level) - 1;
		acoral_set_bit(index, acoral_mem_ctrl->bitmap[level - 1]);
		num -= max_num;
		o_num += max_num;
	}
	acoral_mem_ctrl->free_list[level - 1][i] = -1;

	// 接下来的每层初始化
	while (--level > 0)
	{
		index = o_num >> level;
		if (num == 0)
			break;
		cur = index / 32;
		max_num = (1 << level) - 1; // 每层的内存块大小
		if (num >= max_num)
		{
			acoral_mem_blocks[BLOCK_INDEX(o_num)].level = -1;
			acoral_set_bit(index, acoral_mem_ctrl->bitmap[level - 1]);
			acoral_mem_ctrl->free_list[level - 1][cur] = -1;
			acoral_mem_ctrl->free_cur[level - 1] = cur;
			o_num += max_num;
			num -= max_num;
		}
	}
	return 0;
}

/**
 * @brief 迭代获取空闲块的首num
 *
 * @param level 要获取的层数，起始为0
 * @return int 空闲块的首num
 */
static int recus_malloc(int level)
{
	unsigned int index;
	int cur;
	int num;
	if (level >= acoral_mem_ctrl->level) // 层数超出范围
		return -1;
	cur = acoral_mem_ctrl->free_cur[level]; // 获取首个空闲位图
	if (cur < 0)							// 无空闲
	{
		num = recus_malloc(level + 1); // 迭代向上寻找
		if (num < 0)
			return -1;
		index = num >> level + 1;							   // 计算在位图中位置
		cur = index / 32;									   // 计算在位图链表中位置
		acoral_set_bit(index, acoral_mem_ctrl->bitmap[level]); // 在位图中置1，把偶数块分出去
		// 当前层无空闲，两个空闲块是从上层分配的，所以空闲位图链表更改，然后分配完一块还有一块，所以移动空闲位图链表头
		acoral_mem_ctrl->free_list[level][cur] = -1;
		acoral_mem_ctrl->free_cur[level] = cur;
		return num;
	}
	index = acoral_ffs(acoral_mem_ctrl->bitmap[level][cur]); // 获取空闲块在其32位图中的位置
	index = cur * 32 + index;								 // 计算空闲块实际位置
	acoral_clear_bit(index, acoral_mem_ctrl->bitmap[level]); // 从只有一块空闲变成两块都不空闲了，所以清0
	if (acoral_mem_ctrl->bitmap[level][cur] == 0)			 // 如果此位图无空闲块了
	{
		acoral_mem_ctrl->free_cur[level] = acoral_mem_ctrl->free_list[level][cur]; // 移动空闲位图链表头
	}
	num = index << level + 1; // 计算空闲块首个num
	/*最高level情况*/
	if (level == acoral_mem_ctrl->level - 1) // 最大内存块层
	{
		if ((num >> 1) + (1 << level) > acoral_mem_ctrl->block_num)
			return -1;
		return num >> 1;
	}
	// 其余层
	if (acoral_mem_blocks[BLOCK_INDEX(num)].level >= 0) // 检查这块内存有没有被分配
	{
		return num + (1 << level);
	} // 如果此块已经被分配，那就是后面一块为空闲块
	else
	{
		return num;
	}
}

/**
 * @brief 伙伴系统实际内存分配
 *
 * @param level 要分配的层数，起始为0
 * @return void* 返回分配的地址
 */
static void *r_malloc(unsigned char level)
{
	unsigned int index;
	int num, cur;
	acoral_enter_critical();
	acoral_mem_ctrl->free_num -= 1 << level; // 提前减去即将分配的基本内存块数
	cur = acoral_mem_ctrl->free_cur[level];
	if (cur < 0)
	{
		num = recus_malloc(level + 1);
		if (num < 0)
		{
			acoral_exit_critical();
			return NULL;
		}
		index = num >> level + 1;
		cur = index / 32;
		acoral_set_bit(index, acoral_mem_ctrl->bitmap[level]);
		acoral_mem_ctrl->free_list[level][cur] = -1;
		acoral_mem_ctrl->free_cur[level] = cur;
		if ((num & 0x1) == 0)
			acoral_mem_blocks[BLOCK_INDEX(num)].level = level;
#ifdef CFG_TEST_MEM
		buddy_scan();
#endif
		acoral_exit_critical();
		return (void *)(acoral_mem_ctrl->start_adr + (num << BLOCK_SHIFT));
	}
	index = acoral_ffs(acoral_mem_ctrl->bitmap[level][cur]);
	index = index + cur * 32;
	acoral_clear_bit(index, acoral_mem_ctrl->bitmap[level]);
	if (acoral_mem_ctrl->bitmap[level][cur] == 0)
	{
		acoral_mem_ctrl->free_cur[level] = acoral_mem_ctrl->free_list[level][cur];
	}
	if (level == acoral_mem_ctrl->level - 1)
	{
		num = index << level;
		if (num + (1 << level) > acoral_mem_ctrl->block_num)
		{
			acoral_exit_critical();
			return NULL;
		}
	}
	else
	{
		num = index << level + 1;
		if (acoral_mem_blocks[BLOCK_INDEX(num)].level >= 0)
			num += (1 << level);
	}
	if ((num & 0x1) == 0)
		acoral_mem_blocks[BLOCK_INDEX(num)].level = level;
#ifdef CFG_TEST_MEM
	buddy_scan();
#endif
	acoral_exit_critical();
	return (void *)(acoral_mem_ctrl->start_adr + (num << BLOCK_SHIFT));
}

unsigned int buddy_malloc_size(unsigned int size)
{
	unsigned int resize_size;
	unsigned char level = 0;
	unsigned int num = 1;
	resize_size = BASIC_BLOCK_SIZE;
	if (acoral_mem_ctrl->state == MEM_NO_ALLOC)
		return 0;
	while (resize_size < size && level < acoral_mem_ctrl->level)
	{
		num = num << 1;
		level++;
		resize_size = resize_size << 1;
	}
	return resize_size;
}

void *buddy_malloc(unsigned int size)
{
	unsigned int resize_size;
	unsigned char level = 0;
	unsigned int num = 1;
	resize_size = BASIC_BLOCK_SIZE;
	if (acoral_mem_ctrl->state == MEM_NO_ALLOC)
		return NULL;
	while (resize_size < size) // 本层块大小不满足申请内存
	{
		num = num << 1;
		level++;
		resize_size = resize_size << 1;
	}
	if (num > acoral_mem_ctrl->free_num) // 剩余内存不足
		return NULL;
	if (level >= acoral_mem_ctrl->level) // 申请内存块大小超过顶层内存块大小
		return NULL;
	return r_malloc(level); // 实际的分配函数
}

void buddy_free(void *ptr)
{
	unsigned char level;
	unsigned char buddy_level;
	int cur;
	unsigned int index;
	unsigned int num;
	unsigned int max_level;
	unsigned int adr;
	adr = (unsigned int)ptr;
	if (acoral_mem_ctrl->state == MEM_NO_ALLOC)
	{
		return;
	}

	// 无效地址
	if (ptr == NULL || adr < acoral_mem_ctrl->start_adr || adr + BASIC_BLOCK_SIZE > acoral_mem_ctrl->end_adr)
	{
		printf("Invalid Free Address:0x%x\n", (unsigned int)ptr);
		return;
	}
	max_level = acoral_mem_ctrl->level;						 // 记下层数
	num = (adr - acoral_mem_ctrl->start_adr) >> BLOCK_SHIFT; // 地址与基本块数换算
	// 如果不是block整数倍，肯定是非法地址
	if (adr != acoral_mem_ctrl->start_adr + (num << BLOCK_SHIFT))
	{
		printf("Invalid Free Address:0x%x\n", (unsigned int)ptr);
		return;
	}
	acoral_enter_critical();
	if (num & 0x1) // 奇数基本内存块
	{
		level = 0; // 奇数基本内存块一定是从0层分配
		// 下面是地址检查
		index = num >> 1;
		buddy_level = acoral_mem_blocks[BLOCK_INDEX(num)].level;

		// 不是从0层，直接返回错误
		if (buddy_level > 0)
		{
			printf("Invalid Free Address:0x%x\n", (unsigned int)ptr);
			acoral_exit_critical();
			return;
		}
		/*伙伴分配出去，如果对应的位为1,肯定是回收过一次了*/
		if (buddy_level == 0 && acoral_get_bit(index, acoral_mem_ctrl->bitmap[level]))
		{
			printf("Address:0x%x have been freed\n", (unsigned int)ptr);
			acoral_exit_critical();
			return;
		}
		/*伙伴没有分配出去了，如果对应的位为0,肯定是回收过一次了*/
		if (buddy_level < 0 && !acoral_get_bit(index, acoral_mem_ctrl->bitmap[level]))
		{
			printf("Address:0x%x have been freed\n", (unsigned)ptr);
			acoral_exit_critical();
			return;
		}
	}
	else
	{
		level = acoral_mem_blocks[BLOCK_INDEX(num)].level;
		/*已经释放*/
		if (level < 0)
		{
			printf("Address:0x%x have been freed\n", (unsigned int)ptr);
			acoral_exit_critical();
			return;
		}
		acoral_mem_ctrl->free_num += 1 << level;		// 空闲基本块数增加
		acoral_mem_blocks[BLOCK_INDEX(num)].level = -1; // 标志此基本块未被分配
	}
	if (level == max_level - 1) // 最大内存块直接回收
	{
		index = num >> level;								   // 最大内存块层，一块一位
		acoral_set_bit(index, acoral_mem_ctrl->bitmap[level]); // 标志空闲
		acoral_exit_critical();
		return;
	}
	index = (num >> 1) + level; // 其余层，两块一位
	while (level < max_level)	// 其余层回收，有可能回收到最大层
	{
		cur = index / 32;
		if (!acoral_get_bit(index, acoral_mem_ctrl->bitmap[level])) // 两块都没空闲
		{
			acoral_set_bit(index, acoral_mem_ctrl->bitmap[level]);								// 设置成有一块空闲
			if (acoral_mem_ctrl->free_cur[level] < 0 || cur < acoral_mem_ctrl->free_cur[level]) // 无空闲块或者释放的位比空闲位图链表头小
			{
				acoral_mem_ctrl->free_list[level][cur] = acoral_mem_ctrl->free_cur[level];
				acoral_mem_ctrl->free_cur[level] = cur;
			}
			break;
		}
		/*有个伙伴是空闲的，向上级回收*/
		acoral_clear_bit(index, acoral_mem_ctrl->bitmap[level]);
		if (cur == acoral_mem_ctrl->free_cur[level])
			acoral_mem_ctrl->free_cur[level] = acoral_mem_ctrl->free_list[level][cur];
		level++;
		if (level < max_level - 1)
			index = index >> 1;
	}
	acoral_exit_critical();
#ifdef CFG_TEST_MEM
	buddy_scan();
#endif
}

/*资源池部分*/

/// aCoral资源池数组，总共有ACORAL_MAX_POOLS=40个
acoral_pool_t acoral_pools[ACORAL_MAX_POOLS];

/// aCoral空闲资源池指针
acoral_pool_t *acoral_free_res_pool;

unsigned int acoral_create_pool(acoral_pool_ctrl_t *pool_ctrl)
{
	acoral_pool_t *pool;
	if (pool_ctrl->num >= pool_ctrl->max_pools)
		return ACORAL_RES_MAX_POOL;
	pool = acoral_get_free_pool();
	if (pool == NULL)
		return ACORAL_RES_NO_POOL;
	pool->id = pool_ctrl->type << ACORAL_RES_TYPE_BIT | pool->id;
	pool->size = pool_ctrl->size;
	pool->num = pool_ctrl->num_per_pool;
	pool->base_adr = (void *)acoral_malloc(pool->size * pool->num);
	if (pool->base_adr == NULL)
		return ACORAL_RES_NO_MEM;
	pool->res_free = pool->base_adr;
	pool->free_num = pool->num;
	pool->ctrl = pool_ctrl;
	acoral_pool_res_init(pool);
	acoral_list_add2_tail(&pool->ctrl_list, pool_ctrl->pools);
	acoral_list_add2_tail(&pool->free_list, pool_ctrl->free_pools);
	pool_ctrl->num++;
	return 0;
}

void acoral_release_pool(acoral_pool_ctrl_t *pool_ctrl)
{
	acoral_pool_t *pool;
	acoral_list_t *list, *head;
	head = pool_ctrl->pools;
	if (acoral_list_empty(head))
		return;
	for (list = head->next; list != head; list = list->next)
	{
		pool = list_entry(list, acoral_pool_t, free_list);
		acoral_list_del(&pool->ctrl_list);
		acoral_list_del(&pool->free_list);
		acoral_free(pool->base_adr);
		pool->base_adr = (void *)acoral_free_res_pool;
		pool->id = pool->id & ACORAL_POOL_INDEX_MASK;
		acoral_free_res_pool = pool;
	}
}

acoral_res_t *acoral_get_res(acoral_pool_ctrl_t *pool_ctrl)
{
	acoral_list_t *first;
	acoral_res_t *res;
	acoral_pool_t *pool;
	acoral_enter_critical();
	first = pool_ctrl->free_pools->next;
	if (acoral_list_empty(first))
	{
		if (acoral_create_pool(pool_ctrl))
		{
			acoral_exit_critical();
			return NULL;
		}
		else
		{
			first = pool_ctrl->free_pools->next;
		}
	}
	pool = list_entry(first, acoral_pool_t, free_list);
	res = (acoral_res_t *)pool->res_free;
	pool->res_free = (void *)((unsigned char *)pool->base_adr + res->next_id * pool->size);
	res->id = (res->id >> (ACORAL_RES_INDEX_INIT_BIT - ACORAL_RES_INDEX_BIT)) & ACORAL_RES_INDEX_MASK | pool->id;
	pool->free_num--;
	if (!pool->free_num)
	{
		acoral_list_del(&pool->free_list);
	}
	acoral_exit_critical();
	return res;
}

void acoral_release_res(acoral_res_t *res)
{
	acoral_pool_t *pool;
	unsigned int index;
	void *tmp;
	acoral_pool_ctrl_t *pool_ctrl;
	if (res == NULL || acoral_get_res_by_id(res->id) != res)
	{
		return;
	}
	pool = acoral_get_pool_by_id(res->id);
	if (pool == NULL)
	{
		printf("Res release Err\n"); // TODO 删printerr
		return;
	}
	pool_ctrl = pool->ctrl;
	if ((void *)res < pool->base_adr)
	{
		printf("Err Res\n");
		return;
	}
	index = (((unsigned int)res - (unsigned int)pool->base_adr) / pool->size);
	if (index >= pool->num)
	{
		printf("Err Res\n");
		return;
	}
	tmp = pool->res_free;
	pool->res_free = (void *)res;
	res->id = index << ACORAL_RES_INDEX_INIT_BIT;
	res->next_id = ((acoral_res_t *)tmp)->id >> ACORAL_RES_INDEX_INIT_BIT;
	pool->free_num++;
	if (acoral_list_empty(&pool->free_list))
		acoral_list_add(&pool->free_list, pool_ctrl->free_pools);
	return;
}

acoral_pool_t *acoral_get_pool_by_id(int res_id)
{
	unsigned int index;
	index = (res_id & ACORAL_POOL_INDEX_MASK) >> ACORAL_POOL_INDEX_BIT;
	if (index < ACORAL_MAX_POOLS)
		return acoral_pools + index;
	return NULL;
}

acoral_pool_t *acoral_get_free_pool()
{
	acoral_pool_t *tmp;
	acoral_enter_critical();
	tmp = acoral_free_res_pool;
	if (NULL != tmp)
	{
		acoral_free_res_pool = *(void **)tmp->base_adr;
	}
	acoral_exit_critical();
	return tmp;
}

acoral_res_t *acoral_get_res_by_id(int id)
{
	acoral_pool_t *pool;
	unsigned int index;
	pool = acoral_get_pool_by_id(id);
	if (pool == NULL)
		return NULL;
	index = (id & ACORAL_RES_INDEX_MASK) >> ACORAL_RES_INDEX_BIT;
	return (acoral_res_t *)((unsigned char *)pool->base_adr + index * pool->size);
}

void acoral_pool_res_init(acoral_pool_t *pool)
{
	acoral_res_t *res;
	unsigned int i;
	unsigned char *pblk;
	unsigned int blks;
	blks = pool->num;
	res = (acoral_res_t *)pool->base_adr;
	pblk = (unsigned char *)pool->base_adr + pool->size;
	for (i = 0; i < (blks - 1); i++)
	{
		res->id = i << ACORAL_RES_INDEX_INIT_BIT;
		res->next_id = i + 1;
		res = (acoral_res_t *)pblk;
		pblk += pool->size;
	}
	res->id = blks - 1 << ACORAL_RES_INDEX_INIT_BIT;
	res->next_id = 0;
}

void acoral_pool_ctrl_init(acoral_pool_ctrl_t *pool_ctrl)
{
	unsigned int size;
	pool_ctrl->free_pools = &pool_ctrl->list[0];
	pool_ctrl->pools = &pool_ctrl->list[1];
	pool_ctrl->num = 0;
	acoral_init_list(pool_ctrl->pools);
	acoral_init_list(pool_ctrl->free_pools);
	/*调整pool的对象个数以最大化利用分配了的内存*/
	size = acoral_malloc_adjust_size(pool_ctrl->size * pool_ctrl->num_per_pool);
	if (size < pool_ctrl->size)
	{
		pool_ctrl->num_per_pool = 0;
	}
	else
	{
		pool_ctrl->num_per_pool = size / pool_ctrl->size;
		acoral_create_pool(pool_ctrl); // 先创建一个资源池，后面如果一个池子不够了，那在不超过这类资源的max_pool的条件下再创建新的池子
	}
}

void acoral_res_sys_init()
{
	acoral_pool_t *pool;
	unsigned int i;
	pool = &acoral_pools[0];
	for (i = 0; i < (ACORAL_MAX_POOLS - 1); i++)
	{
		pool->base_adr = (void *)&acoral_pools[i + 1];
		pool->id = i;
		pool++;
	}
	pool->base_adr = (void *)0;
	acoral_free_res_pool = &acoral_pools[0];
}

//SPG原malloc.c

/**
 * @file malloc.c
 * @author 王彬浩 (SPGGOGOGO@outlook.com)
 * @brief kernel层，内存malloc
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

#include "mutex.h"
#include "mem.h"
#include "thread.h"
#include <stdio.h>

struct mem2_ctrl_t
{
	acoral_evt_t mutex;
	char *top_p;
	char *down_p;
	unsigned int *freep_p;
	unsigned char mem_state;
} mem_ctrl;

static void *real_malloc(int size)
{
	unsigned int *tp;
	char *ctp;
	unsigned int b_size;
	size = size + 4;
	while (acoral_mutex_pend(&mem_ctrl.mutex, 0) != MUTEX_SUCCED)
		acoral_suspend_self();

	tp = mem_ctrl.freep_p;
	ctp = (char *)tp;
	while (ctp < mem_ctrl.top_p)
	{
		b_size = BLOCK_SIZE(*tp);
		if (b_size == 0)
		{
			printf("Err address is 0x%x,size should not be 0", (unsigned int)tp);
			acoral_mutex_post(&mem_ctrl.mutex);
			return NULL;
		}
		if (BLOCK_USED(*tp) || b_size < size)
		{
			ctp = ctp + b_size;
			tp = (unsigned int *)ctp;
		}
		else
		{
			BLOCK_SET_USED(tp, size);
			ctp = ctp + size;
			tp = (unsigned int *)ctp;
			if (b_size - size > 0)
				BLOCK_SET_FREE(tp, b_size - size);
			mem_ctrl.freep_p = tp;

			acoral_mutex_post(&mem_ctrl.mutex);
			return (void *)(ctp - size + 4);
		}
	}
	ctp = mem_ctrl.down_p;
	tp = (unsigned int *)ctp;
	while (tp < mem_ctrl.freep_p)
	{
		b_size = BLOCK_SIZE(*tp);
		if (b_size == 0)
		{
			printf("Err address is 0x%x,size should not be 0", (unsigned int)tp);
			acoral_mutex_post(&mem_ctrl.mutex);
			return NULL;
		}
		if (BLOCK_USED(*tp) || b_size < size)
		{
			ctp = ctp + b_size;
			tp = (unsigned int *)ctp;
		}
		else
		{
			BLOCK_SET_USED(tp, size);
			ctp = ctp + size;
			tp = (unsigned int *)ctp;
			if (b_size - size > 0)
				BLOCK_SET_FREE(tp, b_size - size);
			mem_ctrl.freep_p = tp;

			acoral_mutex_post(&mem_ctrl.mutex);
			return (void *)(ctp - size + 4);
		}
	}
	acoral_mutex_post(&mem_ctrl.mutex);
	return NULL;
}

void *v_malloc(int size)
{
	if (mem_ctrl.mem_state == 0)
		return NULL;
	size = (size + 3) & ~3;
	return real_malloc(size);
}

void v_free(void *p)
{
	unsigned int *tp, *prev_tp;
	char *ctp;
	unsigned int b_size, size = 0;
	if (mem_ctrl.mem_state == 0)
		return;
	p = (char *)p - 4;
	tp = (unsigned int *)p;
	while (acoral_mutex_pend(&mem_ctrl.mutex, 0) != 0) // 周期性任务
		acoral_suspend_self();
	if (p == NULL || (char *)p < mem_ctrl.down_p || (char *)p >= mem_ctrl.top_p || !BLOCK_CHECK(*tp))
	{
		printf("Invalide Free address:0x%x\n", (unsigned int)tp);
		return;
	}
	if (BLOCK_FREE(*tp))
	{
		printf("Address:0x%x have been freed\n", (unsigned int)tp);
		return;
	}
	prev_tp = tp;
	ctp = (char *)tp;
	b_size = BLOCK_SIZE(*tp);

	ctp = ctp + b_size;
	tp = (unsigned int *)ctp;
	if (BLOCK_FREE(*tp))
	{
		size = BLOCK_SIZE(*tp);
		if (size == 0)
		{
			printf("Err address is 0x%x,size should not be 0", (unsigned int)tp);
			acoral_mutex_post(&mem_ctrl.mutex);
			return;
		}
		b_size += size;
		BLOCK_CLEAR(tp);
	}
	BLOCK_SET_FREE(prev_tp, b_size);
	mem_ctrl.freep_p = prev_tp;
	if (p == mem_ctrl.down_p)
	{

		acoral_mutex_post(&mem_ctrl.mutex);
		return;
	}
	ctp = mem_ctrl.down_p;
	tp = (unsigned int *)ctp;
	while (ctp < (char *)p)
	{
		size = BLOCK_SIZE(*tp);
		if (size == 0)
		{
			printf("err address is 0x%x,size should not be 0", (unsigned int)tp);
			acoral_mutex_post(&mem_ctrl.mutex);
			return;
		}
		ctp = ctp + size;
		prev_tp = tp;
		tp = (unsigned int *)ctp;
	}
	if (BLOCK_FREE(*prev_tp))
	{
		tp = (unsigned int *)p;
		BLOCK_CLEAR(tp);
		BLOCK_SET_FREE(prev_tp, b_size + size);
		mem_ctrl.freep_p = prev_tp;
	}

	acoral_mutex_post(&mem_ctrl.mutex);
}

void v_mem_init()
{
	unsigned int size;
	size = acoral_malloc_adjust_size(CFG_MEM2_SIZE);
	mem_ctrl.down_p = (char *)acoral_malloc(size);
	if (mem_ctrl.down_p == NULL)
	{
		mem_ctrl.mem_state = 0;
		return;
	}
	else
	{
		mem_ctrl.mem_state = 1;
	}
	acoral_mutex_init(&mem_ctrl.mutex, 0);
	mem_ctrl.top_p = mem_ctrl.down_p + size;
	mem_ctrl.freep_p = (unsigned int *)mem_ctrl.down_p;
	BLOCK_SET_FREE(mem_ctrl.freep_p, size);
}

void v_mem_scan(void)
{
	char *ctp;
	unsigned int *tp;
	unsigned int size;
	if (mem_ctrl.mem_state == 0)
	{
		printf("Mem Init Err ,so no mem space to malloc\r\n");
		return;
	}
	ctp = mem_ctrl.down_p;
	do
	{
		tp = (unsigned int *)ctp;
		size = BLOCK_SIZE(*tp);
		if (size == 0)
		{
			printf("Err address is 0x%x,size should not be 0\r\n", (unsigned int)tp);
			break;
		}
		if (BLOCK_USED(*tp))
		{
			printf("The address is 0x%x,the block is used and it's size is %d\r\n", (unsigned int)tp, size);
		}
		else
		{

			printf("The address is 0x%x,the block is unused and it's size is %d\r\n", (unsigned int)tp, size);
		}
		ctp = ctp + size;
	} while (ctp < mem_ctrl.top_p);
}

