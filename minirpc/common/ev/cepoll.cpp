/*
** Copyright (C) 2014 Wang Yaofu
** All rights reserved.
**
**Author:Wang Yaofu voipman@qq.com
**Description: The source file of class CEventPoll by api epoll.
*/
#include <sys/epoll.h>
#include <cstdlib>
#include <cstring>
#include <cerrno>
#include "common/clogwriter.h"
typedef struct SEventState {
    int epfd;
    struct epoll_event *events;
} SEventState;

int CEventPoll::ApiCreate(CEventLoop * aEventLoop) {
    const char* funcName = "CEventPoll::ApiCreate";
    DEBUG(LL_ALL, "%s:Begin", funcName);
    LOG(LL_INFO, "%s:try malloc event state.", funcName);
    iEventState = reinterpret_cast<SEventState *>(malloc(sizeof(SEventState)));
    if (iEventState == NULL) {
        LOG(LL_ERROR, "%s:malloc event state error:(%s).",
            funcName, strerror(errno));
        return RET_ERROR;
    }
    iEventLoop = aEventLoop;
    // 1024 is just an hint for the kernel
    iEventState->epfd = epoll_create(1024);
    if (iEventState->epfd == RET_ERROR) {
        LOG(LL_ERROR, "%s:epoll_create error:(%s).",
            funcName, strerror(errno));
        return RET_ERROR;
    }
    iEventState->events =
        (struct epoll_event *)malloc(
        sizeof(struct epoll_event)*iEventLoop->iSetSize);
    if (iEventState->events == NULL) {
        LOG(LL_ERROR, "%s:malloc event state.events error:(%s).",
            funcName, strerror(errno));
        return RET_ERROR;
    }
    DEBUG(LL_ALL, "%s:End", funcName);
    return RET_OK;
}

void CEventPoll::ApiFree() {
    DEBUG(LL_ALL, "CEventPoll::ApiFree():Begin");
    if (iEventState != NULL) {
        free(iEventState);
        iEventState = NULL;
    }
    DEBUG(LL_ALL, "CEventPoll::ApiFree():End");
}

int CEventPoll::ApiAddEvent(int aSockId, int aAddMask) {
    const char* funcName = "CEventPoll::ApiAddEvent";
    DEBUG(LL_ALL, "%s:Begin", funcName);
    LOG(LL_VARS, "%s:sockid:(%d), mask:(%d).",
        funcName, aSockId, aAddMask);
    if (aSockId <= 2) {
        return RET_ERROR;
    }
    struct epoll_event ee;
    /* If the fd was already monitored for some event, we need a MOD
     * operation. Otherwise we need an ADD operation. */
    int op = iEventLoop->iFileEvent[aSockId].iMask == AE_NONE ?
            EPOLL_CTL_ADD : EPOLL_CTL_MOD;

    ee.events = 0;
    aAddMask |= iEventLoop->iFileEvent[aSockId].iMask; /* Merge old events */
    iEventLoop->iFileEvent[aSockId].iMask = aAddMask;
    if (aAddMask & AE_READABLE) ee.events |= EPOLLIN;
    if (aAddMask & AE_WRITABLE) ee.events |= EPOLLOUT;
    ee.data.u64 = 0; /* avoid valgrind warning */
    ee.data.fd = aSockId;
    if (epoll_ctl(iEventState->epfd, op, aSockId, &ee) == RET_ERROR) {
        DEBUG(LL_ERROR, "%s:epoll_ctl error:(%s).",
            funcName, strerror(errno));
        return RET_ERROR;
    }
    DEBUG(LL_ALL, "%s:End", funcName);
    return RET_OK;
}

void CEventPoll::ApiDelEvent(int aSockId, int aDelMask) {
    const char* funcName = "CEventPoll::ApiDelEvent";
    DEBUG(LL_ALL, "%s:Begin", funcName);
    LOG(LL_VARS, "%s:sockid:(%d), mask:(%d).",
        funcName, aSockId, aDelMask);
    struct epoll_event ee;
    ee.events = 0;
    if (aDelMask == AE_NONE)    ee.events |= EPOLLIN | EPOLLOUT;
    if (aDelMask & AE_READABLE) ee.events |= EPOLLIN;
    if (aDelMask & AE_WRITABLE) ee.events |= EPOLLOUT;
    ee.data.u64 = 0; /* avoid valgrind warning */
    ee.data.fd = aSockId;
    /* If the fd was already monitored for some event, we need a MOD
     * operation. Otherwise we need an DEL operation. */
    int op = aDelMask == AE_NONE ? EPOLL_CTL_DEL : EPOLL_CTL_MOD;

    /* Note, Kernel < 2.6.9 requires a non null event pointer even for
     * EPOLL_CTL_DEL. */
    if (epoll_ctl(iEventState->epfd, op, aSockId, &ee) == RET_ERROR) {
        DEBUG(LL_ERROR, "%s:epoll_ctl error:(%s).",
            funcName, strerror(errno));
        return;
    }
    DEBUG(LL_ALL, "%s:End", funcName);
}

int CEventPoll::ApiPollWait(struct timeval *aTvp) {
    const char* funcName = "CEventPoll::ApiPollWait";
    DEBUG(LL_ALL, "%s:Begin", funcName);
    int retval, numevents = 0;

    retval = epoll_wait(iEventState->epfd,
            iEventState->events, iEventLoop->iSetSize,
            aTvp ? (aTvp->tv_sec*1000 + aTvp->tv_usec/1000) : -1);
    if (retval > 0) {
        int j;

        numevents = retval;
        for (j = 0; j < numevents; j++) {
            int mask = 0;
            struct epoll_event *e = iEventState->events+j;

            if (e->events & EPOLLIN) {
                DEBUG(LL_DBG, "%s:read sockid:(%d) set is ok.",
                    funcName, e->data.fd);
                mask |= AE_READABLE;
            }
            if (e->events & EPOLLOUT) {
                DEBUG(LL_DBG, "%s:write sockid:(%d) set is ok.",
                    funcName, e->data.fd);
                mask |= AE_WRITABLE;
            }
            if (e->events & (EPOLLERR|EPOLLHUP)) {
                DEBUG(LL_DBG, "%s:err sockid:(%d) set is ok.",
                    funcName, e->data.fd);
                mask |= AE_RWERROR;
            }
            iEventLoop->iFiredEvent[j].iSockId = e->data.fd;
            iEventLoop->iFiredEvent[j].iMask   = mask;
        }
    } else if (retval == 0) {
        LOG(LL_INFO, "%s:epoll time out.", funcName);
    } else {
        LOG(LL_ERROR, "%s:epoll error:(%s).", funcName, strerror(errno));
    }
    LOG(LL_INFO, "%s:fired num:(%d).", funcName, numevents);
    DEBUG(LL_ALL, "%s:End", funcName);
    return numevents;
}

const char* CEventPoll::ApiName() {
    return "epoll";
}
