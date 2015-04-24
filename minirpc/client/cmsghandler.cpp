/*
** Copyright (C) 2014 Wang Yaofu
** All rights reserved.
**
**Description: The source file of class CMsgHandler.
*/
#include <signal.h>
#include <unistd.h>
#include <string>
#include "common/cconfig.h"
#include "common/clogwriter.h"
#include "common/csocket.h"
#include "client/cclientsession.h"
#include "client/cmsghandler.h"
#include "common/csingleton.h"

// ---------------------------------------------------------------------------
// CMsgHandler::CMsgHandler()
//
// constructor.
// ---------------------------------------------------------------------------
//
CMsgHandler::CMsgHandler() {
    iSocket = NULL;
    iRecvBuffer = NULL;
    iIsInitialed = false;
}

// ---------------------------------------------------------------------------
// CMsgHandler::~CMsgHandler()
//
// Destructor.
// ---------------------------------------------------------------------------
//
CMsgHandler::~CMsgHandler() {
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
// int CMsgHandler::InitData(struct CConfig aConfigInfo)
//
// init operation where local class is created.
// ---------------------------------------------------------------------------
//
int CMsgHandler::InitData() {
    DEBUG_FUNC_NAME("CMsgHandler::InitData");
    DEBUG(LL_ALL, "%s::Begin.", funcName);
    if (iIsInitialed) {
        DEBUG(LL_WARN, "%s:has initialized.", funcName);
        return RET_OK;
    }
    iErrString = "orbexec exec failed.";
    iConfig = CSingleton<CConfig>::Instance();
    char *home = NULL;
    if ((home = getenv("AIIP_HOME")) != NULL) {
        string frameclnPath = home;
        frameclnPath += "/etc/framecln.ini";
        iConfig->InitConfig(frameclnPath.c_str());
    }
    if (ConfigInitial() == RET_ERROR) {
        return RET_ERROR;
    }

    iRecvBuffer = new char[MAX_SIZE_1M];
    if (iRecvBuffer == NULL) {
        LOG(LL_ERROR, "%s:malloc data buffer failed.", funcName);
        return RET_ERROR;
    }
    memset(iRecvBuffer, 0, MAX_SIZE_1M);
    if ((iSocket = new CSocket()) == NULL) {
        LOG(LL_ERROR, "%s:create CSocket error.", funcName);
        return RET_ERROR;
    }
    if ((iClientSession = CSingleton<CClientSession>::Instance()) == NULL) {
        LOG(LL_ERROR, "%s:create CClientSession error.", funcName);
        return RET_ERROR;
    }
    iSocketRWTimeOut =
        iConfig->GetConfig(CFG_SOCK_RW_TIME_OUT);
    if (RET_ERROR == iClientSession->InitData()) {
        LOG(LL_ERROR, "%s:CClientSession.InitData error.", funcName);
        return RET_ERROR;
    }
    DEBUG(LL_ALL, "%s:End", funcName);
    iIsInitialed = true;
    return RET_OK;
}

// ---------------------------------------------------------------------------
// int CMsgHandler::ConfigInitial()
//
// configuration service is initialized.
// ---------------------------------------------------------------------------
//
int CMsgHandler::ConfigInitial() {
    DEBUG_FUNC_NAME("CMsgHandler::ConfigInitial()");
    DEBUG(LL_ALL, "%s:Begin", funcName);
    if (iConfig->ReadAllConfig() == RET_ERROR) {
        LOG(LL_ERROR, "%s:read all config error.", funcName);
    }
    string tmpStr = iConfig->GetCfgStr("GLOBAL.LOG_FILE", "./log");
    if (tmpStr.find_last_of('/') > 0) {
        string tmpLogDir = tmpStr.substr(0, tmpStr.find_last_of('/'));
        if (access(tmpLogDir.c_str(), R_OK|W_OK|X_OK) < 0) {
            LOG(LL_ERROR, "%s:log path:(%s) open failed.",
                funcName, tmpLogDir.c_str());
            fprintf(stderr, "log path:(%s) open failed.", tmpLogDir.c_str());
            return RET_ERROR;
        }
        CSingleton<CLogWriter>::Instance()->SetLogFileName(tmpStr);
    }
    int tmpInt = 0;
    tmpInt = iConfig->GetCfgInt("GLOBAL.LOG_LEVEL", CFG_LOG_LEVEL);
    iConfig->SetConfig(CFG_LOG_LEVEL, tmpInt);
    CSingleton<CLogWriter>::Instance()->SetLogLevel(tmpInt);
    LOG(LL_VARS, "%s:GLOBAL.LOG_LEVEL=[%d]", funcName, tmpInt);
    char *home = NULL;
    if ((home = getenv("UDP_AGENT_PORT")) == NULL) {
        LOG(LL_ERROR, "%s:miss UDP_AGENT_PORT,get GLOBAL.UDP_AGENT_PORT.",
            funcName);
        tmpInt = iConfig->GetCfgInt("GLOBAL.UDP_AGENT_PORT", CFG_UDP_BIND_PORT);
    } else {
        tmpInt = atoi(home);
    }
    iConfig->SetConfig(CFG_UDP_BIND_PORT, tmpInt);
    if ((home = getenv("TCP_AGENT_PORT")) == NULL) {
        LOG(LL_ERROR,
            "%s:TCP_AGENT_PORT not set, try to get GLOBAL.TCP_AGENT_PORT.",
            funcName);
        tmpInt = iConfig->GetCfgInt("GLOBAL.TCP_AGENT_PORT", CFG_UDP_BIND_PORT);
    } else {
        tmpInt = atoi(home);
    }
    iConfig->SetConfig(CFG_TCP_BIND_PORT, tmpInt);
    LOG(LL_VARS, "%s:GLOBAL.TCP_AGENT_PORT=[%d]", funcName, tmpInt);

    if ((home = getenv("UDP_AGENT_IP")) == NULL) {
        LOG(LL_ERROR,
            "%s:UDP_AGENT_IP not set, try to get GLOBAL.UDP_AGENT_IP.",
            funcName);
        tmpStr = iConfig->GetCfgStr("GLOBAL.UDP_AGENT_IP", CFG_UDP_BIND_IP);
    } else {
        tmpStr = home;
    }
    if (tmpStr.size() < 7) {
        LOG(LL_ERROR, "%s:UDP_AGENT_IP get failed.", funcName);
        fprintf(stderr, "get 'UDP_AGENT_IP' failed.");
        return RET_ERROR;
    }
    iConfig->SetConfig(CFG_UDP_BIND_IP, tmpStr);
    LOG(LL_VARS, "%s:GLOBAL.UDP_AGENT_IP=[%s]", funcName, tmpStr.c_str());

    if ((home = getenv("TCP_AGENT_IP")) == NULL) {
        LOG(LL_ERROR,
            "%s:TCP_AGENT_IP not set, try to get GLOBAL.TCP_AGENT_IP.",
            funcName);
        tmpStr = iConfig->GetCfgStr("GLOBAL.TCP_AGENT_IP", CFG_TCP_BIND_IP);
    } else {
        tmpStr = home;
    }
    if (tmpStr.size() < 7) {
        LOG(LL_ERROR, "%s:TCP_AGENT_IP get failed.", funcName);
        fprintf(stderr, "get 'TCP_AGENT_IP' failed.");
        return RET_ERROR;
    }
    iConfig->SetConfig(CFG_TCP_BIND_IP, tmpStr);
    LOG(LL_VARS, "%s:GLOBAL.TCP_AGENT_IP=[%s]", funcName, tmpStr.c_str());

    tmpInt = iConfig->GetCfgInt("GLOBAL.SOCKET_TIME_OUT", CFG_SOCK_TIME_OUT);
    iConfig->SetConfig(CFG_SOCK_TIME_OUT, tmpInt);
    LOG(LL_VARS, "%s:GLOBAL.SOCKET_TIME_OUT=[%d]", funcName, tmpInt);

    tmpInt = iConfig->GetCfgInt("GLOBAL.SOCKET_RW_TIME_OUT",
        CFG_SOCK_RW_TIME_OUT);
    iConfig->SetConfig(CFG_SOCK_RW_TIME_OUT, tmpInt);
    LOG(LL_VARS, "%s:GLOBAL.SOCKET_RW_TIME_OUT=[%d]", funcName, tmpInt);
    DEBUG(LL_ALL, "%s:End", funcName);
    return RET_OK;
}

// ---------------------------------------------------------------------------
// int CMsgHandler::CmdExec()
//
// execute command.
// ---------------------------------------------------------------------------
//
int CMsgHandler::CmdExec(const char *aCmd, const char* aService) {
    DEBUG_FUNC_NAME("CMsgHandler::CmdExec");
    DEBUG(LL_ALL, "%s:Begin.", funcName);
    if (aCmd == NULL || aService == NULL) {
        LOG(LL_ERROR, "%s:buffer is null!!!", funcName);
        return RET_ERROR;
    }
    int cmdLen = strlen(aCmd);
    if (cmdLen <= 0 || cmdLen > MAX_SIZE_1M || strlen(aService) == 0) {
        LOG(LL_ERROR, "%s:bad buffer length!!!", funcName);
        iErrString = "Too large buffer size.";
        return RET_ERROR;
    }
    LOG(LL_DBG, "%s:cmd:(%s),service:(%s)",
        funcName, aCmd, aService);
    int ret, trytime = 3, theSockId;
    do {
        ret = iClientSession->GetSockIdByService(aService);
    } while (ret == RET_ERROR && trytime-- > 0);
    if (ret == RET_ERROR) {
        LOG(LL_ERROR, "%s:service:(%s) is not found.", funcName, aService);
        iErrString = "can not found service ";
        iErrString += aService;
        return RET_ERROR;
    }
    theSockId = ret;
    int requLen = iSocket->CreatePkgHeader(0, iRecvBuffer, aCmd);
    iRecvBuffer[requLen] = 0x00;
    DEBUG(LL_INFO, "%s:try TcpSend...", funcName);
    trytime = 3;
    while (trytime-- > 0) {
        ret = iSocket->TcpSend(theSockId, iRecvBuffer,
            requLen, iSocketRWTimeOut, true);
        if (ret != RET_ERROR) {
            break;
        } else {
            int connTryTimes = 3;
            iClientSession->EraseServiceSock(theSockId);
            do {
                ret = iClientSession->GetSockIdByService(aService);
            } while (ret == RET_ERROR && connTryTimes-- > 0);
            if (ret == RET_ERROR) {
                LOG(LL_ERROR, "%s:service:(%s) is not found.",
                    funcName, aService);
                continue;
            }
            theSockId = ret;
        }
    }
    if (ret == RET_ERROR) {
        LOG(LL_ERROR, "%s:send to service:(%s) error:(%s).",
            funcName, aService, strerror(errno));
        iErrString = "send requ error to service ";
        iErrString += aService;
        iErrString += ",make sure service type is 'YF.FY'.";
        KickoffService(aService, theSockId);
        return RET_ERROR;
    }

    DEBUG(LL_INFO, "%s:try TcpRecv...", funcName);
    if ((ret = iSocket->TcpRecv(theSockId, iRecvBuffer,
        PKG_HEADER_LEN, iSocketRWTimeOut)) == RET_ERROR) {
        LOG(LL_ERROR, "%s:recv service:(%s) error:(%s).",
            funcName, aService, strerror(errno));
        iErrString = "no response from service ";
        iErrString += aService;
        KickoffService(aService, theSockId);
        return RET_ERROR;
    }
    DEBUG(LL_VARS, "%s:recv decode buffer:(%s).",
        funcName, iRecvBuffer);
    DEBUG(LL_ALL, "%s:End", funcName);
    iClientSession->AddSockIdByService(aService, theSockId);
    return RET_OK;
}

// ---------------------------------------------------------------------------
// int CMsgHandler::CmdExecV()
//
// execute command.
// ---------------------------------------------------------------------------
//
int CMsgHandler::CmdExecV(const char *aCmd, int aLen, const char *aService) {
    DEBUG_FUNC_NAME("CMsgHandler::CmdExec");
    DEBUG(LL_ALL, "%s:Begin.", funcName);
    if (aCmd == NULL || aService == NULL) {
        LOG(LL_ERROR, "%s:buffer is null!!!", funcName);
        return RET_ERROR;
    }
    if (aLen <= 0 || aLen >= MAX_SIZE_1M || strlen(aService) == 0)
    {
        LOG(LL_ERROR, "%s:bad buffer length!!!", funcName);
        iErrString = "Too large buffer size.";
        return RET_ERROR;
    }
    LOG(LL_DBG, "%s:cmd:(%s),service:(%s)",
        funcName, aCmd, aService);
    int ret, trytime = 3, theSockId;
    do {
        ret = iClientSession->GetSockIdByService(aService);
    } while (ret == RET_ERROR && trytime-- > 0);
    if (ret == RET_ERROR) {
        LOG(LL_ERROR, "%s:service:(%s) is not found.",
            funcName, aService);
        iErrString = "can not found service ";
        iErrString += aService;
        return RET_ERROR;
    }
    theSockId = ret;
    ret = iSocket->CreatePkgHeader(aLen, iRecvBuffer);
    INIT_IOV(2);
    SET_IOV_LEN(iRecvBuffer, ret);
    SET_IOV_LEN(aCmd, aLen);
    DEBUG(LL_INFO, "%s:try TcpSend...", funcName);
    trytime = 3;
    while (trytime-- > 0) {
        ret = iSocket->TcpSendV(theSockId, iovs, 2, iSocketRWTimeOut);
        if (ret != RET_ERROR) {
            break;
        } else {
            int connTryTimes = 3;
            iClientSession->EraseServiceSock(theSockId);
            do {
                ret = iClientSession->GetSockIdByService(aService);
            } while (ret == RET_ERROR && connTryTimes-- > 0);
            if (ret == RET_ERROR) {
                LOG(LL_ERROR, "%s:service:(%s) is not found.",
                    funcName, aService);
                continue;
            }
            theSockId = ret;
        }
    }
    if (ret == RET_ERROR) {
        LOG(LL_ERROR, "%s:send to service:(%s) error:(%s).",
            funcName, aService, strerror(errno));
        iErrString = "send request to service ";
        iErrString += aService;
        iErrString += "failed, make sure service type is 'YF.FY'.";
        KickoffService(aService, theSockId);
        return RET_ERROR;
    }

    DEBUG(LL_INFO, "%s:try TcpRecv...", funcName);
    if ((ret = iSocket->TcpRecv(theSockId, iRecvBuffer,
        PKG_HEADER_LEN, iSocketRWTimeOut)) == RET_ERROR) {
        LOG(LL_ERROR, "%s:recv service:(%s) error:(%s).",
            funcName, aService, strerror(errno));
        iErrString = "no response from service ";
        iErrString += aService;
        KickoffService(aService, theSockId);
        return RET_ERROR;
    }
    DEBUG(LL_VARS, "%s:recv decode buffer:(%s).",
        funcName, iRecvBuffer);
    DEBUG(LL_ALL, "%s:End", funcName);
    iClientSession->AddSockIdByService(aService, theSockId);
    return RET_OK;
}

int CMsgHandler::SyncRun(const string& aCmd,
                         const string& aService,
                         const string& aSubService) {
    DEBUG_FUNC_NAME("CMsgHandler::CmdExec");
    DEBUG(LL_ALL, "%s:Begin.", funcName);
    LOG(LL_DBG, "%s:cmd:(%s),host service name:(%s), subservice:(%s)",
        funcName, aCmd.c_str(), aService.c_str(), aSubService.c_str());
    int ret, trytime = 3, theSockId;
    do {
        ret = iClientSession->GetSockIdByService(aService);
    } while (ret == RET_ERROR && trytime-- > 0);
    if (ret == RET_ERROR) {
        LOG(LL_ERROR, "%s:service:(%s) is not found.",
            funcName, aService);
        iErrString = "can not found service ";
        iErrString += aService;
        return RET_ERROR;
    }
    theSockId = ret;
    char header[MAX_SIZE_1K] = {0};
    ret = iSocket->CreatePkgHeader(aCmd.length()+1, header, aSubService);
    header[ret++] = '\n';
    INIT_IOV(2);
    SET_IOV_LEN(header, ret);
    SET_IOV_LEN(aCmd.c_str(), aCmd.length());
    DEBUG(LL_INFO, "%s:try TcpSend...", funcName);
    trytime = 3;
    while (trytime-- > 0) {
        ret = iSocket->TcpSendV(theSockId, iovs, 2, iSocketRWTimeOut, true);
        if (ret != RET_ERROR) {
            break;
        } else {
            int connTryTimes = 3;
            iClientSession->EraseServiceSock(theSockId);
            do {
                ret = iClientSession->GetSockIdByService(aService);
            } while (ret == RET_ERROR && connTryTimes-- > 0);
            if (ret == RET_ERROR) {
                LOG(LL_ERROR, "%s:service:(%s) is not found.",
                    funcName, aService);
                continue;
            }
            theSockId = ret;
        }
    }
    if (ret == RET_ERROR) {
        LOG(LL_ERROR, "%s:send to service:(%s) error:(%s).",
            funcName, aService, strerror(errno));
        iErrString = "send request to service ";
        iErrString += aService;
        iErrString += "failed, make sure service type is 'YF.FY'.";
        KickoffService(aService, theSockId);
        return RET_ERROR;
    }

    DEBUG(LL_INFO, "%s:try TcpRecv...", funcName);
    if ((ret = iSocket->TcpRecv(theSockId, iRecvBuffer,
        PKG_HEADER_LEN, iSocketRWTimeOut)) == RET_ERROR) {
            LOG(LL_ERROR, "%s:recv service:(%s) error:(%s).",
                funcName, aService, strerror(errno));
            iErrString = "no response from service ";
            iErrString += aService;
            KickoffService(aService, theSockId);
            return RET_ERROR;
    }
    iClientSession->AddSockIdByService(aService, theSockId);
    DEBUG(LL_ALL, "%s:End", funcName);
    return ret;
}

// kick off service that is not working well.
void CMsgHandler::KickoffService(const string& aService, int aSockId) {
    struct SAddrInfo addrInfo;
    addrInfo.iSockId = aSockId;
    if (iClientSession->GetSockAddrInfo(&addrInfo) == RET_OK) {
        char logoutStr[100] = {0};
        // LOGOUT {HN}   {UN}      {SN}  {IP}        {port} {PID}
        snprintf(logoutStr, sizeof(logoutStr), "LOGOUT {HN} {UN} {%s} {%s} {%d} {%s}",
            aService.c_str(), addrInfo.iAddr.c_str(), addrInfo.iPort, addrInfo.iPeerKey.c_str());
        iClientSession->LoginAndLogout(logoutStr, string(PKG_CMD_CANCEL));
    }
    iClientSession->EraseServiceSock(aSockId);
    aSockId = iClientSession->ReNewService(aService);
    if (aSockId > 0) {
        iClientSession->AddSockIdByService(aService, aSockId);
    }
}
// ---------------------------------------------------------------------------
// char *CMsgHandler::GetResult()
//
// return result after exec cmd.
// ---------------------------------------------------------------------------
//
char *CMsgHandler::GetResult() {
    //printf("Thread %lu has addr:%p\n", ThisThread::GetThreadId(), iRecvBuffer);
    return iRecvBuffer;
}
// end of local file.
