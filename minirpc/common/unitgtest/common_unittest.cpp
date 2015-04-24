#include <unistd.h>
#include <ctime>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include "gtest/gtest.h"
#include "common/cconfig.h"
#include "common/cprocess.h"
#include "common/csingleton.h"
#include "common/cstringutils.h"
#include "common/ipc/cmsgqueue.h"
#include "common/colorprint.h"
using namespace std;
using namespace wyf;
CConfig *iConfig = CSingleton<CConfig>::Instance();
bool SetGetString()
{
    string key   = "test key";
    string value = "test value";
    iConfig->SetConfig(key, value);
    string retStr = iConfig->GetConfig(key);
    return retStr == value;
}

bool SetGetDefaultString()
{
    string key   = "test key";
    string value = "test value";
    iConfig->SetConfig(key, value);
    string retStr = iConfig->GetCfgStr(key, value);
    return retStr == value;
}

bool SetGetDefaultCharChar()
{
    const char* key   = "test key char";
    const char* value = "test value char char";
    iConfig->SetConfig(key, value);
    string retStr = iConfig->GetCfgStr(key, value);
    return retStr == value;
}

bool SetGetInt()
{
    int key   = 0x1234;
    int value = 0x4321;
    iConfig->SetConfig(key, value);
    int ret = iConfig->GetConfig(key);
    return ret == value;
}

bool SetGetDefaultInt()
{
    int key   = 1234;
    int value = 0x4321;
    iConfig->SetConfig(key, value);
    string keyStr = "1234";
    int ret = iConfig->GetCfgInt(keyStr, value);
    return ret == value;
}

bool SetGetDefaultCharInt()
{
    int key   = 1234;
    int value = 0x4321;
    iConfig->SetConfig(key, value);
    const char* keyPtr = "1234";
    int ret = iConfig->GetCfgInt(keyPtr, value);
    return ret == value;
}

TEST(CConfigTest, SetGetConfig)
{
    EXPECT_EQ(true, SetGetString());
    EXPECT_EQ(true, SetGetInt());  
}

TEST(CConfigTest, SetGetDefaultConfig)
{
    EXPECT_EQ(true, SetGetDefaultString());
    EXPECT_EQ(true, SetGetDefaultInt());
}

TEST(CConfigTest, SetGetDefaultConfigChar)
{
    EXPECT_EQ(true, SetGetDefaultCharChar());
    EXPECT_EQ(true, SetGetDefaultCharInt());
}

TEST(CProcessTest, RunShellTest)
{
    CProcess *proc = CSingleton<CProcess>::Instance();
    string shell = "ps -ef | grep common_unittest | wc -l";
    string f = "test.shell.res";
    string ress = proc->RunShell(shell, f);

    EXPECT_LT(2, atoi(ress.c_str()));
}

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

// CStringUtils Unit test
TEST(CStringUtilsTest, Basic)
{
    string format = "123:44.56:strstrtr:0123";
    string format_str;
    CStrUitls::Format(format_str, "%d:%.2f:%s:%04d", 123, 44.56, "strstrtr", 123);
    EXPECT_EQ(format, format_str);
    EXPECT_EQ(true, CStrUitls::IsUnderLine('_'));
    EXPECT_EQ(true, CStrUitls::IsLetter('a'));
    EXPECT_EQ(true, CStrUitls::IsLetter('z'));
    EXPECT_EQ(true, CStrUitls::IsLetter('A'));
    EXPECT_EQ(true, CStrUitls::IsLetter('Z'));
    EXPECT_EQ(false, CStrUitls::IsLetter('1'));
    EXPECT_EQ(true, CStrUitls::IsNumber('1'));
    EXPECT_EQ(true, CStrUitls::IsNumber('0'));
    EXPECT_EQ(true, CStrUitls::IsNumber('9'));
    EXPECT_EQ(false, CStrUitls::IsNumber('@'));
    EXPECT_EQ("0ABC123E", CStrUitls::HexFormatStr("\x0a\xbc\x12\x3e", 4, true));
    EXPECT_EQ("0abc123e", CStrUitls::HexFormatStr("\x0a\xbc\x12\x3e"));
    std::vector<string> vvss;
    vvss.push_back("11");
    vvss.push_back("22");
    vvss.push_back("3333");
    EXPECT_EQ("11&22&3333", CStrUitls::JoinStr(vvss, '&'));
    EXPECT_EQ("11&&22&&3333", CStrUitls::JoinStr(vvss, "&&"));
    EXPECT_EQ("abcfdgf123", CStrUitls::ToLowerCase("aBCfDGF123"));
    EXPECT_EQ("ABCFDGF123", CStrUitls::ToUpperCase("aBCfdgf123"));
    EXPECT_EQ(12345678901, CStrUitls::Str2Long("12345678901"));
    EXPECT_EQ(1234567890, CStrUitls::Str2Int("1234567890"));
}