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

template< typename T>
threadpool<T>::threadpool(int thread_number, int max_request) :
                m_thread_number(thread_number), m_max_requests(max_request)
{
    if (m_thread_number <=0 || m_max_requests <=0) {
        throw std::exception();
    }
    m_threads = new pthread_t[m_thread_number];
    if (!m_threads) {
        throw std::exception();
    }

    // 创建线程并detach
    for (int i = 0; i < m_thread_number; i++)
    {
        printf("Create the %d thread \n", i);
        if ( pthread_create(m_threads + i, NULL, worker, this) != 0) {
            delete [] m_threads;
            throw std::exception();
        }
        if (pthread_detach( m_threads[i]) != 0) {
            delete [] m_threads;
            throw std::exception();
        }
    }
}

template< typename T>
threadpool<T>::~threadpool()
{
    delete [] m_threads;
    m_stop = true;
}

template< typename T >
bool threadpool<T>::append(T* request)
{
    // 主要一定要加锁
    m_queuelocker.lock();
    if (m_workqueue.size() < m_max_requests) {
        m_workqueue.push_back(request);
        m_queuestat.post();
    } else {
        m_queuelocker.unlock();
        return false;
    }
    m_queuelocker.unlock();
    return true;
}

template< typename T>
void* threadpool<T>::worker(void *arg)
{
    auto* pool = (threadpool*) arg;
    pool->run();
    return pool;
}

template< typename T>
void threadpool< T >::run()
{
    // 
    while ( ! m_stop)
    {
        // 从任务队列中获取request
        m_queuestat.wait();
        m_queuelocker.lock();
        if (m_workqueue.empty()) {
            m_queuelocker.unlock();
            continue;
        }
        auto request = m_workqueue.front();
        m_workqueue.pop_front();
        m_queuelocker.unlock();
        // request中的processor是我们实际上要去完成的
        if (!request)
            continue;
        request->processor();
    }
}

#endif