/*
** Copyright (C) 2014 Wang Yaofu
** All rights reserved.
**
**Author:Wang Yaofu voipman@qq.com
**Description: The source file of class CRpcIpServer.
*/
#include <unistd.h>
#include <csignal>
#include <vector>
#include "common/cconfig.h"
#include "common/clogwriter.h"
#include "common/cprocess.h"
#include "common/cprocmaintain.h"
#include "common/csetproctitle.h"
#include "common/csingleton.h"
#include "common/cstringutils.h"
#include "common/ipc/cmsgqueue.h"
#include "common/localdef.h"
#include "common/rpc/rpc_ipserver.h"
#include "common/rpc/rpc_msghandler.h"
using namespace wyf;
using namespace std;

// ---------------------------------------------------------------------------
// CRpcIpServer::CRpcIpServer()
//
// constructor.
// ---------------------------------------------------------------------------
//
CRpcIpServer::CRpcIpServer() : iStopRequested(false) {
    iSocket = NULL;
    iRpcMsgHandler = NULL;
    iProcMaintain = NULL;
}

// ---------------------------------------------------------------------------
// CRpcIpServer::~CRpcIpServer()
//
// destructor.
// ---------------------------------------------------------------------------
//
CRpcIpServer::~CRpcIpServer() {
    if (iSocket != NULL) {
        delete iSocket;
        iSocket = NULL;
    }
    if (iRpcMsgHandler != NULL) {
        delete iRpcMsgHandler;
        iRpcMsgHandler = NULL;
    }
}

// ---------------------------------------------------------------------------
// void CRpcIpServer::run()
//
// this is a runner of local thread.
// ---------------------------------------------------------------------------
//
void CRpcIpServer::run() {
    while (!iStopRequested) {
        WorkerAcceptStart();
    }
}

// ---------------------------------------------------------------------------
// void CRpcIpServer::on_stop()
//
// stop the local thread.
// ---------------------------------------------------------------------------
//
void CRpcIpServer::on_stop() {
    LOG(LL_ALL, "CRpcIpServer::on_stop()::Begin");
    iStopRequested = true;
    LOG(LL_ALL, "CRpcIpServer::on_stop()::End");
}

// ---------------------------------------------------------------------------
// void CRpcIpServer::InitData()
//
// init operation, get configuration, initializing socket server.
// ---------------------------------------------------------------------------
//
int CRpcIpServer::InitData(struct SRpcConfig aIfSerConfig) {
    const char *funcName = "CRpcIpServer::InitData()";
    DEBUG(LL_ALL, "%s:Begin.", funcName);
    DEBUG(LL_DBG, "%s:try initializing config.", funcName);
    iRpcConfig = aIfSerConfig;
    iConfig = CSingleton<CConfig>::Instance();
    if (iConfig == NULL) {
        LOG(LL_ERROR, "%s:create CConfig failed.", funcName);
        return RET_ERROR;
    }
    if (RET_ERROR == ConfigInitial()) {
        LOG(LL_ERROR, "%s:get config error.", funcName);
        return RET_ERROR;
    }
    iMainPid = getpid();
    iRpcConfig.iMainPid = iMainPid;
    iProcMaintain = CSingleton<CProcMaintain>::Instance();
    iProcMaintain->SetIpcKey(CStrUitls::Int2HexInt(iRpcConfig.iListenPort));
    iProcMaintain->SetMaxChilds(iRpcConfig.iHandlerCounts);
    DEBUG(LL_INFO, "%s:try start tcp server.", funcName);
    iSocket = new CSocket();
    if (iSocket == NULL) {
        LOG(LL_ERROR, "%s:create CSocket failed.", funcName);
        return RET_ERROR;
    }
    iTcpSerAddrInfo.iProtocol = TCP;
    iTcpSerAddrInfo.iPort = iRpcConfig.iListenPort;
    if (iSocket->TcpServer(&iTcpSerAddrInfo) == RET_ERROR) {
        LOG(LL_ERROR, "%s::create CSocket.socket failed.", funcName);
        return RET_ERROR;
    }
    DEBUG(LL_INFO, "%s:start tcp server success.", funcName);
    iLastUpdateScriptSecs = time(NULL);
    iIsSetProcTitleSucc = false;
    DEBUG(LL_ALL, "%s:End.", funcName);
    return RET_OK;
}

void CRpcIpServer::IngoreSignal() {
    signal(SIGABRT, SIG_IGN);
    signal(SIGQUIT, SIG_IGN);
    signal(SIGFPE,  SIG_IGN);
    signal(SIGSEGV, SIG_IGN);
    signal(SIGTERM, SIG_IGN);
    signal(SIGUSR1, SIG_IGN);
    signal(SIGCLD,  SIG_IGN);
    signal(SIGALRM, SIG_IGN);
    signal(SIGPIPE, SIG_IGN);
    signal(SIGSTOP, SIG_IGN);
    signal(SIGTSTP, SIG_IGN);
    //signal(SIGINT, SIG_IGN);
}

// ---------------------------------------------------------------------------
// int CRpcIpServer::ConfigInitial()
//
// configuration service is initialized.
// ---------------------------------------------------------------------------
//
int CRpcIpServer::ConfigInitial() {
    const char *funcName = "CRpcIpServer::ConfigInitial()";
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
            return RET_ERROR;
        }
        CSingleton<CLogWriter>::Instance()->SetLogFileName(tmpStr);
    }
    int tmpInt = 0;
    if (!(iRpcConfig.iConfigFlag & 1)) {
        tmpInt = iConfig->GetCfgInt("GLOBAL.LOG_LEVEL", CFG_LOG_LEVEL);
        iConfig->SetConfig(CFG_LOG_LEVEL, tmpInt);
        CSingleton<CLogWriter>::Instance()->SetLogLevel(tmpInt);
        LOG(LL_VARS, "%s:GLOBAL.LOG_LEVEL=[%d]", funcName, tmpInt);
    }
    char *home = NULL;
    if ((home = getenv("UDP_AGENT_PORT")) == NULL) {
        LOG(LL_ERROR, "%s:AGENT_PORT null GLOBAL.UDP_AGENT_PORT.", funcName);
        tmpInt = iConfig->GetCfgInt("GLOBAL.UDP_AGENT_PORT", CFG_UDP_BIND_PORT);
    } else {
        tmpInt = atoi(home);
    }
    iConfig->SetConfig(CFG_UDP_BIND_PORT, tmpInt);
    LOG(LL_VARS, "%s:GLOBAL.UDP_AGENT_PORT=[%d]", funcName, tmpInt);

    if ((home = getenv("UDP_AGENT_IP")) == NULL) {
        LOG(LL_ERROR, "%s:AGENT_IP null GLOBAL.UDP_AGENT_IP.", funcName);
        tmpStr = iConfig->GetCfgStr("GLOBAL.UDP_AGENT_IP", CFG_UDP_BIND_IP);
    } else {
        tmpStr = home;
    }
    if (tmpStr.size() < 7) {
        LOG(LL_ERROR, "%s:AGENT_IP get failed.", funcName);
        return RET_ERROR;
    }
    iConfig->SetConfig(CFG_UDP_BIND_IP, tmpStr);
    LOG(LL_VARS, "%s:GLOBAL.UDP_BIND_IP=[%s]", funcName, tmpStr.c_str());

    tmpStr = iConfig->GetCfgStr("GLOBAL.LOCAL_HOST_IP", "127.0.0.1");
    LOG(LL_VARS, "%s:GLOBAL.LOCAL_HOST_IP=[%s]", funcName, tmpStr.c_str());
    iRpcConfig.iLocalHostIP = tmpStr;

    tmpInt = iConfig->GetCfgInt("GLOBAL.SOCKET_TIME_OUT", CFG_SOCK_TIME_OUT);
    iConfig->SetConfig(CFG_SOCK_TIME_OUT, tmpInt);
    LOG(LL_VARS, "%s:GLOBAL.SOCKET_TIME_OUT=[%d]", funcName, tmpInt);

    tmpInt = iConfig->GetCfgInt("GLOBAL.SOCKET_RW_TIME_OUT", CFG_SOCK_RW_TIME_OUT);
    iConfig->SetConfig(CFG_SOCK_RW_TIME_OUT, tmpInt);
    LOG(LL_VARS, "%s:GLOBAL.SOCKET_RW_TIME_OUT=[%d]", funcName, tmpInt);

    tmpInt = iConfig->GetCfgInt("GLOBAL.CMD_GIVE_UP_RUNTIME",
        CFG_CMD_GIVE_UP_RUNTIME);
    iConfig->SetConfig(CFG_CMD_GIVE_UP_RUNTIME, tmpInt);
    LOG(LL_VARS, "%s:GLOBAL.CMD_GIVE_UP_RUNTIME=[%d]", funcName, tmpInt);

    tmpInt = iConfig->GetCfgInt("GLOBAL.CMD_WARN_RUNTIME", CFG_CMD_WARN_RUNTIME);
    iConfig->SetConfig(CFG_CMD_WARN_RUNTIME, tmpInt);
    LOG(LL_VARS, "%s:GLOBAL.CMD_WARN_RUNTIME=[%d]", funcName, tmpInt);


    tmpInt = iConfig->GetCfgInt("GLOBAL.COUNT_HANDLER", CFG_COUNT_HANDLER);
    if (tmpInt > MAX_SIZE_1K) {
        tmpInt = MAX_SIZE_1K;
        LOG(LL_WARN, "%s:GLOBAL.COUNT_HANDLER too large, max count:[%d]",
            funcName, tmpInt);
    }
    iConfig->SetConfig(CFG_COUNT_HANDLER, tmpInt);
    LOG(LL_VARS, "%s:GLOBAL.COUNT_HANDLER=[%d]", funcName, tmpInt);
    iRpcConfig.iHandlerCounts = tmpInt;
    iNeedForkCounts = tmpInt;

    tmpInt = iConfig->GetCfgInt("GLOBAL.MAX_REQUESTS_PER_CHILD", 0);
    if (tmpInt < 0) {
        tmpInt = CFG_MAX_REQU_PER_CHILD;
        LOG(LL_WARN, "%s:bad GLOBAL.MAX_REQUESTS_PER_CHILD, default:[%d]",
            funcName, tmpInt);
    }
    iConfig->SetConfig(CFG_MAX_REQU_PER_CHILD, tmpInt);
    LOG(LL_VARS, "%s:GLOBAL.MAX_REQUESTS_PER_CHILD=[%d]", funcName, tmpInt);

    DEBUG(LL_ALL, "%s:End", funcName);
    return RET_OK;
}

// ---------------------------------------------------------------------------
// void CRpcIpServer::ForkWorker()
//
// fork new worker.
// ---------------------------------------------------------------------------
//
void CRpcIpServer::ForkWorker() {
    const char *funcName = "CRpcIpServer::ForkWorker()";
    DEBUG(LL_ALL, "%s:Begin need fork [%d] sub-process:",
        funcName, iNeedForkCounts);
    if (iNeedForkCounts <= 0) {
        return;
    }

    int pid = fork();
    if (pid == 0) {
        WorkerStart();
        _exit(0);
    } else if (pid < 0) {
        LOG(LL_ERROR, "%s:fork failed.", funcName);
        return;
    }
    iProcMaintain->AddChilds(pid, time(NULL));
    iNeedForkCounts--;
    DEBUG(LL_ALL, "%s:End fork a subprocess %d ok", funcName, pid);
}

void CRpcIpServer::AddRpcObserver(CRpcSerObserver *aRpcObserver) {
    iRpcSerObserver = aRpcObserver;
}

void CRpcIpServer::IncrOutPks() {
    iRpcMsgHandler->IncrOutPks();
}

void CRpcIpServer::AddVectorDesc(const string& aVecterDesc) {
    iRpcMsgHandler->AddVectorDesc(aVecterDesc);
}
// ---------------------------------------------------------------------------
// void CRpcIpServer::UpdateConns();
//
// update conns.
// ---------------------------------------------------------------------------
//
int CRpcIpServer::UpdateConns() {
    if (!iIsSetProcTitleSucc) {
        return RET_OK;
    }
    const char *funcName = "CRpcIpServer::UpdateConns()";
    LOG(LL_WARN, "%s:Begin, iNeedForkCounts is:[%d]",
        funcName, iNeedForkCounts);
    char psbuf[512], fileName[100];
    int curCounts = iRpcConfig.iHandlerCounts;

    snprintf(fileName, sizeof(fileName),
        ".aiif.%d.sys", iRpcConfig.iListenPort);
    snprintf(psbuf, sizeof(psbuf),
        "ps -ef | grep \"%s\" | grep -v grep | wc -l", getproctitle());
    string psret = CSingleton<CProcess>::Instance()->RunShell(psbuf, fileName);
    if (psret.empty()) {
        LOG(LL_ERROR, "%s:run shell error:%s", funcName, psbuf);
        return RET_ERROR;
    }
    int tmp_cnt = atoi(psret.c_str());
    LOG(LL_INFO, "%s:check res:[%s]-[%d]", funcName, psret.c_str(), tmp_cnt);
    if (tmp_cnt > 0) {
        curCounts = tmp_cnt - 1;
    }
    iNeedForkCounts = iRpcConfig.iHandlerCounts - curCounts;
    return RET_OK;
}

// ---------------------------------------------------------------------------
// int CRpcIpServer::MainStart()
//
// start the threads for data operation.
// ---------------------------------------------------------------------------
//
int CRpcIpServer::MainStart() {
    DEBUG(LL_ALL, "CRpcIpServer::OnStart():Begin");
    iProcMaintain->IpcReset();
    const char* newTitle = getproctitle();
    if (newTitle != NULL) {
        iIsSetProcTitleSucc = strstr(newTitle, "Master") != NULL ? true : false;
    }
    while (1) {
        while (iNeedForkCounts > 0) {
            ForkWorker();
            usleep(5000);
        }
        sleep(5);
        iProcMaintain->IpcChilds();
        iNeedForkCounts = iProcMaintain->KickTimeoutChilds(6);
        UpdateConns();
    }
}

// ---------------------------------------------------------------------------
// int CRpcIpServer::WorkerAcceptStart()
//
// start the worker thread for socket accept.
// ---------------------------------------------------------------------------
//
int CRpcIpServer::WorkerAcceptStart() {
    const char *funcName = "CRpcIpServer::WorkerAcceptStart()";
    DEBUG(LL_ALL, "%s:Begin", funcName);
    struct SAddrInfo clntAddInfo;
    int ret = iSocket->AcceptConnection(iTcpSerAddrInfo.iSockId, &clntAddInfo);
    if (ret == RET_ERROR) {
        LOG(LL_ERROR, "%s:accept error:(%s).", funcName, strerror(errno));
        return RET_ERROR;
    }
    ret = iRpcMsgHandler->AddSockidIntoPoll(clntAddInfo.iSockId);
    if (ret == RET_ERROR) {
        LOG(LL_ERROR, "%s:add sockid:(%d) into poll error:(%s).",
            funcName, clntAddInfo.iSockId, strerror(errno));
        _exit(0);
    }
    DEBUG(LL_ALL, "%s:End", funcName);
    return RET_OK;
}

// start the worker process/thread for working.
int CRpcIpServer::WorkerStart() {
    const char *funcName = "CRpcIpServer::WorkerAcceptStart()";
    DEBUG(LL_ALL, "%s:Begin", funcName);
    setproctitle("%s:Worker.%s.", getproctitle(),
        iRpcConfig.iServerName.c_str());
    setsid();
    IngoreSignal();
    DEBUG(LL_INFO, "%s:try to reload config file...", funcName);
    ConfigInitial();
    DEBUG(LL_INFO, "%s:try to creating MsgHandler...", funcName);
    iRpcMsgHandler = new CRpcMsgHandler();
    if (iRpcMsgHandler == NULL) {
        LOG(LL_ERROR, "%s:new a worker handler failed.", funcName);
        return RET_ERROR;
    }
    DEBUG(LL_INFO, "%s:try initializing MsgHandler...", funcName);
    if (RET_ERROR == iRpcMsgHandler->InitData(iRpcConfig)) {
        LOG(LL_ERROR, "%s:Msg Handler initializing error.", funcName);
        kill(iMainPid, 9);
        return RET_ERROR;
    }
    iRpcMsgHandler->AddRpcObserver(iRpcSerObserver);
    iRpcMsgHandler->AddSockidIntoPoll(0);
    DEBUG(LL_INFO, "%s:try to start tcp server accepting...", funcName);
    try {
        start();
    } catch (string catchStr) {
        LOG(LL_ERROR, "%s:start worker accept error:(%s).",
            funcName, catchStr.c_str());
        delete iRpcMsgHandler;
        iRpcMsgHandler = NULL;
        return RET_ERROR;
    }
    DEBUG(LL_INFO, "%s:try to start MsgHandler...", funcName);
    iRpcMsgHandler->Start();
    LOG(LL_ERROR, "%s:try stop MsgHandler...", funcName);
    iRpcMsgHandler->Stop();
    if (iRpcMsgHandler != NULL) {
        delete iRpcMsgHandler;
        iRpcMsgHandler = NULL;
    }
    LOG(LL_ERROR, "%s:MsgHandler:(%d) is exit.", funcName, getpid());
    DEBUG(LL_ALL, "%s:End", funcName);
    return RET_OK;
}

// ---------------------------------------------------------------------------
// int CRpcIpServer::SignalToChildForUpdateScript()
//
// send signal to child pid for script updating.
// ---------------------------------------------------------------------------
//
int CRpcIpServer::SignalToChildForUpdateScript() {
    int tmpNowSecs = time(NULL);
    if (iLastUpdateScriptSecs + 5 < tmpNowSecs) {
        iProcMaintain->IpcBroadcastChilds("reload script.");
        iLastUpdateScriptSecs = tmpNowSecs;
    }
    return RET_OK;
}
// end of local file.
