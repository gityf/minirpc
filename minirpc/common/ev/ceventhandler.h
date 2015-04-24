/*
** Copyright (C) 2014 Wang Yaofu
** All rights reserved.
**
**Author:Wang Yaofu voipman@qq.com
**Description: The header file of class CFileEventHandler.
*/
#ifndef _COMMON_EV_CEVENTHANDLER_
#define _COMMON_EV_CEVENTHANDLER_

class CFileEventHandler {
 public:
     CFileEventHandler() {}
     ~CFileEventHandler() {}

    virtual void Request(int aSockId) = 0;
    virtual void Response(int aSockId) = 0;
    virtual void ErrSocket(int aSockId) = 0;
};
#endif  // _COMMON_EV_CEVENTHANDLER_
