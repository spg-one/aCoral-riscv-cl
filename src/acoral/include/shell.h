/**
 * @file shell.h
 * @author 王彬浩 (SPGGOGOGO@outlook.com)
 * @brief kernel层，定时器相关头文件
 * @version 1.0
 * @date 2022-07-20
 * @copyright Copyright (c) 2023
 * @revisionHistory 
 *  <table> 
 *   <tr><th> 版本 <th>作者 <th>日期 <th>修改内容 
 *   <tr><td> 0.1 <td>jivin <td>2010-03-08 <td>Created 
 *   <tr><td> 1.0 <td>王彬浩 <td> 2022-07-20 <td>Standardized 
 *  </table>
 */

#ifndef SHELL_H
#define SHELL_H

typedef enum{
	PS_WHITESPACE,
	PS_TOKEN,
	PS_STRING,
	PS_ESCAPE
}parse_state;

typedef struct acoral_shell_cmd_t acoral_shell_cmd_t;

/**
 * @struct acoral_shell_cmd_t
 * @brief aCoral shell命令结构体
 * 
 */
struct acoral_shell_cmd_t{
	char *name;							 ///<命令的名字（shell中输入的那个）
	void (*exe)(int argc,const char **); ///<命令函数指针
	char *comment;					 	 ///<命令的解释
	acoral_shell_cmd_t *next;			 ///<创建的时候置位NULL就好
};

void acoral_shell_init(void);
void acoral_shell_enter(void *args);
void cmd_init(void);

/***************shell API****************/

/**
 * @brief 添加shell命令，需要在cmd_init函数中调用
 * 
 * @param cmd shell命令结构体
 */
void add_command(acoral_shell_cmd_t *cmd);

#endif
