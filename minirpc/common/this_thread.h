/*
** Copyright (C) 2015 Wang Yaofu
** All rights reserved.
**
**Author:Wang Yaofu voipman@qq.com
**Description: Header file of class ThisThread.
*/

#ifndef _COMMON_ATOMIC_THIS_THREAD_H
#define _COMMON_ATOMIC_THIS_THREAD_H

#include <pthread.h>
#include <unistd.h>
#include <sys/syscall.h>

class ThisThread {
public:
    static void SleepInMs(int ms)
    {
        ::usleep(ms * 1000);
    }

    static void SleepInUs(int us)
    {
        ::usleep(us);
    }

    static void Yield()
    {
        ::pthread_yield();
    }

    static int GetId()
    {
        static __thread pid_t tid = 0;
        if (tid == 0) {
            tid = ::syscall(SYS_gettid);
        }
        return tid;
    }

    static pthread_t GetThreadId()
    {
        return ::pthread_self();
    }

private:
    ThisThread();
};

#endif // _COMMON_ATOMIC_THIS_THREAD_H
