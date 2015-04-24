/*
** Copyright (C) 2014 Wang Yaofu
** All rights reserved.
**
**Author:Wang Yaofu voipman@qq.com
**Description: The header file of class CThread,CMutex and CThreadWatcher.
*/

#ifndef _COMMON_CTHREAD_H_
#define _COMMON_CTHREAD_H_

#include <pthread.h>
#include <sys/time.h>
#include <time.h>
#include <errno.h>
#include <queue>

/**
* \brief C++ Wrapper class for pthread mutex
*/
class CMutex {
    pthread_mutex_t m;

 public:
    CMutex();
    ~CMutex();
    void lock();
    void unlock();
};

/**
* \brief  Simple lock class
*/
class CLock {
    CMutex& m;
 public:
    explicit CLock(CMutex& _m) : m(_m) {
        m.lock();
    }
    ~CLock(){
        m.unlock();
    }
};

class CSpinLock {
 public:
    CSpinLock() {
        pthread_spin_init(&mSpin, PTHREAD_PROCESS_PRIVATE);
    }
    ~CSpinLock() {
        pthread_spin_destroy(&mSpin);
    }
    void Lock() {
        pthread_spin_lock(&mSpin);
    }
    void Unlock() {
        pthread_spin_unlock(&mSpin);
    }
    bool IsLocked() const {
        return mSpin == 0;
    }

 private:
    pthread_spinlock_t mSpin;
};
/**
* \brief Shared variable.
*
* Include a variable and its mutex.
* @warning Don't use safe functions (set,get)
* within a {lock(); ... unlock();} block. Use
* unsafe function instead.
*/
template<class T>
class CSharedVar {
    T t;
    CMutex m;

 public:
    explicit CSharedVar(const T& _t) : t(_t) {}
    CSharedVar() {}

    T get() {
        lock();
        T res = unsafe_get();
        unlock();
        return res;
    }

    void set(const T& new_val) {
        lock();
        unsafe_set(new_val);
        unlock();
    }

    void lock() { m.lock(); }
    void unlock() { m.unlock(); }

    const T& unsafe_get() { return t; }
    void unsafe_set(const T& new_val) { t = new_val; }
};

/**
* \brief C++ Wrapper class for pthread condition
*/
template<class T>
class CCondition {
    T t;
    pthread_mutex_t m;
    pthread_cond_t  cond;

 public:
    explicit CCondition(const T& _t)
        : t(_t) {
        pthread_mutex_init(&m, NULL);
        pthread_cond_init(&cond, NULL);
    }

    ~CCondition() {
        pthread_cond_destroy(&cond);
        pthread_mutex_destroy(&m);
    }

    /** Change the condition's value. */
    void set(const T& newval) {
        pthread_mutex_lock(&m);
        t = newval;
        if (t) pthread_cond_signal(&cond);
        pthread_mutex_unlock(&m);
    }

    T get() {
        T val;
        pthread_mutex_lock(&m);
        val = t;
        pthread_mutex_unlock(&m);
        return val;
    }
    /** Waits for the condition to be true. */
    void wait_for() {
        pthread_mutex_lock(&m);
        while (!t) {
            pthread_cond_wait(&cond, &m);
        }
        pthread_mutex_unlock(&m);
    }

    /** Waits for the condition to be true or a timeout. */
    bool wait_for_to(unsigned long usec) {
        struct timeval now;
        struct timespec timeout;
        int retcode = 0;
        bool ret = false;

        gettimeofday(&now, NULL);
        timeout.tv_sec = now.tv_sec + (usec / 1000000);
        timeout.tv_nsec = (now.tv_usec + (usec % 1000000)) * 1000;

        pthread_mutex_lock(&m);
        while (!t && !retcode) {
            retcode = pthread_cond_timedwait(&cond, &m, &timeout);
        }

        if (t) ret = true;
        pthread_mutex_unlock(&m);

        return ret;
    }
};

template<class T>
class CRWCondition {
    T r_t;
    T w_t;
    pthread_mutex_t m;
    pthread_cond_t  r_cond;
    pthread_cond_t  w_cond;

 public:
    explicit CRWCondition(const T& _t)
        : r_t(_t), w_t(_t) {
        pthread_mutex_init(&m, NULL);
        pthread_cond_init(&r_cond, NULL);
        pthread_cond_init(&w_cond, NULL);
    }

    ~CRWCondition() {
        pthread_cond_destroy(&r_cond);
        pthread_cond_destroy(&w_cond);
        pthread_mutex_destroy(&m);
    }

    /** Change the condition's value. */
    void set(const T& newval, bool isR) {
        if (isR) {
            r_t = newval;
            if (r_t) pthread_cond_signal(&r_cond);
        } else {
            w_t = newval;
            if (w_t) pthread_cond_signal(&w_cond);
        }
    }
    void lock() {
        pthread_mutex_lock(&m);
    }
    void unlock() {
        pthread_mutex_unlock(&m);
    }
    void wait_for(bool isR) {
        if (isR) {
            while (!r_t) {
                pthread_cond_wait(&r_cond, &m);
            }
        } else {
            while (!w_t) {
                pthread_cond_wait(&w_cond, &m);
            }
        }
    }
};

/**
* \brief C++ Wrapper class for pthread
*/
class CThread {
    pthread_t _td;
    CMutex   _m_td;

    CSharedVar<bool> _stopped;

    static void* _start(void* _t);

 protected:
    virtual void run()=0;
    virtual void on_stop()=0;

 public:
    unsigned long _pid;
    CThread();
    virtual ~CThread() {}

    virtual void onIdle() {}

    /** Start it ! */
    void start(bool realtime = false);
    /** Stop it ! */
    void stop();
    /** @return true if this thread doesn't run. */
    bool is_stopped() { return _stopped.get(); }
    /** Wait for this thread to finish */
    void join();
    /** kill the thread 
     *(if pthread_setcancelstate(PTHREAD_CANCEL_ENABLED) has been set) **/
    void cancel();

    int setRealtime();
};

/**
* \brief Container/garbage collector for threads.
*
* CThreadWatcher waits for threads to stop
* and delete them.
* It gets started automatically when needed.
* Once you added a thread to the container,
* there is no mean to get it out.
*/
class CThreadWatcher: public CThread {
    static CThreadWatcher* _instance;
    static CMutex          _inst_mut;
    /*
    * the condition of thread to start or stop.
    *
    */
    CSharedVar<bool> iStopRequested;

    std::queue<CThread*> thread_queue;
    CMutex          q_mut;

    /** the daemon only runs if this is true */
    CCondition<bool> _run_cond;
    CThreadWatcher();
    void run();
    void on_stop();

 public:
    static CThreadWatcher* instance();
    void add(CThread* t);
    /*
    * delete instance
    *
    */
    static void Destroy();
};

class CThreadTimer: public CThread {
    static CThreadTimer* _instance;
    /*
    * the condition of thread to start or stop.
    *
    */
    bool iStopRequested;

    /** the daemon only runs if this is true */
    bool _run_cond;
    CThreadTimer();
    void run();
    void on_stop();
    time_t iTimeSecs;

 public:
    static CThreadTimer* instance();
    inline long getNowSecs() {
        return iTimeSecs;
    }
    /*
    * delete instance
    *
    */
    static void Destroy();
};

//
// Read-Write Lock
//
class CRWLock {
 public:
    // constructor
    CRWLock() {
        // default lock attr
        pthread_rwlock_init(&m_rwLock, NULL);
    }

    // destructor
    virtual ~CRWLock() {
        pthread_rwlock_destroy(&m_rwLock);
    }

 public:
    // lock read lock
    bool useRD() {
        return (pthread_rwlock_rdlock(&m_rwLock) == 0) ? true : false;
    }

    // try to lock read lock
    bool tryRD() {
        return (pthread_rwlock_tryrdlock(&m_rwLock) == 0) ? true : false;
    }

    // lock write lock
    bool useWR() {
        return (pthread_rwlock_wrlock(&m_rwLock) == 0) ? true : false;
    }

    // lock write lock
    bool tryWR() {
        return (pthread_rwlock_trywrlock(&m_rwLock) == 0) ? true : false;
    }

    // unlock read-write lock
    bool unlock() {
        return (pthread_rwlock_unlock(&m_rwLock) == 0) ? true : false;
    }

 private:
    // read-write lock
    pthread_rwlock_t m_rwLock;
};

//
// Read Scope lock
//
class CScopeRDLock {
 public:
    // constructor
    explicit CScopeRDLock(CRWLock &rdlock) : m_srdLock(rdlock) {
        m_srdLock.useRD();
    }

    // destructor
    virtual ~CScopeRDLock() {
        m_srdLock.unlock();
    }

 private:
    // read lock
    CRWLock &m_srdLock;
};

//
// write scope lock
//
class CScopeWRLock {
 public:
    // constructor
    explicit CScopeWRLock(CRWLock &wrlock) : m_swrLock(wrlock) {
        m_swrLock.useWR();
    }

    // destructor
    virtual ~CScopeWRLock() {
        m_swrLock.unlock();
    }

 private:
    // read lock
    CRWLock &m_swrLock;
};
#endif  // _COMMON_CTHREAD_H_
