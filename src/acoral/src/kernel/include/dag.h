#ifndef DAG_H
#define DAG_H

#include "thread.h"



typedef enum{
    ACORAL_DAG_NODE_FULL= -10,
    ACORAL_DAG_NODE_THREAD_NULL,
    ACORAL_DAG_EDGE_NULL,
    ACORAL_DAG_EDGE_SUCCESS
}acoralDagEnum;

typedef struct node_struct
{
    int tid;                        ///<DAG节点对应的线程id
    acoral_thread_t* tcb;           ///<DAG节点对应的线程
    int former_task_num;            ///<DAG节点剩余未完成的前驱节点数
    int former_task_num_origin;     ///<DAG节点前驱节点数
}node;

typedef struct dag_struct
{
    node nodes[CFG_DAG_SIZE];               ///<DAG节点
    int edges[CFG_DAG_SIZE][CFG_DAG_SIZE];  ///<DAG边
    int num;                                ///<既表示系统中DAG节点数量，也作为节点编号使用
}acoral_dag_t;

/**
 * @brief 往dag_global中添加一个DAG节点，并创建对应的线程
 * 
 * @param route 线程函数
 * @param input 
 * @param output 
 * @return int DAG节点号
 */
int dag_add_node(void (*route)(void *args),void* input, void* output);

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