#include "ThreadBase.h"

ThreadBase::ThreadBase()
{    
}

ThreadBase::~ThreadBase()
{    
}

void *ThreadBase::run0(void *opt)
{
    ThreadBase *p = (ThreadBase *) opt;
    p->run1();
    return p;
}

void *ThreadBase::run1()
{
    _tid = pthread_self();
    run();
    _tid = 0;
    pthread_exit(NULL);
}

bool ThreadBase::start()
{
    return pthread_create(&_tid, NULL, run0, this) == 0;
}

void ThreadBase::join()
{
    if( _tid > 0 ){
        pthread_join(_tid, NULL);
    }
}
