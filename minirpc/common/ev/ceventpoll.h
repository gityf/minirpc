/*
** Copyright (C) 2014 Wang Yaofu
** All rights reserved.
**
**Author:Wang Yaofu voipman@qq.com
**Description: The header file of class CEventPoll.
*/
#ifndef _COMMON_EV_CEVENTPOLL_
#define _COMMON_EV_CEVENTPOLL_
class CEventLoop;
struct SEventState;
class CEventPoll {
 public:
    CEventPoll() {}
    ~CEventPoll() {}

    int ApiCreate(CEventLoop * aEventLoop);
    void ApiFree();
    int ApiAddEvent(int aSockId, int aAddMask);
    void ApiDelEvent(int aSockId, int aDelMask);
    int ApiPollWait(struct timeval *aTvp);
    const char* ApiName();
 private:
    CEventLoop *iEventLoop;
    SEventState *iEventState;
};
#endif  // _COMMON_EV_CEVENTPOLL_
