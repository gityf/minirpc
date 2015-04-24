#include <unistd.h>
#include <ctime>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include "gtest/gtest.h"
#include "common/ipc/cmsgqueue.h"
#include "common/colorprint.h"
using namespace std;
using namespace wyf;
TEST(CMsgQueuetTest, sendBlock)
{
    CMsgQueue* msgQueue = new CMsgQueue();
    int msgid = msgQueue->MsgGet(getpid(), IPC_EXCL | IPC_CREAT);
    EXPECT_LT(0, msgid);
    SMsgBuffer msgBuf;
    msgBuf.iMsgType = getpid();
    sprintf(msgBuf.iMsgStr, "Msg:Test at:%u", time(NULL));
    for (int i = 0; i < 20; ++i) {
        EXPECT_EQ(RET_OK, msgQueue->MsgSend(msgid, &msgBuf));
    }
    wyf::COLOR_GREEN(cout) << "[ INFO     ] sendBlock:sendstr:" << msgBuf.iMsgStr
        << " type:" << msgBuf.iMsgType
        << " times: 20" 
        << " msgid: " << msgid << endl;
    wyf::COLOR_RESTORE(cout);

    delete msgQueue;
}

TEST(CMsgQueuetTest, set)
{
    CMsgQueue* msgQueue = new CMsgQueue();
    int msgid = msgQueue->MsgGet(getpid(), IPC_CREAT);
    EXPECT_LT(0, msgid);
    char *p = getlogin();
    if (p != NULL && memcmp("root", p, 4) == 0) {
        EXPECT_EQ(RET_OK, msgQueue->MsgSet(msgid, 12345678));
    } else {
        EXPECT_EQ(RET_ERROR, msgQueue->MsgSet(msgid, 12345678));
    }
    
    delete msgQueue;
}

TEST(CMsgQueuetTest, stat)
{
    CMsgQueue* msgQueue = new CMsgQueue();
    int msgid = msgQueue->MsgGet(getpid(), IPC_CREAT);
    EXPECT_LT(0, msgid);
    string statStr = msgQueue->MsgStat(msgid);    
    wyf::COLOR_GREEN(cout) << "[ INFO     ] MSG_STAT:" << statStr << endl;
    wyf::COLOR_RESTORE(cout);
    delete msgQueue;
}

TEST(CMsgQueuetTest, recvBlock)
{
    CMsgQueue* msgQueue = new CMsgQueue();
    int msgid = msgQueue->MsgGet(getpid(), IPC_CREAT);
    EXPECT_LT(0, msgid);
    SMsgBuffer msgBuf;
    msgBuf.iMsgType = getpid();
    for (int i = 0; i < 20; ++i) {
        EXPECT_LT(RET_OK,
            msgQueue->MsgRecv(msgid, &msgBuf, sizeof(msgBuf.iMsgStr)));
    }
    wyf::COLOR_GREEN(cout) << "[ INFO     ] recvBlock:recvstr:" << msgBuf.iMsgStr
        << " type:" << msgBuf.iMsgType
        << " times: 20" 
        << " msgid: " << msgid << endl;
    wyf::COLOR_RESTORE(cout);
    delete msgQueue;
}

TEST(CMsgQueuetTest, sendUnBlock)
{
    CMsgQueue* msgQueue = new CMsgQueue();
    int msgid = msgQueue->MsgGet(getpid(), IPC_EXCL | IPC_CREAT);
    EXPECT_LT(0, msgid);
    SMsgBuffer msgBuf;
    msgBuf.iMsgType = getpid();
    sprintf(msgBuf.iMsgStr, "Msg:Test at:%u", time(NULL));
    for (int i = 0; i < 20; ++i) {
        EXPECT_EQ(RET_OK, msgQueue->MsgSend(msgid, &msgBuf, IPC_NOWAIT));
    }
    wyf::COLOR_GREEN(cout) << "[ INFO     ] sendUnBlock:sendstr:" << msgBuf.iMsgStr
        << " type:" << msgBuf.iMsgType
        << " times: 20" 
        << " msgid: " << msgid << endl;
    wyf::COLOR_RESTORE(cout);

    delete msgQueue;
}

TEST(CMsgQueuetTest, recvUnBlock)
{
    CMsgQueue* msgQueue = new CMsgQueue();
    int msgid = msgQueue->MsgGet(getpid(), IPC_CREAT);
    EXPECT_LT(0, msgid);
    SMsgBuffer msgBuf;
    msgBuf.iMsgType = getpid();
    for (int i = 0; i < 20; ++i) {
        EXPECT_LT(RET_OK,
            msgQueue->MsgRecv(msgid, &msgBuf, sizeof(msgBuf.iMsgStr), IPC_NOWAIT));
    }
    wyf::COLOR_GREEN(cout) << "[ INFO     ] recvUnBlock:recvstr:" << msgBuf.iMsgStr
        << " type:" << msgBuf.iMsgType
        << " times: 20" 
        << " msgid: " << msgid << endl;
    wyf::COLOR_RESTORE(cout);
    delete msgQueue;
}

TEST(CMsgQueuetTest, rm)
{
    CMsgQueue* msgQueue = new CMsgQueue();
    int msgid = msgQueue->MsgGet(getpid(), IPC_CREAT);
    EXPECT_LT(0, msgid);
    EXPECT_EQ(RET_OK, msgQueue->MsgRm(msgid));
    SMsgBuffer msgBuf;
    msgBuf.iMsgType = getpid();
    sprintf(msgBuf.iMsgStr, "Msg:Test at:%u", time(NULL));
    EXPECT_EQ(RET_ERROR, msgQueue->MsgSend(msgid, &msgBuf));
    delete msgQueue;
}
