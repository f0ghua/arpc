#ifndef THREADBASE_H
#define THREADBASE_H

#include "pthread.h"

class ThreadBase
{
protected:
    pthread_t _tid;
    static void* run0(void* opt);
    void* run1();

public:
    ThreadBase();
    ~ThreadBase();

    bool start();
    void join();
    virtual void run() {}
};


#endif
