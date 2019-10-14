// 半同步半反应堆线程池的实现
// 适用与聊天室服务器的构建
#ifndef THREADPOOL_H
#define THREADPOOL_H

#include <list>
#include <cstdio>
#include <exception>
#include <pthread.h>
#include "locker.h"

/* 线程池类*/
template< typename T >
class threadpool
{
public:
    threadpool( int thread_number = 8, int max_requests = 10000 );
    ~threadpool();
    bool append( T* request ); // 请求队列中添加任务

private:
    static void* worker( void* arg ); // 工作线程运行的函数
    void run();

private:
    int m_thread_number; // 线程池中的线程数
    int m_max_requests;  // 请求队列允许的最大请求数
    pthread_t* m_threads; // 线程组数组
    std::list< T* > m_workqueue; // 请求队列 (模板体现这里) 使用List插入删除快 但是随机访问慢
    locker m_queuelocker; // 保护请求队列的互斥量 
    sem m_queuestat;  // 是否有任务需要处理
    bool m_stop; // 是否结束线程
};





// template< typename T >
// void threadpool< T >::run()
// {
//     while ( ! m_stop )
//     {
//         m_queuestat.wait();
//         m_queuelocker.lock();
//         if ( m_workqueue.empty() )
//         {
//             m_queuelocker.unlock();
//             continue;
//         }
//         T* request = m_workqueue.front();
//         m_workqueue.pop_front();
//         m_queuelocker.unlock();
//         if ( ! request )
//         {
//             continue;
//         }
//         request->process();
//     }
// }

// #endif
#endif