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
    int nid;                      ///<DAG节点id，用于全局DAG图中的节点数组索引
    int tid;                        ///<DAG节点对应的线程id
    acoral_thread_t* tcb;           ///<DAG节点对应的线程
    int former_task_num;            ///<DAG节点剩余未完成的前驱节点数
    int former_task_num_origin;     ///<DAG节点前驱节点数
    int core_id;                    ///<DAG节点运行的目标核
    //spinlock
}acoral_dag_node;

typedef struct dag_struct
{
    acoral_dag_node nodes[CFG_DAG_SIZE];         ///<DAG节点
    int edges[CFG_DAG_SIZE][CFG_DAG_SIZE];       ///<DAG边
    int node_num;                                ///<既表示系统中DAG节点数量
    //spinlock
    //加一个最近完成的节点编号用于核间中断传参
}acoral_dag_t;

/**
 * @brief 往dag_global中添加一个DAG节点，并创建对应的线程
 * 
 * @param route 线程函数
 * @param core_id 
 * @param input 
 * @param output 
 * @return int DAG节点号
 */
int dag_add_node(void (*route)(void *args), int core_id, void* input, void* output);

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

int dag_start();

int dag_check_cycle();

int acoral_dag_thread_exit();

#endif