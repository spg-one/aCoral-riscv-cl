#include "dag.h"

acoral_dag_t dag_global;

int dag_init(){
    for(int i = 0;i<CFG_DAG_SIZE;i++){
        for(int j = 0;j<CFG_DAG_SIZE;j++){
            dag_global.edges[i][j] = 0;
        }
    }
    dag_global.node_num = 0;
}

int dag_add_node(void (*route)(void *args),int core_id, void* input, void* output){
    // if(dag_global.num>=10){
    //     return ACORAL_DAG_NODE_FULL;
    // }
    // acoral_create_thread(func,1024,input,"DAG##")
    // acoral_thread_t* tcb = (acoral_thread_t *)acoral_get_res_by_id(tid);
    // if(tcb == NULL){
    //     return ACORAL_DAG_NODE_THREAD_NULL;
    // }
    // dag_global.nodes[dag_global.num] = (node){
    //     tid,
    //     tcb,
    //     0
    // };
    // return dag_global.num++;
}

int dag_add_edge(int start, int end){
    if((start<0)||(start>=CFG_DAG_SIZE)||(end<0)||(end>=CFG_DAG_SIZE)){
        return ACORAL_DAG_EDGE_NULL;
    }
    dag_global.edges[start][end] = 1;
    return ACORAL_DAG_EDGE_SUCCESS;
}

int acoral_dag_thread_exit(){
    acoral_suspend_self();

}