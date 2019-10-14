#include "threadpool.h"

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
        request->processer();
    }
}

