/*
** Copyright (C) 2014 Wang Yaofu
** All rights reserved.
**
**Author:Wang Yaofu voipman@qq.com
**Description: The source file of class CEventPoll by api select.
*/
#include <string.h>
#include <sys/select.h>
#include <stdlib.h>
#include <errno.h>
#include "common/clogwriter.h"
typedef struct SEventState {
    fd_set rfds, wfds;
    /* We need to have a copy of the fd sets as it's not safe to reuse
     * FD sets after select(). */
    fd_set _rfds, _wfds;
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
    FD_ZERO(&iEventState->rfds);
    FD_ZERO(&iEventState->wfds);
    FD_ZERO(&iEventState->_rfds);
    FD_ZERO(&iEventState->_wfds);
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
    // do not add fd=0 into select to avoid while(1) in ApiPollWait.
    if (aSockId <= 2) {
        return RET_ERROR;
    }
    aAddMask |= iEventLoop->iFileEvent[aSockId].iMask; /* Merge old events */
    iEventLoop->iFileEvent[aSockId].iMask = aAddMask;

    if (aAddMask & AE_READABLE) FD_SET(aSockId, &iEventState->rfds);
    if (aAddMask & AE_WRITABLE) FD_SET(aSockId, &iEventState->wfds);
    DEBUG(LL_ALL, "%s:End", funcName);
    return RET_OK;
}

void CEventPoll::ApiDelEvent(int aSockId, int aDelMask) {
    const char* funcName = "CEventPoll::ApiDelEvent";
    DEBUG(LL_ALL, "%s:Begin", funcName);
    LOG(LL_VARS, "%s:sockid:(%d), mask:(%d).",
        funcName, aSockId, aDelMask);
    FD_CLR(aSockId, &iEventState->rfds);
    FD_CLR(aSockId, &iEventState->wfds);
    if (aDelMask & AE_READABLE) FD_SET(aSockId, &iEventState->rfds);
    if (aDelMask & AE_WRITABLE) FD_SET(aSockId, &iEventState->wfds);
    DEBUG(LL_ALL, "%s:End", funcName);
}

int CEventPoll::ApiPollWait(struct timeval *aTvp) {
    const char* funcName = "CEventPoll::ApiPollWait";
    DEBUG(LL_ALL, "%s:Begin", funcName);
    int retval, j, numevents = 0;

    memcpy(&iEventState->_rfds, &iEventState->rfds, sizeof(fd_set));
    memcpy(&iEventState->_wfds, &iEventState->wfds, sizeof(fd_set));

    LOG(LL_INFO, "%s:try select with max sockid:(%d)...",
        funcName, iEventLoop->iMaxSockId);
    #ifdef HPUX
    retval = select(iEventLoop->iMaxSockId+1,
                reinterpret_cast<int *>(&iEventState->_rfds),
                reinterpret_cast<int *>(&iEventState->_wfds),
                NULL, aTvp);
    #else
    retval = select(iEventLoop->iMaxSockId+1,
                &iEventState->_rfds,
                &iEventState->_wfds,
                NULL, aTvp);
    #endif
    if (retval > 0) {
        for (j = 3; j <= iEventLoop->iMaxSockId; j++) {
            int mask = AE_NONE;
            SFileEvent *fe = &iEventLoop->iFileEvent[j];

            if (fe->iMask == AE_NONE) continue;
            /*
            if ((fe->iMask & AE_READABLE) && FD_ISSET(j,&iEventState->_rfds))
            {
                DEBUG(LL_DBG, "%s:read sockid:(%d) set is ok.", funcName, j);
                mask |= AE_READABLE;
            }
            if ((fe->iMask & AE_WRITABLE) && FD_ISSET(j,&iEventState->_wfds))
            {
                DEBUG(LL_DBG, "%s:write sockid:(%d) set is ok.", funcName, j);
                mask |= AE_WRITABLE;
            }
            */
            if (FD_ISSET(j, &iEventState->_rfds)) {
                DEBUG(LL_DBG, "%s:read sockid:(%d) set is ok.", funcName, j);
                mask |= AE_READABLE;
            }
            if (FD_ISSET(j, &iEventState->_wfds)) {
                DEBUG(LL_DBG, "%s:write sockid:(%d) set is ok.", funcName, j);
                mask |= AE_WRITABLE;
            }
            if (mask != AE_NONE) {
                iEventLoop->iFiredEvent[numevents].iSockId = j;
                iEventLoop->iFiredEvent[numevents].iMask   = mask;
                numevents++;
            }
            DEBUG(LL_DBG, "%s:fired event sockid:(%d),mask:(%d).",
                funcName, j, mask);
        }
    } else if (retval == 0) {
        LOG(LL_INFO, "%s:select time out.", funcName);
    } else {
        LOG(LL_ERROR, "%s:select error:(%s).", funcName, strerror(errno));
        FD_ZERO(&iEventState->rfds);
        FD_ZERO(&iEventState->wfds);
        FD_ZERO(&iEventState->_rfds);
        FD_ZERO(&iEventState->_wfds);
        iEventLoop->iMaxSockId = 0;
    }
    LOG(LL_INFO, "%s:fired num:(%d).", funcName, numevents);
    DEBUG(LL_ALL, "%s:End", funcName);
    return numevents;
}

const char* CEventPoll::ApiName() {
    return "select";
}
