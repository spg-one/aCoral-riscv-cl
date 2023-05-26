# aCoral-I User Programming API

## 关于每个API的详细使用方法，参见doxygen文档
---
## 内存相关
* acoral_malloc 伙伴系统分配内存
* acoral_free 伙伴系统回收内存
* acoral_malloc_adjust_size 用户指定的内存大小不一定合适，可以先用这个函数进行一下调整
* acoral_malloc2 任意大小内存分配
* acoral_free2 任意大小内存释放
* acoral_get_res 获取某一类型的资源（tcb、event等）
* acoral_release_res 释放某一资源
* acoral_get_res_by_id 根据id获取某一资源
---
## 线程相关
* acoral_create_thread 创建线程
* acoral_suspend_self 挂起当前线程
* acoral_suspend_thread_by_id 挂起某个线程
* acoral_resume_thread_by_id 唤醒某个线程
* acoral_delay_self 将当前线程延时
* acoral_kill_thread_by_id 干掉某个线程
* acoral_thread_exit 结束当前线程
* acoral_change_prio_self 改变当前线程优先级
* acoral_thread_change_prio_by_id 改变某个线程优先级
---
## 调度策略相关
* acoral_register_sched_policy 向aCoral中注册新的调度策略
---
## 中断相关
* acoral_intr_enable aCoral全局中断打开
* acoral_intr_disable aCoral全局中断关闭
* acoral_enter_critical aCoral进入临界区（本质就是关中断）
* acoral_exit_critical aCoral退出临界区（本质就是开中断）
* acoral_intr_attach 给某个plic中断绑定中断服务函数
* acoral_intr_detach 给某个plic中断解绑中断服务函数
* acoral_intr_unmask 使能某个中断
* acoral_intr_mask 除能某个中断
---
## 信号量相关
* acoral_sem_init 初始化信号量
* acoral_sem_create 创建并初始化信号量
* acoral_sem_del 删除信号量
* acoral_sem_trypend 获取信号量(非阻塞)
* acoral_sem_pend 获取信号量(阻塞式)
* acoral_sem_post 释放信号量
* acoral_sem_getnum 得到当前信号量值
---
## 互斥量相关
* acoral_mutex_init 初始化互斥量
* acoral_mutex_create 创建并初始化互斥量
* acoral_mutex_del 删除互斥量
* acoral_mutex_trypend 获取互斥量（非阻塞式）
* acoral_mutex_pend 获取互斥量（优先级继承的优先级反转解决）
* acoral_mutex_pend2 获取互斥量（优先级天花板的优先级反转解决）
* acoral_mutex_post 释放互斥量
---
## 消息相关
* acoral_msgctr_create 创建消息容器
* acoral_msg_create 创建消息
* acoral_msg_send 发送消息
* acoral_msg_recv 接收消息
* acoral_msgctr_del 删除消息容器
* acoral_msg_del 删除消息
* acoral_print_all_msg 打印消息容器上全部消息，不会消耗消息的count
---
## ticks相关
* acoral_set_ticks 设置aCoral心跳tick的值
* acoral_get_ticks 得到tick的值
---
## shell相关
* add_command 添加shell命令