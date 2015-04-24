/*
** Copyright (C) 2014 Wang Yaofu
** All rights reserved.
**
**Author:Wang Yaofu voipman@qq.com
**Description: The header file of class CMsgQueue.
*/
#ifndef _COMMON_IPC_CMSGQUEUE_H_
#define _COMMON_IPC_CMSGQUEUE_H_
#include <sys/sem.h>
#include <string>
namespace wyf {

#define RET_OK     0
#define RET_ERROR -1
using std::string;
struct SMsgBuffer {
    // msgqueue's msg type ID
    long iMsgType;
    // variable storage
    char iMsgStr[1024];
};

class CMsgQueue {
 public:
    CMsgQueue();
    ~CMsgQueue();
    int InitData();

    // methods for message queue.
    int MsgGet(int aKey, int aFlags);
    int MsgSend(int aMsgId, const SMsgBuffer* aMsgBuffer, int aFlags = 0);
    int MsgRecv(int aMsgId, SMsgBuffer* aMsgBuffer, int aLen, int aFlags = 0);
    int MsgRm(int aMsgId);
    int MsgSet(int aMsgId, int aQBytes);
    const string MsgStat(int aMsgId);
    const char* GetFileMode(mode_t aMode, char* aResp);
};

} // namespace wyf
#endif  // _COMMON_IPC_CMSGQUEUE_H_
