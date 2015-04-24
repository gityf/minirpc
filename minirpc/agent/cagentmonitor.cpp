/*
** Copyright (C) 2014 Wang Yaofu
** All rights reserved.
**
**Author:Wang Yaofu voipman@qq.com
**Description: The source file of class CAgentMonitor.
*/
#include <csignal>
#include "agent/cagentmonitor.h"
#include "agent/cagentservice.h"
#include "common/csocket.h"
#include "common/csmtpmail.h"
#include "common/csingleton.h"
#include "common/cconfig.h"
#include "common/clogwriter.h"
// ---------------------------------------------------------------------------
// CAgentMonitor::CAgentMonitor()
//
// constructor.
// ---------------------------------------------------------------------------
//
CAgentMonitor::CAgentMonitor()
    : iStopRequested(false), iRunCondition(false) {
    DEBUG(LL_ALL, "CAgentMonitor::Begin.");
    DEBUG(LL_ALL, "CAgentMonitor::End.");
}

// ---------------------------------------------------------------------------
// CAgentMonitor::~CAgentMonitor()
//
// Destructor.
// ---------------------------------------------------------------------------
//
CAgentMonitor::~CAgentMonitor() {
    DEBUG(LL_ALL, "~CAgentMonitor::Begin.");
    if (iSocket != NULL) {
        delete iSocket;
        iSocket = NULL;
    }
    DEBUG(LL_ALL, "~CAgentMonitor::End.");
}

// ---------------------------------------------------------------------------
// int CAgentMonitor::InitData(struct CConfig aConfigInfo)
//
// init operation where local class is created.
// ---------------------------------------------------------------------------
//
int CAgentMonitor::InitData() {
    const char *funcName = "CAgentMonitor::InitData";
    DEBUG(LL_ALL, "%s::Begin.", funcName);
    iConfig = CSingleton<CConfig>::Instance();
    string tmpStr = iConfig->GetCfgStr("MONITOR.FROM", DEFAULT_EMAIL_FROM);
    LOG(LL_VARS, "%s:MONITOR.FROM=[%s]", funcName, tmpStr.c_str());
    iEmailInfos.iLoginName = tmpStr;

    tmpStr = iConfig->GetCfgStr("MONITOR.TO", DEFAULT_EMAIL_TO);
    LOG(LL_VARS, "%s:MONITOR.TO=[%s]", funcName, tmpStr.c_str());
    iEmailInfos.iEmailTo = tmpStr;

    tmpStr = iConfig->GetCfgStr("MONITOR.SERVER", DEFAULT_EMAIL_SERVER);
    LOG(LL_VARS, "%s:MONITOR.SERVER=[%s]", funcName, tmpStr.c_str());
    iEmailInfos.iEmailServer = tmpStr;

    tmpStr = iConfig->GetCfgStr("MONITOR.PASSWORD", DEFAULT_EMAIL_PASSWORD);
    LOG(LL_VARS, "%s:MONITOR.PASSWORD=[******]", funcName);
    iEmailInfos.iUserPass = tmpStr;

    iMonSqsName = iConfig->GetCfgStr("MONITOR.SQS", "AISQS");
    LOG(LL_VARS, "%s:MONITOR.SQS=[%s]", funcName, iMonSqsName.c_str());
    iSocketFd = 0;
    DEBUG(LL_DBG, "%s:try create CSocket.", funcName);
    iSocket = new CSocket();
    if (iSocket == NULL) {
        LOG(LL_ERROR, "%s::create CSocket failed.", funcName);
        return RET_ERROR;
    }
    DEBUG(LL_ALL, "%s:End", funcName);
    return RET_OK;
}

// ---------------------------------------------------------------------------
// int CAgentMonitor::AddPkg(struct SAddrInfo aAddrInfo)
//
// Add request into queue.
// ---------------------------------------------------------------------------
//
int CAgentMonitor::AddNotifyMsg(string aSubject, string aMsg) {
    const char *funcName = "CAgentMonitor::AddNotifyMsg";
    DEBUG(LL_ALL, "%s:Begin.", funcName);
    LOG(LL_VARS, "[%lu] %s, msg:(%s)",
        _pid, funcName, aMsg.c_str());
    if (iMonMsgQueue.size() >= MAX_MON_QUEUE_SIZE) {
        LOG(LL_ERROR, "[%lu] %s:too many request.", _pid, funcName);
        return RET_ERROR;
    }
    struct SEmailObj tmpEmailObj;
    tmpEmailObj.iSubject = aSubject;
    tmpEmailObj.iEmailContent = aMsg;
    iEmailMutex.lock();
    iMonMsgQueue.push(tmpEmailObj);
    iEmailMutex.unlock();
    iRunCondition.set(true);
    DEBUG(LL_ALL, "%s:End", funcName);
    return RET_OK;
}

// ---------------------------------------------------------------------------
// void CAgentMonitor::run()
//
// this is a runner of local thread.
// ---------------------------------------------------------------------------
//
void CAgentMonitor::run() {
    while (!iStopRequested) {
        iRunCondition.wait_for();
        if (MONITOR_TYPE_SQS == iConfig->GetCfgInt(
            "MONITOR.TYPE", MONITOR_TYPE_SQS)) {
            PushIntoSQS();
        } else {
            EmailNotify();
        }        
        iRunCondition.set(false);
    }
}

// ---------------------------------------------------------------------------
// void CAgentMonitor::on_stop()
//
// stop the local thread.
// ---------------------------------------------------------------------------
//
void CAgentMonitor::on_stop() {
    LOG(LL_ALL, "CAgentMonitor::on_stop()::Begin");
    iStopRequested = true;
    LOG(LL_ALL, "CAgentMonitor::on_stop()::End");
}

// ---------------------------------------------------------------------------
// void CAgentMonitor::EmailNotify()
//
// get request from queue and handle it.
// ---------------------------------------------------------------------------
//
void CAgentMonitor::EmailNotify() {
    const char *funcName = "CAgentMonitor::EmailNotify()";
    DEBUG(LL_ALL, "%s:Begin", funcName);

    struct SEmailObj tmpEmailObj;
    while (!iMonMsgQueue.empty()) {        
        iEmailMutex.lock();
        tmpEmailObj = iMonMsgQueue.front();
        iMonMsgQueue.pop();
        iEmailMutex.unlock();
        iEmailInfos.iEmailContent = tmpEmailObj.iEmailContent;
        iEmailInfos.iSubJect = tmpEmailObj.iSubject;
        CSingleton<CSmtpMail>::Instance()->SendMail(iEmailInfos);
    }
    DEBUG(LL_ALL, "%s:End", funcName);
}

// ---------------------------------------------------------------------------
// void CAgentMonitor::PushIntoSQS()
//
// get request from queue and put into sqs queue.
// ---------------------------------------------------------------------------
//
void CAgentMonitor::PushIntoSQS() {
    const char *funcName = "CAgentMonitor::PushIntoSQS()";
    DEBUG(LL_ALL, "%s:Begin", funcName);

    struct SEmailObj tmpEmailObj;
    while (!iMonMsgQueue.empty()) {
        iEmailMutex.lock();
        tmpEmailObj = iMonMsgQueue.front();
        iMonMsgQueue.pop();
        iEmailMutex.unlock();
        if (iSocketFd == 0)
        {
            string ipport =
                CSingleton<CAgentService>::Instance()->GetIpPortByService(iMonSqsName);
            char itemBuf[SERVICE_ITEM_LEN] = {0};
            // {SERVICE} {IP} {PORT}
            SAddrInfo addrInfo;
            int ret = sscanf(ipport.c_str(),
                "{%200[^}]%*[^{]{%200[^}]%*[^{]{%d",
                itemBuf, itemBuf+100, &addrInfo.iPort);
            if (ret != 3) {
                LOG(LL_ERROR, "%s:SQS decode failed,ret:(%d).", funcName, ret);
                break;
            }
            addrInfo.iAddr = itemBuf+100;
            DEBUG(LL_DBG, "%s:ip:port=(%s:%d)",
                funcName, addrInfo.iAddr.c_str(), addrInfo.iPort);
            addrInfo.iProtocol = TCP;
            addrInfo.iFamily = AF_INET;
            DEBUG(LL_DBG, "%s:try to create socket.", funcName);
            if (RET_ERROR == iSocket->MakeTcpConn(&addrInfo)) {
                LOG(LL_ERROR, "%s:MakeTcpConn.ip:(%s),port:(%d),error:(%s).",
                    funcName, addrInfo.iAddr.c_str(),
                    addrInfo.iPort, strerror(errno));
                break;
            }
            iSocketFd = addrInfo.iSockId;
        }
        int requlen = iSocket->CreatePkgHeader(tmpEmailObj.iEmailContent.length(),
            iRecvBuffer, "imq|");
        INIT_IOV(2);
        SET_IOV_LEN(iRecvBuffer, requlen);
        SET_IOV_LEN(tmpEmailObj.iEmailContent.c_str(), tmpEmailObj.iEmailContent.length());
        if ((RET_ERROR == iSocket->TcpSendV(iSocketFd, iovs, 2, 2, true)) \
            || (RET_ERROR == iSocket->TcpRecv(iSocketFd, iRecvBuffer,
            PKG_HEADER_LEN, 5))) {
            close(iSocketFd);
            iSocketFd = 0;
            break;
        }
    }
    DEBUG(LL_ALL, "%s:End", funcName);
}
// end of local file.
