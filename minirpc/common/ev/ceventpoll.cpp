/*
** Copyright (C) 2014 Wang Yaofu
** All rights reserved.
**
**Author:Wang Yaofu voipman@qq.com
**Description: The source file of class CEventPoll.
*/
#include "common/ev/ceventpoll.h"
#include "common/ev/ceventloop.h"
/*
#ifdef __linux__
#include "common/ev/cepoll.cpp"
#else
#include "common/ev/cselect.cpp"
#endif
//*/
/* Test for polling API */
#ifdef __linux__
#define HAVE_EPOLL 1
#endif

#if (defined(__APPLE__) && defined(MAC_OS_X_VERSION_10_6)) \
  || defined(__FreeBSD__) || defined(__OpenBSD__) || defined (__NetBSD__)
#define HAVE_KQUEUE 1
#endif

#ifdef USE_POLL
#include "cpoll.cpp"
#else
    #ifdef HAVE_EPOLL
    #include "cepoll.cpp"
    #else
        #ifdef HAVE_KQUEUE
        #include "ckqueue.cpp"
        #else
        #include "cselect.cpp"
        #endif
    #endif
#endif