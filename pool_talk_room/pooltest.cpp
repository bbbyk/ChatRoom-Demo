#include "threadpool.h"
#include <unistd.h>
#include <iostream>
#include <sys/select.h>

void block_sec(int sec, int usec)
{
    timeval sleep_sec;
    sleep_sec.tv_sec = sec; 
    sleep_sec.tv_usec = usec;
    select(0, NULL, NULL, NULL, &sleep_sec);
}

class event
{
private:
    int index;
public:
    event(){};
    // event(int sec, int usec, int in) {sleep_sec.tv_sec = sec; sleep_sec.tv_usec = 0;index = 0;}
    ~event() { };
    void setIndex(int i) {index = i;}
    void processor() { block_sec(3, 0); 
                        std::cout << "event" << index << "block 3s" << std::endl;};
};

int main(int argc, char *argv[])
{
    // threadpool<event>* pool1;
    threadpool<event>* pool = new threadpool<event>(4, 20);
    event* m_event = new event[30]();
    for (int i = 0; i < 10; i++) {
         std::cout << "add " << i << " event to list" << std::endl;
         m_event[i].setIndex(i);
         pool->append(&m_event[i]);
    }
    for (int i = 10; i < 30; i++)
    {
        //初始化event
        std::cout << "add " << i << " event to list" << std::endl;
        m_event[i].setIndex(i);
        pool->append(&m_event[i]);
        block_sec(2, 0);
    }
    return 0;
}

