/**
 * @file shell.c
 * @author 王彬浩 (SPGGOGOGO@outlook.com)
 * @brief kernel层，aCoral shell
 * @version 1.0
 * @date 2023-05-08
 * @copyright Copyright (c) 2023
 * @revisionHistory
 *  <table>
 *   <tr><th> 版本 <th>作者 <th>日期 <th>修改内容
 *   <tr><td> 0.1 <td>jivin <td> 2010-03-08 <td>Created
 *   <tr><td> 1.0 <td>王彬浩 <td> 2023-05-08 <td>Standardized
 *  </table>
 */

#include<acoral.h>
#include "shell.h"
#include "thread.h"
#include <stdio.h>
#include <string.h>

#define BUF_SIZE 128
#define MAX_ARGS_NUM 8
acoral_shell_cmd_t *head_cmd,*tail_cmd;

void parse_args(char *argstr, int *argc_p, char **argv, char** resid, parse_state *stacked)
{
	int argc = 0;
	char c;
	parse_state newState;
	parse_state stacked_state=*stacked;
	parse_state lastState = PS_WHITESPACE;

	while ((c = *argstr) != 0&&argc<MAX_ARGS_NUM) {

		if (c == ';' && lastState != PS_STRING && lastState != PS_ESCAPE)
			break;

		if (lastState == PS_ESCAPE) {
			newState = stacked_state;
		} else if (lastState == PS_STRING) {
			if (c == '"') {
				newState = PS_WHITESPACE;
				*argstr = 0;
			} else {
				newState = PS_STRING;
			}
		} else if ((c == ' ') || (c == '\t')) {
			*argstr = 0;
			newState = PS_WHITESPACE;
		} else if (c == '"') {
			newState = PS_STRING;
			*argstr++ = 0;
			argv[argc++] = argstr;
		} else if (c == '\\') {
			stacked_state = lastState;
			newState = PS_ESCAPE;
		} else {
			if (lastState == PS_WHITESPACE) {
				argv[argc++] = argstr;
			}
			newState = PS_TOKEN;
		}

		lastState = newState;
		argstr++;
	}

	argv[argc] = NULL;
	if (argc_p != NULL)
		*argc_p = argc;

	if (*argstr == ';') {
		*argstr++ = '\0';
	}
	*resid = argstr;
	*stacked=stacked_state;
}

void add_command(acoral_shell_cmd_t *cmd)
{
	if (head_cmd == NULL) {
		head_cmd = tail_cmd = cmd;
	} else {
		tail_cmd->next = cmd;
		tail_cmd = cmd;
	}
}

acoral_shell_cmd_t *find_cmd(const char *cmdname)
{
	acoral_shell_cmd_t *curr;
	curr =head_cmd;
	while(curr != NULL) {
		if (strcmp(curr->name, cmdname) == 0)
			return curr;
		curr = curr->next;
	}
	return NULL;
}

void execmd(int argc,const char **argv)
{
	acoral_shell_cmd_t *cmd = find_cmd(argv[0]);
	if (cmd == NULL) {
		printf("Could not found '%s' command\r\n", argv[0]);
		printf("you can type 'help'\r\n"); 
		return;
	}
	cmd->exe(argc, argv);
}


void cmd_exe(char *buf){
	int argc;
	char *argv[MAX_ARGS_NUM];
	char *resid;
	parse_state stacked_state;
	while (*buf) {
		memset(argv, 0, sizeof(argv));
		parse_args(buf, &argc, argv, &resid,&stacked_state);
		if (argc > 0)
			execmd(argc, (const char **)argv);//TODO 保留const？
		buf = resid;
	}	
}

#define SHELL_STACK_SIZE 1024
void acoral_shell_init(void){
	acoral_comm_policy_data_t data;
	head_cmd=NULL;
	tail_cmd=NULL;
	cmd_init();

	data.prio=ACORAL_NONHARD_RT_PRIO_MIN;
	data.prio_type=ACORAL_HARD_PRIO;
	acoral_create_thread(acoral_shell_enter,SHELL_STACK_SIZE,NULL,"shell",NULL,ACORAL_SCHED_POLICY_COMM,&data);
}


void acoral_shell_enter(void *args){
	char *cmd_buf;
	cmd_buf=acoral_malloc(sizeof(BUF_SIZE));
	while(1){
		printf("\r\n");
		printf("aCoral:>\n");
		gets(cmd_buf);
		if(cmd_buf[0])
			cmd_exe(cmd_buf);
	}
}
