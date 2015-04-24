/*
** Copyright (C) 2014 Wang Yaofu
** All rights reserved.
**
**Author:Wang Yaofu voipman@qq.com
**Description: The source file of class CEventLoop.
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include "common/ev/ceventloop.h"
#include "common/ev/ceventhandler.h"
#include "common/ev/ceventpoll.h"
#include "common/clogwriter.h"
CEventLoop::CEventLoop() {
    const char* funcName = "CEventLoop::CEventLoop()";
    DEBUG(LL_ALL, "%s:Begin", funcName);
    iFiredEvent = NULL;
    iFileEvent  = NULL;
    iEventPoll  = NULL;
    iSetSize = 0;
    iMaxSockId = 0;
    iIsInitialed = false;
    DEBUG(LL_ALL, "%s:End", funcName);
}

CEventLoop::~CEventLoop() {
    const char* funcName = "CEventLoop::~CEventLoop()";
    DEBUG(LL_ALL, "%s:Begin", funcName);
    DeleteEventLoop();
    DEBUG(LL_ALL, "%s:End", funcName);
}

int CEventLoop::CreateEventLoop(int aSetSize) {
    const char* funcName = "CEventLoop::CreateEventLoop()";
    DEBUG(LL_ALL, "%s:Begin", funcName);
    LOG(LL_VARS, "%s:set size:(%d).", funcName, aSetSize);
    int ret = RET_ERROR;
    iSetSize = aSetSize;
    LOG(LL_INFO, "%s:try malloc fire event.", funcName);
    iFiredEvent =
        reinterpret_cast<SFileEvent *>(malloc(sizeof(SFileEvent)*iSetSize));
    if (iFiredEvent == NULL) {
        LOG(LL_ERROR, "%s:malloc fire event error:(%s).",
            funcName, strerror(errno));
        return RET_ERROR;
    }
    LOG(LL_INFO, "%s:try malloc file event.", funcName);
    iFileEvent =
        reinterpret_cast<SFileEvent *>(malloc(sizeof(SFileEvent)*iSetSize));
    if (iFileEvent == NULL) {
        LOG(LL_ERROR, "%s:malloc file event error:(%s).",
            funcName, strerror(errno));
        free(iFiredEvent);
        iFiredEvent = NULL;
        return RET_ERROR;
    }
    iFileEventHandler = NULL;
    LOG(LL_INFO, "%s:try zero file and fire event.", funcName);
    memset(iFiredEvent, 0, sizeof(SFileEvent) * iSetSize);
    memset(iFileEvent,  0, sizeof(SFileEvent) * iSetSize);
    int ii = 0;
    LOG(LL_INFO, "%s:try initializing file and fire event.", funcName);
    while (ii < iSetSize) {
        iFileEvent[ii].iMask   = AE_NONE;
        iFileEvent[ii].iSockId = 0;
        ii++;
    }
    try {
        LOG(LL_INFO, "%s:try new CEventPoll.", funcName);
        iEventPoll = new CEventPoll();
    } catch (...) {
        LOG(LL_ERROR, "%s:new CEventPoll failed.", funcName);
        free(iFiredEvent);
        iFiredEvent = NULL;
        free(iFileEvent);
        iFileEvent = NULL;
        return RET_ERROR;
    }
    if (iEventPoll == NULL) {
        LOG(LL_ERROR, "%s:iEventPoll is null.", funcName);
        free(iFiredEvent);
        iFiredEvent = NULL;
        free(iFileEvent);
        iFileEvent = NULL;
        return RET_ERROR;
    }
    LOG(LL_INFO, "%s:try CEventPoll.ApiCreate.", funcName);
    ret = iEventPoll->ApiCreate(this);
    if (ret != RET_OK) {
        LOG(LL_ERROR, "%s:ApiCreate failed.", funcName);
        free(iFiredEvent);
        iFiredEvent = NULL;
        free(iFileEvent);
        iFileEvent = NULL;
        delete iEventPoll;
        iEventPoll = NULL;
        return ret;
    }
    LOG(LL_WARN, "%s:CEventPoll.(%s) is using...",
        funcName, iEventPoll->ApiName());
    iIsInitialed = true;
    DEBUG(LL_ALL, "%s:End", funcName);
    return RET_OK;
}

void CEventLoop::DeleteEventLoop() {
    const char* funcName = "CEventLoop::DeleteEventLoop()";
    DEBUG(LL_ALL, "%s:Begin", funcName);
    DEBUG(LL_INFO, "%s:try to free fire and file event.", funcName);
    if (iFiredEvent != NULL) {
        free(iFiredEvent);
        iFiredEvent = NULL;
    }
    if (iFileEvent != NULL) {
        free(iFileEvent);
        iFileEvent = NULL;
    }
    DEBUG(LL_INFO, "%s:try to free ApiPoll event.", funcName);
    if (iEventPoll != NULL) {
        iEventPoll->ApiFree();
        free(iEventPoll);
        iFiredEvent = NULL;
    }
    DEBUG(LL_INFO, "%s:try to zero file event handler.", funcName);
    if (iFileEventHandler != NULL) {
        iFileEventHandler = NULL;
    }
    iSetSize = 0;
    iMaxSockId = 0;
    iIsInitialed = false;
    DEBUG(LL_ALL, "%s:End", funcName);
}

void CEventLoop::Stop() {
    LOG(LL_INFO, "CEventLoop::Stop():Begin");
    LOG(LL_INFO, "CEventLoop::Stop():End");
}

int CEventLoop::AddFileEvent(int aSockId, int aMask,
                    CFileEventHandler *aFEHandler) {
    const char* funcName = "CEventLoop::AddFileEvent";
    DEBUG(LL_ALL, "%s:Begin", funcName);
    LOG(LL_DBG, "%s:sockid:(%d),mask:(%d).",
        funcName, aSockId, aMask);
    if (!iIsInitialed) {
        LOG(LL_ERROR, "%s:is not initialed.", funcName);
        return RET_ERROR;
    }
    if (aSockId >= iSetSize) {
        errno = ERANGE;
        LOG(LL_ERROR, "%s:to large sockid:(%d),max:(%d),error:(%s).",
            funcName, aSockId, iSetSize, strerror(errno));
        return RET_ERROR;
    }
    if (aFEHandler == NULL) {
        LOG(LL_ERROR, "%s:file event handler is null.", funcName);
        return RET_ERROR;
    } else if (iFileEventHandler == NULL) {
        iFileEventHandler = aFEHandler;
    }

    DEBUG(LL_DBG, "%s:try add event poll.", funcName);
    if (RET_ERROR == iEventPoll->ApiAddEvent(aSockId, aMask)) {
        LOG(LL_ERROR, "%s:ApiAddEvent failed sockid:(%d), mask:(%d).",
            funcName, aSockId, aMask);
        return RET_ERROR;
    }
    iFileEvent[aSockId].iSockId = aSockId;
    DEBUG(LL_VARS, "%s:sockid:(%d), new mask:(%d).",
        funcName, aSockId, iFileEvent[aSockId].iMask);

    if (aSockId > iMaxSockId) {
        iMaxSockId = aSockId;
        LOG(LL_INFO, "%s:max sockid:(%d)", funcName, iMaxSockId);
    }
    DEBUG(LL_ALL, "%s:End", funcName);
    return RET_OK;
}

void CEventLoop::DeleteFileEvent(int aSockId, int aMask) {
    const char* funcName = "CEventLoop::DeleteFileEvent";
    DEBUG(LL_ALL, "%s:Begin", funcName);
    LOG(LL_DBG, "%s:sockid:(%d),mask:(%d).",
        funcName, aSockId, aMask);
    if (!iIsInitialed) {
        LOG(LL_ERROR, "%s:is not initialed.", funcName);
        return;
    }
    if (aSockId >= iSetSize) return;
    if (iFileEvent[aSockId].iMask == AE_NONE) return;
    iFileEvent[aSockId].iMask &= ~aMask;
    DEBUG(LL_VARS, "%s:max sockid:(%d),new mask is:(%d).",
        funcName, iMaxSockId, iFileEvent[aSockId].iMask);
    if (aSockId == iMaxSockId && iFileEvent[aSockId].iMask == AE_NONE) {
        /* Update the max fd */
        int j;
        for (j = iMaxSockId-1; j >= 3; j--)
            if (iFileEvent[j].iMask != AE_NONE) break;
        iMaxSockId = j;
        LOG(LL_VARS, "%s:update max sockid:(%d).", funcName, iMaxSockId);
    }
    DEBUG(LL_INFO, "%s:try delete event from poll.", funcName);
    iEventPoll->ApiDelEvent(aSockId, iFileEvent[aSockId].iMask);
    DEBUG(LL_ALL, "%s:End", funcName);
}

int CEventLoop::GetFileEvents(int aSockId) {
    if (!iIsInitialed) {
        LOG(LL_ERROR, "CEventLoop::GetFileEvents is not initialed.");
        return RET_ERROR;
    }
    if (aSockId >= iSetSize) return 0;

    return iFileEvent[aSockId].iMask;
}

int CEventLoop::ProcessEvents(int aEventFlag, int aWaitTime) {
    const char* funcName = "CEventLoop::ProcessEvents";
    DEBUG(LL_ALL, "%s:Begin", funcName);
    int processed = 0, numevents;

    /* Nothing to do? return ASAP */
    if (!(aEventFlag & AE_FILE_EVENTS)) return 0;

    /* Note that we want call select() even if there are no
     * file events to process as long as we want to process time
     * events, in order to sleep until the next time event is ready
     * to fire. */
    if (iMaxSockId != 0 || !(aEventFlag & AE_DONT_WAIT)) {
        int j;
        struct timeval tv, *tvp;
        /* If we have to check for events but need to return
         * ASAP because of AE_DONT_WAIT we need to set the timeout
         * to zero
         */
        tvp = NULL;
        if (aEventFlag & AE_DONT_WAIT) {
            tv.tv_sec = tv.tv_usec = 0;
            tvp = &tv;
        } else if (aEventFlag & AE_WAIT_FOREVER) {
            tvp = NULL; /* wait forever */
        } else {
            tv.tv_sec = aWaitTime / 1000;
            tv.tv_usec = (aWaitTime % 1000) * 1000;
            tvp = &tv;
        }
        numevents = iEventPoll->ApiPollWait(tvp);
        for (j = 0; j < numevents; j++) {
            int mask = iFiredEvent[j].iMask;
            int fd   = iFiredEvent[j].iSockId;
            int rfired = 0;

            /* note the fe->mask & mask & ... code: maybe an already processed
             * event removed an element that fired and we still didn't
             * processed, so we check if the event is still valid. */
            LOG(LL_DBG, "%s:fireMask=(%d),fileMask=(%d), ",
                funcName, mask, iFileEvent[fd].iMask);

            if (iFileEvent[fd].iMask & mask & AE_RWERROR) {
                iFileEventHandler->ErrSocket(fd);
            }
            if (iFileEvent[fd].iMask & mask & AE_READABLE) {
                rfired = 1;
                iFileEventHandler->Request(fd);
            }
            if (iFileEvent[fd].iMask & mask & AE_WRITABLE) {
                if (!rfired) iFileEventHandler->Response(fd);
            }
            processed++;
        }
    }

    DEBUG(LL_ALL, "%s:End", funcName);
    return processed; /* return the number of processed file events */
}

void CEventLoop::MainLoop(int aWaitTime) {
    if (!iIsInitialed) {
        LOG(LL_ERROR, "CEventLoop::MainLoop is not initialed.");
        return;
    }
    ProcessEvents(AE_ALL_EVENTS, aWaitTime);
}
// end of local file.
