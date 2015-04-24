/*
** Copyright (C) 2014 Wang Yaofu
** All rights reserved.
**
**Author:Wang Yaofu voipman@qq.com
**Description: The source file of class CMsgQueue.
*/
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <string>
#include "common/ipc/cmsgqueue.h"
namespace wyf {

CMsgQueue::CMsgQueue() {
}

CMsgQueue::~CMsgQueue() {
}

int CMsgQueue::InitData() {
    return RET_OK;
}

int CMsgQueue::MsgGet(int aKey, int aFlags) {
    mode_t um, mode;
    int msgid = 0;
    um = umask(077);
    umask(um);
    mode = 0666 & ~um;
    do {
        msgid = msgget(aKey, aFlags|mode);
        aFlags |= IPC_CREAT;
        // erase flag IPC_EXCL when msgqueue exist.
        if (errno == EEXIST) aFlags &= ~IPC_EXCL;
    } while (msgid == RET_ERROR && errno == EEXIST);
    if (msgid == RET_ERROR) {
        fprintf(stderr, "MsgGet:semget(key:%d,error:(%s)\n",
            aKey, strerror(errno));
        return RET_ERROR;
    }
    return msgid;
}

int CMsgQueue::MsgSend(int aMsgId, const SMsgBuffer* aMsgBuffer, int aFlags) {
    if (aMsgBuffer == NULL || aMsgId <= 0) {
        return RET_ERROR;
    }   
    int ret = RET_ERROR;
    do {
        ret = msgsnd(aMsgId, aMsgBuffer, strlen(aMsgBuffer->iMsgStr), aFlags);
    } while (ret == RET_ERROR && errno == EINTR);
    return ret;
}

int CMsgQueue::MsgRecv(int aMsgId, SMsgBuffer* aMsgBuffer,
                       int aLen, int aFlags) {
    if (aMsgBuffer == NULL || aLen <= 0 || aMsgId <= 0) {
        return RET_ERROR;
    }
    int ret = RET_ERROR;
    long type = aMsgBuffer->iMsgType;
    do {
        ret = msgrcv(aMsgId, aMsgBuffer, aLen, type, aFlags);
    } while (ret == RET_ERROR && errno == EINTR);
    return ret;
}

int CMsgQueue::MsgRm(int aMsgId) {
    int ret = RET_ERROR;
    do {
        ret = msgctl(aMsgId, IPC_RMID, NULL);
    } while (ret == RET_ERROR && errno == EINTR);
    return ret;
}

int CMsgQueue::MsgSet(int aMsgId, int aQBytes) {
    int ret = RET_ERROR;
    struct msqid_ds queueStat;
    memset(&queueStat, 0, sizeof(queueStat));
    do {
        ret = msgctl(aMsgId, IPC_STAT, &queueStat);
    } while (ret == RET_ERROR && errno == EINTR);
    if (ret == RET_OK) {
        queueStat.msg_qbytes = aQBytes;
        // mode should not be changed.
        // queueStat.msg_perm.mode = aMode;
        // IPC_SET must be execed by root.
        return msgctl(aMsgId, IPC_SET, &queueStat);
    }
    return ret;
}

const string CMsgQueue::MsgStat(int aMsgId) {
    int ret = RET_ERROR;
    struct msqid_ds queueStat;
    memset(&queueStat, 0, sizeof(queueStat));
    do {
        ret = msgctl(aMsgId, IPC_STAT, &queueStat);
    } while (ret == RET_ERROR && errno == EINTR);
    if (ret == RET_OK) {
        char statBuf[100] = {0};
        char modeBuf[10] = {0};
        snprintf(statBuf, sizeof(statBuf),
            "TYPE:q; KEY:%x; MODE:%s; OWNER:%d; GROUP:%d; "
            "CBYTES:%d; QNUM:%d; QBYTES:%d.",
            aMsgId,
            GetFileMode(queueStat.msg_perm.mode, modeBuf),
            queueStat.msg_perm.uid,
            queueStat.msg_perm.gid,
            queueStat.msg_cbytes,
            queueStat.msg_qnum,
            queueStat.msg_qbytes);
        return statBuf;
    }
    return string(" ");
}

const char* CMsgQueue::GetFileMode(mode_t aMode, char* aResp) {
    if (aResp == NULL)  return "---------";
    aResp[0] = aMode & S_IRUSR ? 'r' : '-';
    aResp[1] = aMode & S_IWUSR ? 'w' : '-';
    aResp[2] = aMode & S_IXUSR ? 'x' : '-';
    aResp[3] = aMode & S_IRGRP ? 'r' : '-';
    aResp[4] = aMode & S_IWGRP ? 'w' : '-';
    aResp[5] = aMode & S_IXGRP ? 'x' : '-';
    aResp[6] = aMode & S_IROTH ? 'r' : '-';
    aResp[7] = aMode & S_IWOTH ? 'w' : '-';
    aResp[8] = aMode & S_IXOTH ? 'x' : '-';
    aResp[9] = 0x00;
    return aResp;
}

} // namespace wyf
// end of local file.
