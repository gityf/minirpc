/*
** Copyright (C) 2014 Wang Yaofu
** All rights reserved.
**
**Author:Wang Yaofu voipman@qq.com
**Description: The source file of class CThread,CMutex and CThreadWatcher.
*/

#include <unistd.h>
#include <cerrno>
#include <string>
#include "common/cthread.h"
#include "common/clogwriter.h"
using std::string;

CMutex::CMutex() {
    pthread_mutex_init(&m,NULL);
}

CMutex::~CMutex() {
    pthread_mutex_destroy(&m);
}

void CMutex::lock() {
    pthread_mutex_lock(&m);
}

void CMutex::unlock() {
    pthread_mutex_unlock(&m);
}

CThread::CThread()
: _stopped(true) , _pid(0) {
}

// int thread_nr=0;
// CMutex thread_nr_mut;

void * CThread::_start(void * _t) {
    CThread* _this = (CThread*)_t;
    _this->_pid = (unsigned long) _this->_td;
    LOG(LL_INFO, "CThread::Thread %lu is starting.",
        (unsigned long) _this->_pid);
    _this->_stopped.set(false);
    _this->run();

    LOG(LL_INFO, "CThread::Thread %lu is ending.",
        (unsigned long) _this->_pid);
    _this->_stopped.set(true);

    //thread_nr_mut.lock();
    //INFO("threads = %i\n",--thread_nr);
    //thread_nr_mut.unlock();

    return NULL;
}

void CThread::start(bool realtime) {
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setstacksize(&attr,1024*1024*2);// 1 MB
    pthread_attr_setdetachstate(&attr,PTHREAD_CREATE_DETACHED);

    int res;
    _pid = 0;
    // unless placed here, a call seq like run(); join(); will not wait to join
    // b/c creating the thread can take too long
    this->_stopped.set(false);
    res = pthread_create(&_td,&attr,_start,this);
    pthread_attr_destroy(&attr);
    if (res != 0) {
        LOG(LL_ERROR, "CThread::start():pthread create failed with code %i", res);
        throw string("CThread::start():thread could not be started");
    }
}

void CThread::stop() {
    _m_td.lock();

    if(is_stopped()){
        _m_td.unlock();
        return;
    }

    // gives the thread a chance to clean up
    LOG(LL_INFO, "CThread::stop():Thread %lu (%lu) calling on_stop, give it a chance to clean up.",
        (unsigned long int) _pid, (unsigned long int) _td);

    try { on_stop(); } catch(...) {}

    int res;
    if ((res = pthread_detach(_td)) != 0) {
        if (res == EINVAL) {
            LOG(LL_ERROR, "CThread::stop():pthread_detach failed with code EINVAL: thread already in detached state.");
        } else if (res == ESRCH) {
            LOG(LL_ERROR, "CThread::stop():pthread_detach failed with code ESRCH: thread could not be found.");
        } else {
            LOG(LL_ERROR, "CThread::stop():pthread_detach failed with code %i", res);
        }
    }

    LOG(LL_INFO, "CThread::stop():Thread %lu (%lu) finished detach.",
        (unsigned long int) _pid, (unsigned long int) _td);

    //pthread_cancel(_td);

    _m_td.unlock();
}

void CThread::cancel() {
    _m_td.lock();

    int res;
    if ((res = pthread_cancel(_td)) != 0) {
        LOG(LL_INFO, "CThread::cancel():pthread_cancel failed with code %i", res);
    } else {
        LOG(LL_INFO, "CThread::cancel():Thread %lu is canceled.", (unsigned long int) _pid);
        _stopped.set(true);
    }

    _m_td.unlock();
}

void CThread::join() {
    if(!is_stopped())
        pthread_join(_td,NULL);
}

int CThread::setRealtime() {
    return 0;
}


CThreadWatcher* CThreadWatcher::_instance=0;
CMutex CThreadWatcher::_inst_mut;

CThreadWatcher::CThreadWatcher()
: _run_cond(false),iStopRequested(false) {
}

CThreadWatcher* CThreadWatcher::instance() {
    _inst_mut.lock();
    if(!_instance){
        _instance = new CThreadWatcher();
        _instance->start();
    }

    _inst_mut.unlock();
    return _instance;
}

void CThreadWatcher::add(CThread* t) {
    LOG(LL_INFO, "CThreadWatcher::add():trying to add thread %lu to thread watcher.",
        (unsigned long int) t->_pid);
    q_mut.lock();
    thread_queue.push(t);
    _run_cond.set(true);
    q_mut.unlock();
    LOG(LL_INFO, "CThreadWatcher::add():added thread %lu to thread watcher.",
        (unsigned long int) t->_pid);
}

// ---------------------------------------------------------------------------
// void CThreadWatcher::Destroy()
//
// delete instance.
// ---------------------------------------------------------------------------
//
void CThreadWatcher::Destroy() {
    DEBUG(LL_ALL, "CThreadWatcher::Destroy():Begin");
    if(_instance != NULL) {
        if(!_instance->is_stopped()) {
            _instance->stop();

            while(!_instance->is_stopped()) {
                usleep(10000);
            }
        }
        delete _instance;
        _instance = NULL;
    }
    DEBUG(LL_ALL, "CThreadWatcher::Destroy():End");
}

void CThreadWatcher::on_stop() {
    iStopRequested.set(true);
}

void CThreadWatcher::run() {
    while(!iStopRequested.get()){

        _run_cond.wait_for();
        // Let some time for to threads
        // to stop by themselves
        sleep(10);

        q_mut.lock();
        LOG(LL_VARS, "CThreadWatcher::run():Thread watcher starting its work");

        try {
            std::queue<CThread*> n_thread_queue;

            while(!thread_queue.empty()){

                CThread* cur_thread = thread_queue.front();
                thread_queue.pop();

                q_mut.unlock();
                LOG(LL_VARS, "CThreadWatcher::run():thread %lu is to be processed in thread watcher.",
                    (unsigned long int) cur_thread->_pid);
                if(cur_thread->is_stopped()){
                    LOG(LL_INFO, "CThreadWatcher::run():thread %lu has been destroyed.",
                        (unsigned long int) cur_thread->_pid);
                    delete cur_thread;
                }
                else {
                    LOG(LL_VARS, "CThreadWatcher::run():thread %lu still running.",
                        (unsigned long int) cur_thread->_pid);
                    n_thread_queue.push(cur_thread);
                }
                q_mut.lock();
            }

            swap(thread_queue,n_thread_queue);

        }catch(...){
            /* this one is IMHO very important, as lock is called in try block! */
            LOG(LL_ERROR, "CThreadWatcher::run():unexpected exception, state may be invalid!");
        }

        bool more = !thread_queue.empty();
        q_mut.unlock();

        LOG(LL_VARS, "CThreadWatcher::run():Thread watcher finished");
        if(!more)
            _run_cond.set(false);
    }
}

// timer...
CThreadTimer* CThreadTimer::_instance=0;

CThreadTimer::CThreadTimer()
: _run_cond(false),iStopRequested(false), iTimeSecs(0) {
    iTimeSecs = time(NULL);
}

CThreadTimer* CThreadTimer::instance() {
    if(!_instance){
        _instance = new CThreadTimer();
        _instance->start();
    }
    return _instance;
}

// ---------------------------------------------------------------------------
// void CThreadWatcher::Destroy()
//
// delete instance.
// ---------------------------------------------------------------------------
//
void CThreadTimer::Destroy() {
    DEBUG(LL_ALL, "CThreadTimer::Destroy():Begin");
    if(_instance != NULL) {
        if(!_instance->is_stopped()) {
            _instance->stop();

            while(!_instance->is_stopped()) {
                usleep(10000);
            }
        }
        delete _instance;
        _instance = NULL;
    }
    DEBUG(LL_ALL, "CThreadTimer::Destroy():End");
}

void CThreadTimer::on_stop() {
    iStopRequested = true;
}

void CThreadTimer::run() {
    while(!iStopRequested){

        iTimeSecs = time(NULL);
        // Let some time for to threads
        // to sleep
        usleep(400000);
    }
}
// end of local file.
