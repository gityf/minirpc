/*
** Copyright (C) 2012 AsiaInfo
** All rights reserved.
**
**Author:Wang Yaofu voipman@qq.com
**Description: The source file of class CSessionSync.
*/
#include <unistd.h>
#include "agent/csessionsync.h"
#include "agent/version.h"
#include "agent/cagentmonitor.h"
#include "common/cconfig.h"
#include "common/clogwriter.h"
#include "common/csocket.h"
#include "common/csingleton.h"
#include "common/chttputils.h"
// ---------------------------------------------------------------------------
// CSessionSync::CSessionSync()
//
// constructor.
// ---------------------------------------------------------------------------
//
CSessionSync::CSessionSync() : iStopRequested(false) {
    iSocket   = NULL;
    iSocketRWTimeOut = 5;
}

// ---------------------------------------------------------------------------
// CSessionSync::~CSessionSync()
//
// Destructor.
// ---------------------------------------------------------------------------
//
CSessionSync::~CSessionSync() {
    if (iSocket != NULL) {
        delete iSocket;
        iSocket = NULL;
    }
}

// ---------------------------------------------------------------------------
// int CSessionSync::InitData(struct CConfig aConfigInfo)
//
// init operation where local class is created.
// ---------------------------------------------------------------------------
//
int CSessionSync::InitData() {
    const char *funcName = "CSessionSync::InitData";
    DEBUG(LL_ALL, "%s::Begin.", funcName);
    iConfig = CSingleton<CConfig>::Instance();
    iSocketRWTimeOut   = iConfig->GetConfig(CFG_SOCK_RW_TIME_OUT);
    iHttpSyncBuffer = (char *)malloc((MAX_SIZE_1M+1) * sizeof(char));
    if (iHttpSyncBuffer == NULL) {
        throw string("malloc http body buffer failed.");
        return RET_ERROR;
    }
    memset(iHttpSyncBuffer, 0, MAX_SIZE_1M);
    DEBUG(LL_DBG, "%s:try create CSocket.", funcName);
    iSocket = new CSocket();
    if (iSocket == NULL) {
        LOG(LL_ERROR, "%s::create CSocket failed.", funcName);
        return RET_ERROR;
    }
    iSyncAddrList.clear();
    iSyncAddrStr.clear();
    iDumpSyncInteval = time(NULL);
    DEBUG(LL_DBG, "%s:try initializing TCP CSocket.", funcName);
    iSyncAddrInfo.iProtocol = TCP;
    iSyncAddrInfo.iPort = iConfig->GetConfig(CFG_SYNC_PORT);
    iSyncAddrInfo.iAddr = iConfig->GetConfig(CFG_SYNC_IP);
    DEBUG(LL_ALL, "%s:End", funcName);
    return 0;
}

// ---------------------------------------------------------------------------
// void CSessionSync::run()
//
// this is a runner of local thread.
// ---------------------------------------------------------------------------
//
void CSessionSync::run() {
    while (!iStopRequested) {
        iSyncAddrInfo.iPort = iConfig->GetConfig(CFG_SYNC_PORT);
        iSyncAddrInfo.iAddr = iConfig->GetConfig(CFG_SYNC_IP);
        if (iSyncAddrInfo.iAddr.length() < 7) {
            sleep(HB_TIME_OUT/2);
            continue;
        }
        CSyncAddrInfo curSyncAddrInfo;
        int ii = 0;
        while (ii++ < iSyncAddrList.size()) {
            iSyncMutex.lock();
            curSyncAddrInfo = iSyncAddrList.front();
            iSyncAddrList.pop_front();
            // update sync time to now.
            curSyncAddrInfo.iTryTimes++;
            curSyncAddrInfo.iLastSyncSecs = time(NULL);
            iSyncAddrList.push_back(curSyncAddrInfo);
            iSyncMutex.unlock();
            iSyncAddrInfo.iAddr = curSyncAddrInfo.iIpAddress;
            iSyncAddrInfo.iPort = curSyncAddrInfo.iPort;
            if (SyncHandler(curSyncAddrInfo.iServices) == RET_ERROR) {
                if (curSyncAddrInfo.iTryTimes >= 10) {
                    // sync error, monitor email is sent.
                #ifdef MONITOR_EMAIL
                    // send monitor mail.
                    char mailBuffer[SERVICE_ITEM_LEN+1] = {0};
                    snprintf(mailBuffer, sizeof(mailBuffer),
                        "SyncAgent Infos...\r\n"
                        "\tIP: %s.\r\n"
                        "\tPort: %d.\r\n"
                        "\tInterval: %d.\r\n"
                        "\tStart Time: %s"
                        "\tLast Sync Time: %s"
                        "\tServices: %s",
                        curSyncAddrInfo.iIpAddress.c_str(),
                        curSyncAddrInfo.iPort,
                        curSyncAddrInfo.iInterval,
                        curSyncAddrInfo.iStartSecsStr.c_str(),
                        curSyncAddrInfo.iLastSyncSecsStr.c_str(),
                        curSyncAddrInfo.iServices.c_str());
                    string subject = "Agent Sync Error.";
                    CSingleton<CAgentMonitor>::Instance()->AddNotifyMsg(subject, mailBuffer);
                #endif
                    DelSyncAddr(curSyncAddrInfo);
                }
            } else {
                curSyncAddrInfo.iTryTimes = 0;
                AddSyncAddr(curSyncAddrInfo);
            }
            CloseSyncSockId();
            usleep(5000);
        }
        sleep(iConfig->GetConfig(CFG_SYNC_INTERVAL));
    }
}

// ---------------------------------------------------------------------------
// void CSessionSync::on_stop()
//
// stop the local thread.
// ---------------------------------------------------------------------------
//
void CSessionSync::on_stop() {
    LOG(LL_ALL, "CSessionSync::on_stop()::Begin");
    iStopRequested = true;
    LOG(LL_ALL, "CSessionSync::on_stop()::End");
}

// ---------------------------------------------------------------------------
// int CSessionSync::DumpSyncAddrList()
//
// dump sync address list into string.
// ---------------------------------------------------------------------------
//
int CSessionSync::DumpSyncAddrList() {
    list<CSyncAddrInfo>::iterator it;
    CLock scopeLock(iSyncMutex);
    char syncItem[SERVICE_ITEM_LEN] = {0};
    sprintf(syncItem, "<h2>Statistics Report for pid %d with %d Sync-agents.</h2>\n",
        getpid(), iSyncAddrList.size());
    string htmlStr = "<body><h1>Agent Center</h1>\n";
    htmlStr += syncItem;
    htmlStr += "<hr><h3>&gt; General sync config information</h3>"
        "<table border=0><tr class=a6>"
        "<td>NO.</td><td>Sync ip</td>"
        "<td>port</td><td>interval</td>"
        "<td>add time</td><td>Last sync time</td>"
        "<td>Services</td></tr>\n";
    int ii = 0, jj = 0;
    for (it = iSyncAddrList.begin();
        it != iSyncAddrList.end(); ++it) {
        ii = jj++ % 7;
        snprintf(syncItem, sizeof(syncItem),
            "<tr class=a%d><td>%d</td><td>%s</td><td>%d</td>"
            "<td>%d</td><td>%s</td><td>%s</td><td>%s</td></tr>",
            ii, jj, it->iIpAddress.c_str(), it->iPort, it->iInterval,
            it->iStartSecsStr.c_str(), it->iLastSyncSecsStr.c_str(),
            it->iServices.c_str());
        htmlStr += syncItem;
    }
    htmlStr += "</table><hr><p width=80% align=center>"
               "Copyright (C) 2015 Wang Yaofu</p></body></html>";
    iSyncAddrStr = htmlStr;
    return RET_OK;
}

// ---------------------------------------------------------------------------
// int CSessionSync::SyncHandler(string aServices)
//
// sync handler method.
// ---------------------------------------------------------------------------
//
int CSessionSync::SyncHandler(const string& aServices) {
    const char *funcName = "CSessionSync::SyncHandler()";
    DEBUG(LL_ALL, "%s:Begin", funcName);
    if (iSocket->MakeTcpConn(&iSyncAddrInfo) == RET_ERROR) {
        return RET_ERROR;
    }
    char syncBuffer[PKG_MAX_LEN] = {0};
    sprintf(syncBuffer, "GET /sync/%s HTTP/1.1\r\n\r\n", aServices.c_str());
    int ret = iSocket->TcpSend(iSyncAddrInfo.iSockId, syncBuffer,
        strlen(syncBuffer), iSocketRWTimeOut);

    if (ret == RET_ERROR) {
        LOG(LL_ERROR, "%s:send sync to agent:(%s:%d) error:(%s).",
            funcName, iSyncAddrInfo.iAddr.c_str(),
            iSyncAddrInfo.iPort, strerror(errno));
        return RET_ERROR;
    }
    if (GetSyncResponse() == RET_ERROR) {
        LOG(LL_ERROR, "%s:get sync reponse from agent:(%s:%d) error:(%s).",
            funcName, iSyncAddrInfo.iAddr.c_str(),
            iSyncAddrInfo.iPort, strerror(errno));
        return RET_ERROR;
    }
    SyncDataHandle();
    DEBUG(LL_ALL, "%s:End", funcName);
    return RET_OK;
}

// ---------------------------------------------------------------------------
// int CSessionSync::GetSyncResponse()
//
// get sync response from sync master agent.
// ---------------------------------------------------------------------------
//
int CSessionSync::GetSyncResponse() {
    const char *funcName = "CSessionSync::GetSyncResponse";
    DEBUG(LL_ALL, "%s:Begin", funcName);
    int recvLen = iSocket->RecvHttpPkg(iSyncAddrInfo.iSockId,
        iHttpSyncBuffer, MAX_SIZE_1M, iSocketRWTimeOut);
    if (recvLen == RET_ERROR) {
        LOG(LL_ERROR, "%s:sockid:(%d),recv data error:(%s).",
            funcName, iSyncAddrInfo.iSockId, strerror(errno));
        return RET_ERROR;
    }
    DEBUG(LL_VARS, "%s:recv len:(%d),buffer:", funcName, recvLen);
    DEBUG_RAW(LL_VARS, iHttpSyncBuffer);
    DEBUG(LL_ALL, "%s:End", funcName);
    return RET_OK;
}

// ---------------------------------------------------------------------------
// void CSessionSync::SyncDataHandle()
//
// handle sync data.
// ---------------------------------------------------------------------------
//
void CSessionSync::SyncDataHandle() {
    const char *funcName = "CSessionSync::SyncDataHandle()";
    DEBUG(LL_ALL, "%s:Begin", funcName);
    char itemBuf[PKG_MAX_LEN] = {0};
    int ret = RET_OK;
    char *pLine = iHttpSyncBuffer;
    long remoteSyncTimestamp = 0;
    long now = CThreadTimer::instance()->getNowSecs();
    pLine = strstr(iHttpSyncBuffer, "Date:");
    if (pLine != NULL) {
        DEBUG(LL_VARS, "%s: try to get Date: string", funcName);
        sscanf(pLine, "Date: %ld", &remoteSyncTimestamp);
    } else {
        pLine = iHttpSyncBuffer;
    }
    DEBUG(LL_VARS, "%s:RemoteAgent.(%s:%d).time=(%ld),now:(%ld)",
        funcName, iSyncAddrInfo.iAddr.c_str(), iSyncAddrInfo.iPort,
        remoteSyncTimestamp, now);
    if (remoteSyncTimestamp <= 0) {
        remoteSyncTimestamp = now;
    }
    CStatisticInfos service;
    while ((pLine = GetLine(pLine)) != NULL) {
        DEBUG(LL_DBG, "%s:in while.", funcName);
        /*
        * msg {Type} {UN}      {SN}  {IP}       {port} {PID}  {CC}{HC} {IS) {OS} {TPS} {ST}         {RT} {UpdateTime}
        * msg:{line} {wangyf5} {BMS} {10.1.1.1} {8081} {1234} {0} {20} {10} {10} {10} {1380202020} {123} {123}
        */
        char vectorBuf[257] = {0};
        ret = sscanf(pLine,
            "%*[^{]{%200[^}]%*[^{]{%200[^}]"
            "%*[^{]{%200[^}]%*[^{]{%200[^}]"
            "%*[^{]{%d%*[^{]{%d"
            "%*[^{]{%lu%*[^{]{%lld"
            "%*[^{]{%lld%*[^{]{%lld%*[^{]{%lld"
            "%*[^{]{%lu%*[^{]{%lu%*[^{]{%lu%*[^{]{%256[^}]",
            itemBuf, itemBuf+SERVICE_ITEM_LEN,
            itemBuf+SERVICE_ITEM_LEN*2, itemBuf+SERVICE_ITEM_LEN*3,
            &service.iPort, &service.iPid,
            &service.iCurConns, &service.iHandledConns,
            &service.iInputPkgs, &service.iOutputPkgs, &service.iInputPkgTps,
            &service.iStartTime, &service.iRunningTime, &service.iSecs, vectorBuf);
        DEBUG(LL_DBG, "%s:sscanf ret:(%d)", funcName, ret);
        if (ret < 14) {
            LOG(LL_WARN, "%s:sync request:(%s),ret:(%d),decode failed.",
                funcName, pLine, ret);
            continue;
        }
        service.iServiceType = itemBuf;
        service.iLoginName = itemBuf + SERVICE_ITEM_LEN;
        service.iService  = itemBuf + SERVICE_ITEM_LEN*2;
        service.iIpAddress = itemBuf + SERVICE_ITEM_LEN*3;
        service.iVectorInfo = vectorBuf;
        LOG(LL_VARS, "%s:sync request:(%s)", funcName, service.iService.c_str());
        LOG(LL_VARS, "%s:decode->type:(%s),user:(%s),service:(%s),"
            "ip:(%s),port:(%d),pid:(%d),cur-conns:(%lu),handled-conns:(%lld),"
            "inPkg:(%lld),outPkg:(%lld),inputPkgTps:(%lld),starttime:(%lu),runtime:(%lu)",
            funcName, service.iServiceType.c_str(), service.iLoginName.c_str(),
            service.iService.c_str(), service.iIpAddress.c_str(),
            service.iPort, service.iPid, service.iCurConns, service.iHandledConns,
            service.iInputPkgs, service.iOutputPkgs, service.iInputPkgTps,
            service.iStartTime, service.iRunningTime);
        service.iStartTimeStr = ctime(&service.iStartTime);
        service.iSecs += now - remoteSyncTimestamp;
        CSingleton<CAgentService>::Instance()->AddService(service);
    }
    DEBUG(LL_ALL, "%s:End", funcName);
}
// end of local file.
