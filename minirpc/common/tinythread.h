/*
** Copyright (C) 2015 Wang Yaofu
** All rights reserved.
**
**Author:Wang Yaofu voipman@qq.com
**Description: Header file of TinyThread.
*/

#ifndef _COMMON_TINY_THREAD_THREAD_H
#define _COMMON_TINY_THREAD_THREAD_H

#include <pthread.h>
#include "common/closure.h"

typedef pthread_t ThreadHandle;
ThreadHandle CreateThread(ClosureA* func, const char* name = NULL);

#endif // _COMMON_TINY_THREAD_THREAD_H
