/*
** Copyright (C) 2014 Wang Yaofu
** All rights reserved.
**
**Author:Wang Yaofu voipman@qq.com
**Description: The source file of class CRpcMsgHandler.
*/
#include "common/rpc/rpc_msghandler.h"
#include <unistd.h>
#include <csignal>
#include "common/atomic/atomic.h"
#include "common/cconfig.h"
#include "common/clogwriter.h"
#include "common/cprocmaintain.h"
#include "common/csingleton.h"
#include "common/csocket.h"
#include "common/cstringutils.h"
#include "common/ev/ceventloop.h"
#include "common/ipc/cmsgqueue.h"
#include "common/rpc/rpc_serverobserver.h"
using namespace wyf;
using namespace std;
// ---------------------------------------------------------------------------
// CRpcMsgHandler::CRpcMsgHandler()
//
// constructor.
// ---------------------------------------------------------------------------
//
CRpcMsgHandler::CRpcMsgHandler()
: iStopRequested(false) {
    iSocket = NULL;
    iRecvBuffer = NULL;
}

// ---------------------------------------------------------------------------
// CRpcMsgHandler::~CRpcMsgHandler()
//
// Destructor.
// ---------------------------------------------------------------------------
//
CRpcMsgHandler::~CRpcMsgHandler() {
    if (iSocket != NULL) {
        delete iSocket;
        iSocket = NULL;
    }
    if (iRecvBuffer != NULL) {
        delete[] iRecvBuffer;
        iRecvBuffer = NULL;
    }
}

// ---------------------------------------------------------------------------
// int CRpcMsgHandler::InitData(string aTclFile)
//
// get the init data from a tcl file.
// ---------------------------------------------------------------------------
//
int CRpcMsgHandler::InitData(const SRpcConfig& aConfig) {
    const char *funcName = "CRpcMsgHandler::InitData()";
    DEBUG(LL_ALL, "%s:Begin.", funcName);
    iRpcConfig = aConfig;
    iConfig = CSingleton<CConfig>::Instance();
    iProcMaintain = CSingleton<CProcMaintain>::Instance();
    iProcMaintain->SetIpcKey(CStrUitls::Int2HexInt(iRpcConfig.iListenPort));
    DEBUG(LL_INFO, "%s:try create socket instance.", funcName);
    iSocket = new CSocket();
    if (iSocket == NULL) {
        LOG(LL_ERROR, "%s:create CSocket failed.", funcName);
        return RET_ERROR;
    }
    iStatisticInfos.iService = iRpcConfig.iServerName;
    iStatisticInfos.iIsUpMonitorInfos = 1;
    DEBUG(LL_DBG, "%s:try initializing UDP CSocket.", funcName);
    iUdpClntAddrInfo.iProtocol = UDP;
    iUdpClntAddrInfo.iAddr = iConfig->GetConfig(CFG_UDP_BIND_IP);
    iUdpClntAddrInfo.iPort = iConfig->GetConfig(CFG_UDP_BIND_PORT);
    DEBUG(LL_DBG, "%s:try call socket.", funcName);
    if (iSocket->CreateSocket(&iUdpClntAddrInfo) == RET_ERROR) {
        LOG(LL_ERROR, "%s:create CSocket.socket failed.", funcName);
        return RET_ERROR;
    }
    DEBUG(LL_DBG, "%s:try initializing statistic Infos...", funcName);
    char tmpStatisticBuf[PKG_MAX_LEN] = {0};
    // service type
    iStatisticInfos.iServiceType = "YF.FY";
    // login name.
    char *p = getlogin();
    iStatisticInfos.iLoginName = (p == NULL) ? "NULL" : p;
    DEBUG(LL_DBG, "%s:get loginName:(%s)",
        funcName, iStatisticInfos.iLoginName.c_str());
    // local ip address.
    if (iRpcConfig.iLocalHostIP == "127.0.0.1") {
        if (iSocket->GetLocalIP(tmpStatisticBuf) == RET_OK) {
            iStatisticInfos.iIpAddress = tmpStatisticBuf;
        } else {
            LOG(LL_ERROR, "%s:get local ip address failed.", funcName);
            return RET_ERROR;
        }
    } else {
        iStatisticInfos.iIpAddress = iRpcConfig.iLocalHostIP;
    }
    // local process listen port.
    iStatisticInfos.iPort = iRpcConfig.iListenPort;
    // local process id.
    iStatisticInfos.iPid = getpid();
    // current connections.
    iStatisticInfos.iCurConns = 0;
    // handled connections.
    iStatisticInfos.iHandledConns = 0;
    // input packages.
    iStatisticInfos.iInputPkgs = 0;
    iStatisticInfos.iLastInputPkgs = 0;
    // input packages tps.
    iStatisticInfos.iInputPkgTps = 0;
    // output packages.
    iStatisticInfos.iOutputPkgs = 0;
    // max requests per child.
    iStatisticInfos.iMaxRequestsPerChild =
        iConfig->GetConfig(CFG_MAX_REQU_PER_CHILD);
    DEBUG(LL_VARS, "%s:iMaxRequestsPerChild=[%d].",
        funcName, iStatisticInfos.iMaxRequestsPerChild);
    // start time.
    iStatisticInfos.iStartTime = time(NULL);
    // running time.
    iStatisticInfos.iRunningTime = 0;

    iRecvBuffer = new char[MAX_SIZE_1M+1];
    if (iRecvBuffer == NULL) {
        LOG(LL_ERROR, "%s:malloc data buffer failed.", funcName);
        return RET_ERROR;
    }
    memset(iRecvBuffer, 0, MAX_SIZE_1M);

    int ii = 0;
    while (ii < MAX_SIZE_1K) {
        iSockIdTimer[ii++] = 0;
    }
    DEBUG(LL_INFO, "%s:try create event loop instance.", funcName);
    iEventLoop = new CEventLoop();
    if (iEventLoop == NULL) {
        LOG(LL_ERROR, "%s:create CEventLoop failed.", funcName);
        return RET_ERROR;
    }
    if (iEventLoop->CreateEventLoop(MAX_SIZE_1K) == RET_ERROR) {
        LOG(LL_ERROR, "%s:CEventLoop.CreateEventLoop failed.", funcName);
        return RET_ERROR;
    }
    iCurFdSize = 0;
    iIsRunningNow = false;
    iSocketTimeOut = iConfig->GetConfig(CFG_SOCK_TIME_OUT);
    iSocketRWTimeOut = iConfig->GetConfig(CFG_SOCK_RW_TIME_OUT);
    DEBUG(LL_ALL, "%s:End.", funcName);
    return RET_OK;
}

int CRpcMsgHandler::IntervalUpdater(const string& aIpcMsg) {
    const char *funcName = "CRpcMsgHandler::IntervalUpdater()";
    DEBUG(LL_ALL, "%s:Begin", funcName);
    string whatOrder = "";
    iProcMaintain->IpcParent(iStatisticInfos.iPid,
        CThreadTimer::instance()->getNowSecs(), aIpcMsg,
        &whatOrder);
    if (!whatOrder.empty()) {
        iRpcSerObserver->ReLoadService(whatOrder);
    }
    DEBUG(LL_ALL, "%s:End", funcName);
    return RET_OK;
}

// ---------------------------------------------------------------------------
// void CRpcMsgHandler::Start()
//
// this is a runner of sub thread message handler.
// ---------------------------------------------------------------------------
//
int CRpcMsgHandler::Start() {
    const char *funcName = "CRpcMsgHandler::Start()";
    DEBUG(LL_ALL, "%s:Begin.", funcName);
    iStopRequested = false;
    LOG(LL_INFO, "%s:try start message handler.", funcName);
    long nowSecs, waitTimeSecs;
    nowSecs = CThreadTimer::instance()->getNowSecs();
    waitTimeSecs = nowSecs;
    waitTimeSecs += 5;
    iStatisticInfos.iRunningTime = nowSecs;
    SendStatisticInfos();
    LOG(LL_INFO, "%s:run into while.", funcName);
    while (!iStopRequested) {
        /*
        if (CIpServer::instance()->is_stopped()) {
            LOG(LL_ERROR, "%s:CIpServer is exit,local worker exit now.", funcName);
            break;
        }
        */
        iEventLoop->MainLoop(AE_WAIT_MS);
        // Statistic infos should be send to agent node by udp.
        nowSecs = CThreadTimer::instance()->getNowSecs();
        if (nowSecs >= waitTimeSecs) {
            waitTimeSecs = nowSecs + 5;
            // 超时检查
            for (int i = 4; i <= iEventLoop->iMaxSockId; i++) {
                //  检查Socket是否有错误
                if (iSockIdTimer[i] != 0 && iSockIdTimer[i] <= nowSecs) {
                    iSockIdTimer[i] = 0;
                    close(i);
                    iEventLoop->DeleteFileEvent(i,
                        AE_READABLE|AE_WRITABLE|0x0f);
                    AtomicDecrement(&iCurFdSize);
                }
            }
            iStatisticInfos.iRunningTime = nowSecs;
            SendStatisticInfos();
            IntervalUpdater();
            if (iStatisticInfos.iMaxRequestsPerChild > 0 &&
                iStatisticInfos.iOutputPkgs >
                iStatisticInfos.iMaxRequestsPerChild) {
                for (int i = 4; i <= iEventLoop->iMaxSockId; i++) {
                    close(i);
                }
                break;
            }
        }
        if (iCurFdSize <= 0) {
            iCurFdSize = 0;
        } else if (iCurFdSize > MAX_SIZE_1K) {
            iCurFdSize = MAX_SIZE_1K;
        }
    }
    LOG(LL_ERROR, "%s:exit itself.", funcName);
    return RET_OK;
}

// ---------------------------------------------------------------------------
// void CRpcMsgHandler::Stop()
//
// stop the sub thread of message handler.
// ---------------------------------------------------------------------------
//
void CRpcMsgHandler::Stop() {
    iStopRequested = true;
}


void CRpcMsgHandler::AddRpcObserver(CRpcSerObserver *aRpcObserver) {
    iRpcSerObserver = aRpcObserver;
}
// ---------------------------------------------------------------------------
// int CRpcMsgHandler::AddSockidIntoPoll(int aSockId)
//
// add socket id into poll.
// ---------------------------------------------------------------------------
//
int CRpcMsgHandler::AddSockidIntoPoll(int aSockId) {
    const char *funcName = "CRpcMsgHandler::AddSockidIntoPoll()";
    DEBUG(LL_ALL, "%s:Begin", funcName);
    LOG(LL_VARS, "%s:sockid:[%d],current socket size:[%d]",
        funcName, aSockId, iCurFdSize);
    long nowSecs = CThreadTimer::instance()->getNowSecs();
    if (iIsRunningNow) {
        if (nowSecs - iHistorySecs >=
            iConfig->GetConfig(CFG_CMD_GIVE_UP_RUNTIME)) {
            // local worker is too busy,the connection is deny.
            LOG(LL_ERROR, "%s:worker is busy, socket [%d] is deny,"
                "give up command:[%s],run time [%lu]s incomplete.",
                funcName, aSockId,
                iRecvBuffer, nowSecs - iHistorySecs);
            close(aSockId);
            return RET_ERROR;
        }
    }

    if (aSockId >= MAX_SIZE_1K) {
        LOG(LL_WARN, "%s:max conns:[%d],cur conns:[%d]",
            funcName, MAX_SIZE_1K, aSockId);
        close(aSockId);
        return RET_OK;
    }
    LOG(LL_INFO, "%s:try set keepalive.", funcName);
    if (aSockId > 0) {
        iSocket->KeepAlive(aSockId, 15);
    }
    LOG(LL_WARN, "%s:cur conns:[%d]", funcName, iCurFdSize);
    if (iEventLoop->AddFileEvent(aSockId, AE_READABLE, this) == RET_ERROR) {
        LOG(LL_ERROR, "%s:CEventLoop.AddFileEvent failed.", funcName);
        return RET_OK;
    }
    iSockIdTimer[aSockId] = iSocketTimeOut + nowSecs;
    AtomicIncrement(&iCurFdSize);
    iStatisticInfos.iHandledConns++;
    LOG(LL_VARS, "%s:End, init timer:%d", funcName, iSocketTimeOut + nowSecs);
    return RET_OK;
}

// ---------------------------------------------------------------------------
// int CRpcMsgHandler::HandleMsg(int aSockId)
//
// receiving data when read event is occurred. then handle it and send response.
// ---------------------------------------------------------------------------
//
int CRpcMsgHandler::HandleMsg(int aSockId) {
    const char *funcName = "CRpcMsgHandler::HandleMsg";
    DEBUG(LL_ALL, "%s:Begin", funcName);
    LOG(LL_WARN, "%s:handle sockid:(%d) request.", funcName, aSockId);
    DEBUG(LL_INFO, "%s:try TcpRecv...", funcName);
    int recvLen = 0;
    if (RET_ERROR == (recvLen = iSocket->TcpRecv(aSockId, iRecvBuffer,
        PKG_HEADER_LEN, iSocketRWTimeOut, true))) {
            LOG(LL_ERROR, "%s:recv data error:(%s).",
                funcName, strerror(errno));
            return RET_ERROR;
    }
    iRecvBuffer[recvLen] = 0x00;
    if (strncasecmp(iRecvBuffer, "pid", 3) == 0) {
        char pidInfo[20] = {0};
        string pidStr = wyf::CStrUitls::ToStringT(iStatisticInfos.iPid);
        int len = iSocket->CreatePkgHeader(0, pidInfo, pidStr);
        if (RET_ERROR == iSocket->TcpSend(aSockId, pidInfo, len, 1, true)) {
            return RET_ERROR;
        }
        return RET_OK;
    }

    // input packages plus 1.
    iStatisticInfos.iInputPkgs++;
    char *requPtr = strchr(iRecvBuffer, '\n');
    if (requPtr == NULL) {
        return RET_ERROR;
    }
    *requPtr++ = 0x00;
    if (RET_ERROR == iRpcSerObserver->RpcRequest(aSockId,
        iRecvBuffer, requPtr, recvLen-(requPtr-iRecvBuffer))) {
        return RET_ERROR;
    }
    DEBUG(LL_ALL, "%s:End", funcName);
    return RET_OK;
}

// ---------------------------------------------------------------------------
// void CRpcMsgHandler::Request(int aSockId)
//
// read event is occurred.
// ---------------------------------------------------------------------------
//
void CRpcMsgHandler::Request(int aSockId) {
    const char *funcName = "CRpcMsgHandler::Request";
    DEBUG(LL_ALL, "%s:Begin", funcName);
    DEBUG(LL_WARN, "%s:sockid:(%d) read set is ok.", funcName, aSockId);
    iHistorySecs = CThreadTimer::instance()->getNowSecs();
    iIsRunningNow = true;
    if (HandleMsg(aSockId) == RET_ERROR) {
        AtomicDecrement(&iCurFdSize);
        iSockIdTimer[aSockId] = 0;
        LOG(LL_ERROR, "%s:HandleMsg sockid:(%d) failed.", funcName, aSockId);
        close(aSockId);
        iEventLoop->DeleteFileEvent(aSockId, AE_READABLE|AE_WRITABLE|0x0f);
    } else {
        // exec ok,update timer.
        long now = CThreadTimer::instance()->getNowSecs();
        iSockIdTimer[aSockId] = now + iSocketTimeOut;
        if (now >= iHistorySecs + iConfig->GetConfig(CFG_CMD_WARN_RUNTIME)) {
            LOG(LL_ERROR, "%s:run-time:(%d)secs,cmd:(%s)", funcName,
                now-iHistorySecs, iRecvBuffer);
        }
    }
    iIsRunningNow = false;
    DEBUG(LL_ALL, "%s:End", funcName);
}

// ---------------------------------------------------------------------------
// void CHttpHandler::Response(int aSockId)
//
// write event is occurred.
// ---------------------------------------------------------------------------
//
void CRpcMsgHandler::Response(int aSockId) {
    const char *funcName = "CRpcMsgHandler::Response";
    DEBUG(LL_ALL, "%s:Begin", funcName);
    LOG(LL_WARN, "%s:sockid:(%d) write set is ok.",
        funcName, aSockId);
    ErrSocket(aSockId);
    DEBUG(LL_ALL, "%s:End", funcName);
}

// ---------------------------------------------------------------------------
// void CRpcMsgHandler::ErrSocket(int aSockId)
//
// error event is occurred.
// ---------------------------------------------------------------------------
//
void CRpcMsgHandler::ErrSocket(int aSockId) {
    const char *funcName = "CHttpHandler::ErrSocket";
    DEBUG(LL_ALL, "%s:Begin", funcName);
    LOG(LL_WARN, "%s:sockid:(%d) err set is ok.",
        funcName, aSockId);
    iSockIdTimer[aSockId] = 0;
    AtomicDecrement(&iCurFdSize);
    close(aSockId);
    LOG(LL_DBG, "%s:try delete sockid:(%d) from poll.",
        funcName, aSockId);
    iEventLoop->DeleteFileEvent(aSockId, AE_READABLE|AE_WRITABLE|0x0f);
    LOG(LL_DBG, "%s:delete sockid:(%d) from poll sucess.",
        funcName, aSockId);
    DEBUG(LL_ALL, "%s:End", funcName);
}

void CRpcMsgHandler::IncrOutPks() {
    iStatisticInfos.iOutputPkgs++;
}

void CRpcMsgHandler::AddVectorDesc(const string& aVecterDesc) {
    iStatisticInfos.iVectorInfo = aVecterDesc;
    // every library is ipc to all process.
    IntervalUpdater(aVecterDesc);
}
// ---------------------------------------------------------------------------
// void CRpcMsgHandler::SendStatisticInfos()
//
// sending statistic Informations.
// ---------------------------------------------------------------------------
//
void CRpcMsgHandler::SendStatisticInfos() {
    if (iStatisticInfos.iIsUpMonitorInfos == 0) {
        return;
    }
    iStatisticInfos.iCurConns = iCurFdSize;
    iStatisticInfos.MakeStatisticInfos();
    iStatisticInfos.MakePkg(iRecvBuffer, iStatisticInfos.iFormatStr,
        PKG_CMD_REGISTER);
    if (RET_ERROR == iSocket->UdpSend(iUdpClntAddrInfo.iSockId, iRecvBuffer,
        strlen(iRecvBuffer), iSocketRWTimeOut, &iUdpClntAddrInfo.iSockUnion)) {
        LOG(LL_ERROR, "SendStatisticInfos():statistic infos send error.");
    }
}

// end of local file.
