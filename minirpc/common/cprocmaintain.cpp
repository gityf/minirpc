/*
** Copyright (C) 2014 Wang Yaofu
** All rights reserved.
**
**Author:Wang Yaofu voipman@qq.com
**Description: The source file of class CProcMaintain.
*/

#include <unistd.h>
#include <string.h>
#include <sys/time.h>
#include <csignal>
#include <vector>
#include "common/cprocmaintain.h"
#include "common/clogwriter.h"
#include "common/cstringutils.h"
#include "common/ipc/cmsgqueue.h"
using namespace wyf;
CProcMaintain::CProcMaintain() {
    iMaxChilds = 0;
    iIpcKey = 0;
    iChildsMap.clear();
    iHistoryCmds.clear();
}

CProcMaintain::~CProcMaintain() {
}

// reset ipc.
int CProcMaintain::IpcReset() {
    CMsgQueue *msgQptr = CSingleton<CMsgQueue>::Instance();

    int msgid = msgQptr->MsgGet(iIpcKey, IPC_CREAT);
    if (msgid < 0) {
        return RET_ERROR;
    }
    if (msgQptr->MsgRm(msgid) == RET_ERROR) {
        return RET_ERROR;
    }
    return RET_OK;
}

// broadcast message or command is sent to childs.
int CProcMaintain::IpcBroadcastChilds(const string& aStr) {
    const char *funcName = "CProcMaintain::IpcBroadcastChilds()";
    DEBUG(LL_ALL, "%s:Begin.", funcName);
    CMsgQueue *msgQptr = CSingleton<CMsgQueue>::Instance();

    int msgid = msgQptr->MsgGet(iIpcKey, IPC_EXCL | IPC_CREAT);
    if (msgid < 0) {
        LOG(LL_WARN, "%s:MsgGet key:%d error.", funcName, iIpcKey);
        return RET_ERROR;
    }
    SMsgBuffer msgBuf;
    snprintf(msgBuf.iMsgStr, sizeof(msgBuf.iMsgStr), "0|%s", aStr.c_str());
    for (typeof(iChildsMap.end()) it = iChildsMap.begin();
        it != iChildsMap.end(); ++it) {
        msgBuf.iMsgType = it->first;
        if (RET_ERROR == msgQptr->MsgSend(msgid, &msgBuf, IPC_NOWAIT)) {
            DEBUG(LL_WARN, "%s:MsgSend type:%d error.", funcName, it->first);
            continue;
        }
    }
    DEBUG(LL_ALL, "%s:End.", funcName);
    return RET_OK;
}

// heart beat message or comand is sent to childs.
int CProcMaintain::IpcChilds() {
    const char *funcName = "CProcMaintain::IpcChids()";
    DEBUG(LL_ALL, "%s:Begin.", funcName);
    CMsgQueue *msgQptr = CSingleton<CMsgQueue>::Instance();

    int msgid = msgQptr->MsgGet(iIpcKey, IPC_EXCL | IPC_CREAT);
    if (msgid < 0) {
        LOG(LL_WARN, "%s:MsgGet key=%d error.", funcName, iIpcKey);
        return RET_ERROR;
    }
    SMsgBuffer msgBuf;
    msgBuf.iMsgType = iIpcKey;
    string tmpCmdBakStr = "";
    int ii = 0;
    do {
        memset(msgBuf.iMsgStr, 0, sizeof(msgBuf.iMsgStr));
        if (RET_ERROR ==
            msgQptr->MsgRecv(msgid, &msgBuf, sizeof(msgBuf.iMsgStr), IPC_NOWAIT)) {
            DEBUG(LL_WARN, "%s:MsgRecv msgtype:%d error.", funcName, iIpcKey);
            break;
        }
        DEBUG(LL_VARS, "%s:MsgRecv.str=%s", funcName, msgBuf.iMsgStr);
        std::vector<string> msgList;
        // pid|time.secs|desc
        CStrUitls::SplitString(msgBuf.iMsgStr, "|", &msgList);
        if (msgList.size() != 3) {
            continue;
        }
        int pid = CStrUitls::Str2Int(msgList[0]);
        if (pid > 0) {
            iChildsMap[pid] = CStrUitls::Str2Long(msgList[1]);
        }
        if (msgList[2] != "Living" && tmpCmdBakStr != msgList[2]) {
            tmpCmdBakStr = msgList[2];
            iHistoryCmds = tmpCmdBakStr;
            IpcBroadcastChilds(msgList[2]);
        }
    } while (++ii < iMaxChilds * 2);
    DEBUG(LL_ALL, "%s:End.", funcName);
    return RET_OK;
}

// heart beat message or command is sent to parent.
int CProcMaintain::IpcParent(int aFromPid, long aNowSecs,
                             const string& aStr, string* aOut) {
    const char *funcName = "CProcMaintain::IpcParent()";
    DEBUG(LL_ALL, "%s:Begin.", funcName);
    CMsgQueue *msgQptr = CSingleton<CMsgQueue>::Instance();

    int msgid = msgQptr->MsgGet(iIpcKey, IPC_CREAT);
    if (msgid < 0) {
        LOG(LL_WARN, "%s:MsgGet key:%d error.", funcName, iIpcKey);
        return RET_ERROR;
    }
    SMsgBuffer msgBuf;
    msgBuf.iMsgType = iIpcKey;
    sprintf(msgBuf.iMsgStr, "%d|%ld|%s", aFromPid, aNowSecs, aStr.c_str());
    if (RET_ERROR == msgQptr->MsgSend(msgid, &msgBuf, IPC_NOWAIT)) {
        DEBUG(LL_WARN, "%s:MsgSend msgtype:%derror.", funcName, iIpcKey);
        return RET_ERROR;
    }
    memset(msgBuf.iMsgStr, 0, sizeof(msgBuf.iMsgStr));
    msgBuf.iMsgType = aFromPid;
    if (RET_ERROR ==
        msgQptr->MsgRecv(msgid, &msgBuf, sizeof(msgBuf.iMsgStr), IPC_NOWAIT)) {
        DEBUG(LL_WARN, "%s:MsgRecv msgtype:%d error.", funcName, aFromPid);
        return RET_ERROR;
    }
    DEBUG(LL_VARS, "%s:MsgRecv.str=%s", funcName, msgBuf.iMsgStr);
    *aOut = msgBuf.iMsgStr+2;
    DEBUG(LL_ALL, "%s:End.", funcName);
    return RET_OK;
}

// timeout child is erase from the map.
int CProcMaintain::KickTimeoutChilds(int aTimeout) {
    const char *funcName = "CProcMaintain::KickTimeoutChilds()";
    DEBUG(LL_ALL, "%s:Begin.", funcName);

    long nowSec = time(NULL) - aTimeout;
    for (typeof(iChildsMap.end()) it = iChildsMap.begin();
        it != iChildsMap.end(); ++it) {
        if (it->second <= nowSec) {
            if (kill(it->first, 0) != 0) {
                typeof(iChildsMap.end()) tmpit = it;
                ++tmpit;
                LOG(LL_WARN, "%s:erase pid:%d.", funcName, it->first);
                iChildsMap.erase(it);
                it = tmpit;
            }
        }
    }
    DEBUG(LL_ALL, "%s:End.", funcName);
    return GetNeedCounts();
}

// add
void CProcMaintain::AddChilds(int aPid, long aStartSecs) {
    iChildsMap[aPid] = aStartSecs;
    if (!iHistoryCmds.empty()) {
        IpcBroadcastChilds(iHistoryCmds);
    }    
}

// del
void CProcMaintain::DelChilds(int aPid) {
    iChildsMap.erase(iChildsMap.find(aPid));
}
// end of local file.
