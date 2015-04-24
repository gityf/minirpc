/*
** Copyright (C) 2014 Wang Yaofu
** All rights reserved.
**
**Author:Wang Yaofu voipman@qq.com
**Description: The header file of class CEventLoop.
*/
#ifndef _COMMON_EV_CEVENTLOOP_
#define _COMMON_EV_CEVENTLOOP_
/*
* return code defination.
*/
#define RET_OK      0
#define RET_ERROR  -1
#define AE_NONE     0
#define AE_READABLE 1
#define AE_WRITABLE 2
#define AE_RWERROR  4


#define AE_FILE_EVENTS 1
#define AE_TIME_EVENTS 2
#define AE_ALL_EVENTS (AE_FILE_EVENTS|AE_TIME_EVENTS)
#define AE_DONT_WAIT   4
#define AE_WAIT_FOREVER 8
#define AE_WAIT_MS      500

class CEventPoll;
/* A fired event */
typedef struct SFileEvent {
    int iSockId;
    int iMask;
} SFileEvent;
class CFileEventHandler;
class CEventLoop {
 public:
    CEventLoop();
    ~CEventLoop();

    int CreateEventLoop(int aSetSize);
    void DeleteEventLoop();
    void Stop();
    int AddFileEvent(int aSockId, int aMask, CFileEventHandler *aFEHandler);
    void DeleteFileEvent(int aSockId, int aMask);
    int GetFileEvents(int aSockId);
    int ProcessEvents(int aEventFlag, int aWaitTime);
    void MainLoop(int aWaitTime);

    SFileEvent *iFileEvent;
    SFileEvent *iFiredEvent;
    int iMaxSockId;
    int iSetSize;
 private:
    CFileEventHandler *iFileEventHandler;
    CEventPoll *iEventPoll;
    bool iIsInitialed;
};
#endif  // _COMMON_EV_CEVENTLOOP_
