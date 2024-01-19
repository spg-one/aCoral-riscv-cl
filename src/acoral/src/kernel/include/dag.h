#ifndef DAG_H
#define DAG_H

#include "thread.h"

#define ENABLE_DAG 1
#define DAG_SIZE 10

typedef enum{
    ACORAL_DAG_NODE_FULL= -10,
    ACORAL_DAG_NODE_THREAD_NULL,
    ACORAL_DAG_EDGE_NULL,
    ACORAL_DAG_EDGE_SUCCESS
}acoralDagEnum;

typedef struct node_struct
{
    int tid;
    acoral_thread_t* tcb;
    int former_task_num;
}node;

typedef struct dag_struct
{
    node nodes[DAG_SIZE];
    int edges[DAG_SIZE][DAG_SIZE];
    int num;
}DAG;

/**
 * @brief 
 * 
 * @param tid 线程id 
 * @param input 
 * @param output 
 * @return int DAG节点号
 */
int dag_add_node(int tid,void* input, void* output);

/**
 * @brief 
 * 
 * @param node DAG节点号
 * @return int 
 */
int dag_delete_node(int node);

/**
 * @brief 
 * 
 * @param start 起始节点
 * @param end 终止节点
 * @return int 
 */
int dag_add_edge(int start, int end);

/**
 * @brief 
 * 
 * @param start 起始节点
 * @param end 终止节点
 * @return int 
 */
int dag_delete_edge(int start, int end);

int dag_init();

int dag_check_cycle();

int acoral_dag_thread_exit();

#endif