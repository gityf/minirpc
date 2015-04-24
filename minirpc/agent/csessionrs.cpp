/*
** Copyright (C) 2014 Wang Yaofu
** All rights reserved.
**
**Author:Wang Yaofu voipman@qq.com
**Description: The source file of class CSessionRS.
*/
#include <unistd.h>
#include <cstdio>
#include <cstdlib>
#include <csignal>
#include "agent/csessionrs.h"
#include "agent/version.h"
#include "common/cconfig.h"
#include "common/clogwriter.h"
#include "common/csocket.h"
#include "common/csingleton.h"

// ---------------------------------------------------------------------------
// CSessionRS::CSessionRS()
//
// constructor.
// ---------------------------------------------------------------------------
//
CSessionRS::CSessionRS() : iStopRequested(false) {
    iSocket   = NULL;
    iSocketRWTimeOut = 5;
}

// ---------------------------------------------------------------------------
// CSessionRS::~CSessionRS()
//
// Destructor.
// ---------------------------------------------------------------------------
//
CSessionRS::~CSessionRS() {
    if (iSocket != NULL) {
        delete iSocket;
        iSocket = NULL;
    }
}

// ---------------------------------------------------------------------------
// int CSessionRS::InitData(struct CConfig aConfigInfo)
//
// init operation where local class is created.
// ---------------------------------------------------------------------------
//
int CSessionRS::InitData() {
    const char *funcName = "CSessionRS::InitData";
    DEBUG(LL_ALL, "%s::Begin.", funcName);
    iConfig = CSingleton<CConfig>::Instance();
    iSocketRWTimeOut   = iConfig->GetConfig(CFG_SOCK_RW_TIME_OUT);
    DEBUG(LL_DBG, "%s:try create CSocket.", funcName);
    iSocket = new CSocket();
    if (iSocket == NULL) {
        LOG(LL_ERROR, "%s::create CSocket failed.", funcName);
        return RET_ERROR;
    }
    iAgentService = CSingleton<CAgentService>::Instance();
    if (iAgentService == NULL) {
        LOG(LL_ERROR, "%s::create CAgentService failed.", funcName);
        return RET_ERROR;
    }
    DEBUG(LL_DBG, "%s:try initializing UDP CSocket.", funcName);
    iUdpSerAddrInfo.iProtocol = UDP;
    iUdpSerAddrInfo.iPort = iConfig->GetConfig(CFG_UDP_BIND_PORT);
    DEBUG(LL_DBG, "%s:try call socket.", funcName);
    if (iSocket->TcpServer(&iUdpSerAddrInfo) == RET_ERROR) {
        LOG(LL_ERROR, "%s:create CSocket.socket failed.", funcName);
        return RET_ERROR;
    }
    if (iConfig->GetConfig(CFG_ADD_MULTICAST_FLAG) > 0) {
        DEBUG(LL_DBG, "%s:try add socket into multicast membership.", funcName);
        int ret = iSocket->SetMultiCast(iUdpSerAddrInfo.iSockId, kJoinMultiCast,
            iConfig->GetConfig(CFG_UDP_BIND_IP));
        if (ret == RET_ERROR) {
            LOG(LL_ERROR, "%s:SetMultiCast failed.", funcName);
            return RET_ERROR;
        }
    }
    DEBUG(LL_DBG, "%s:try initializing statistic Infos...", funcName);
    char tmpStatisticBuf[PKG_MAX_LEN] = {0};
    // service type.
    iStatisticInfos.iServiceType  = "Stat.Center";
    // login name.
    char *p = getlogin();
    iStatisticInfos.iLoginName = (p == NULL) ? "NULL" : p;
    DEBUG(LL_DBG, "%s:get loginName:(%s)", funcName, iStatisticInfos.iLoginName.c_str());
    // local ip address.
    string tcpbindip = iConfig->GetConfig(CFG_TCP_BIND_IP);
    if (iSocket->GetLocalIP(tmpStatisticBuf) == RET_OK) {
        iStatisticInfos.iIpAddress = tmpStatisticBuf;
    } else {
        LOG(LL_ERROR, "%s:get local ip address failed.", funcName);
        iStatisticInfos.iIpAddress = tcpbindip;
    }
    if (tcpbindip != CFG_TCP_BIND_IP) {
        iStatisticInfos.iIpAddress = tcpbindip;
    }
    iConfig->SetConfig(CFG_TCP_BIND_IP, iStatisticInfos.iIpAddress);
    // local process listen port.
    iStatisticInfos.iPort = iUdpSerAddrInfo.iPort;
    // local process id.
    iStatisticInfos.iPid = getpid();
    // current connections.
    iStatisticInfos.iCurConns = 0;
    // handled connections.
    iStatisticInfos.iHandledConns = 0;
    // input packages.
    iStatisticInfos.iInputPkgs = 0;
    // input packages tps.
    iStatisticInfos.iInputPkgTps = 0;
    // output packages.
    iStatisticInfos.iOutputPkgs = 0;
    // start time.
    iStatisticInfos.iStartTime = time(NULL);
    iStatisticInfos.iSecs = MAX_AGENT_SECS;
    iStatisticInfos.iStartTimeStr = ctime(&iStatisticInfos.iStartTime);
    // running time.
    iStatisticInfos.iRunningTime = 0;
    // service name
    iStatisticInfos.iService = "AAAAgentStatisticInfo";
    iLastInputPkgs = 0;
    iLastSecs = iStatisticInfos.iStartTime;
    iAgentService->AddService(iStatisticInfos);
    DEBUG(LL_ALL, "%s:End", funcName);
    return 0;
}

// ---------------------------------------------------------------------------
// void CSessionRS::run()
//
// this is a runner of local thread.
// ---------------------------------------------------------------------------
//
void CSessionRS::run() {
    while (!iStopRequested) {
        SessionHandler();
    }
}

// ---------------------------------------------------------------------------
// void CSessionRS::on_stop()
//
// stop the local thread.
// ---------------------------------------------------------------------------
//
void CSessionRS::on_stop() {
    LOG(LL_ALL, "CSessionRS::on_stop()::Begin");
    iStopRequested = true;
    LOG(LL_ALL, "CSessionRS::on_stop()::End");
}

// ---------------------------------------------------------------------------
// void CSessionRS::SessionHandler()
//
// session handler method.
// ---------------------------------------------------------------------------
//
void CSessionRS::SessionHandler() {
    const char *funcName = "CSessionRS::SessionHandler()";
    DEBUG(LL_ALL, "%s:Begin", funcName);
    struct SAddrInfo peerInfo;
    // zero buffer is using to much cpu in perf top.
    //memset(iPkgBuffer, 0, sizeof(iPkgBuffer));
    DEBUG(LL_DBG, "%s:try call UdpRecv...", funcName);
    int ret = iSocket->UdpRecv(iUdpSerAddrInfo.iSockId,
        iPkgBuffer, PKG_MAX_LEN, iSocketRWTimeOut, &peerInfo.iSockUnion);
    if (ret == RET_ERROR || !HeadCheck()) {
        LOG(LL_ERROR, "%s:UdpRecv failed,error:(%s)",
            funcName, strerror(errno));
        return;
    }
    iStatisticInfos.iInputPkgs++;
    long nowSecs = CThreadTimer::instance()->getNowSecs();
    if (iLastInputPkgs + 5000 <= iStatisticInfos.iInputPkgs
        || iLastSecs + 5 <= nowSecs) {
        iStatisticInfos.iRunningTime = nowSecs - iStatisticInfos.iStartTime;
        nowSecs = time(NULL);
        long interval = nowSecs - iLastSecs;
        iLastSecs = nowSecs;
        iStatisticInfos.iInputPkgTps
            = (iStatisticInfos.iInputPkgs - iLastInputPkgs) / ((interval > 0) ? interval : 1);
        iStatisticInfos.iSecs = MAX_AGENT_SECS;
        iLastInputPkgs = iStatisticInfos.iInputPkgs;
        iAgentService->AddService(iStatisticInfos);
    }
    iPkgBuffer[ret] = 0x00;
    peerInfo.iSockId = iUdpSerAddrInfo.iSockId;
    DEBUG(LL_DBG, "%s:udp recv sockid:(%d),buffer:(%s).",
        funcName, peerInfo.iSockId, iPkgBuffer);
    SessionReqHandle(&peerInfo);
    DEBUG(LL_ALL, "%s:End", funcName);
}

// ---------------------------------------------------------------------------
// void CSessionRS::SessionReqHandle()
//
// get request from queue and handle it.
// ---------------------------------------------------------------------------
//
void CSessionRS::SessionReqHandle(struct SAddrInfo *aPeerInfo) {
    const char *funcName = "CSessionRS::SessionReqHandle()";
    DEBUG(LL_ALL, "%s:Begin", funcName);
    if (aPeerInfo == NULL) {
        LOG(LL_ERROR, "%s:aPeerInfo is NULL.", funcName);
        return;
    }
    char chksumstr[PKG_CHKSUM_LEN+1] = {0};
    char itemBuf[PKG_MAX_LEN] = {0};
    int ret = RET_OK;
    int cmdLen = strlen(iPkgBuffer);
    //memset(chksumstr, 0, sizeof(chksumstr));
    iStatisticInfos.ChkSum(cmdLen-PKG_HEADFLAG_LEN-PKG_CMDLEN_LEN-PKG_CHKSUM_LEN,
        iPkgBuffer+PKG_HEADFLAG_LEN+PKG_CMDLEN_LEN, chksumstr);
    DEBUG(LL_VARS, "%s:req:(%s).", funcName, iPkgBuffer);
    if (memcmp(PKG_UDP_HEAD_FLAG, iPkgBuffer, PKG_HEADFLAG_LEN) != RET_OK) {
        LOG(LL_ERROR, "%s:bad pkg header:(%s).",
            funcName, iPkgBuffer);
        return;
    }
    if (memcmp(chksumstr, iPkgBuffer+cmdLen-PKG_CHKSUM_LEN,
        PKG_CHKSUM_LEN) != RET_OK) {
        LOG(LL_ERROR, "%s:bad pkg chksum:(%s), ok chksum:(%s)",
            funcName, iPkgBuffer+cmdLen-PKG_CHKSUM_LEN,
            chksumstr);
        return;
    }
    //memset(itemBuf, 0, sizeof(itemBuf));
    if (memcmp(iPkgBuffer+PKG_HEADFLAG_LEN+PKG_CMDLEN_LEN,
        PKG_CMD_REGISTER, PKG_CMDLEN_LEN) == RET_OK
      || memcmp(iPkgBuffer+PKG_HEADFLAG_LEN+PKG_CMDLEN_LEN,
        PKG_CMD_HEARTBEAT, PKG_CMDLEN_LEN) == RET_OK) {
        CStatisticInfos service;
        /*
        * msg LOGIN {HN}   {UN}      {SN}  {IP}       {port} {PID}  {CC}{HC} {IS) {OS} {TPS} {ST}         {RT}
        * msg:LOGIN {v890} {wangyf5} {BMS} {10.1.1.1} {8081} {1234} {0} {20} {10} {10} {10} {1380202020} {123}
        */
        char vectorBuf[257] = {0};
        ret = sscanf(iPkgBuffer+PKG_HEADFLAG_LEN+PKG_CMDLEN_LEN,
            "%*[^{]{%200[^}]%*[^{]{%200[^}]"
            "%*[^{]{%200[^}]%*[^{]{%200[^}]"
            "%*[^{]{%d%*[^{]{%d"
            "%*[^{]{%lu%*[^{]{%lld"
            "%*[^{]{%lld%*[^{]{%lld%*[^{]{%lld"
            "%*[^{]{%lu%*[^{]{%lu%*[^{]{%256[^}]",
            itemBuf, itemBuf+SERVICE_ITEM_LEN,
            itemBuf+SERVICE_ITEM_LEN*2, itemBuf+SERVICE_ITEM_LEN*3,
            &service.iPort, &service.iPid,
            &service.iCurConns, &service.iHandledConns,
            &service.iInputPkgs, &service.iOutputPkgs, &service.iInputPkgTps,
            &service.iStartTime, &service.iRunningTime, vectorBuf);
        if (ret < 13) {
            LOG(LL_ERROR, "%s:session request:(%s),ret:(%d),decode failed.",
                funcName, iPkgBuffer, ret);
            return;
        }
        service.iServiceType = itemBuf;
        service.iLoginName = itemBuf + SERVICE_ITEM_LEN;
        service.iService  = itemBuf + SERVICE_ITEM_LEN*2;
        service.iIpAddress = itemBuf + SERVICE_ITEM_LEN*3;
        service.iVectorInfo = vectorBuf;
        LOG(LL_VARS, "%s:request:(%s)", funcName, service.iService.c_str());
        LOG(LL_VARS, "%s:decode->type:(%s),user:(%s),service:(%s),"
            "ip:(%s),port:(%d),pid:(%d),cur-conns:(%lu),handled-conns:(%lld),"
            "inPkg:(%lld),outPkg:(%lld),inputPkgTps:(%lld),starttime:(%lu),runtime:(%lu)",
            funcName, service.iServiceType.c_str(), service.iLoginName.c_str(),
            service.iService.c_str(), service.iIpAddress.c_str(),
            service.iPort, service.iPid, service.iCurConns, service.iHandledConns,
            service.iInputPkgs, service.iOutputPkgs, service.iInputPkgTps,
            service.iStartTime, service.iRunningTime);
        service.iSecs = CThreadTimer::instance()->getNowSecs();
        service.iStartTimeStr = ctime(&service.iStartTime);
        iAgentService->AddService(service);
    } else if (memcmp(iPkgBuffer+PKG_HEADFLAG_LEN+PKG_CMDLEN_LEN,
        PKG_CMD_REQUEST, PKG_CMDLEN_LEN) == RET_OK) {
        /*
        * msg:GETSERVICE {BMS_TEST}
        */
        ret = sscanf(iPkgBuffer+PKG_HEADFLAG_LEN+PKG_CMDLEN_LEN+PKG_CMDLEN_LEN,
            "%200s%*[^{]{%200[^}]",
            itemBuf, itemBuf+SERVICE_ITEM_LEN);
        if (ret != 2) {
            LOG(LL_ERROR, "%s:service request:(%s),decode failed.",
                funcName, iPkgBuffer);
            return;
        }
        DEBUG(LL_DBG, "%s:commond:(%s),service:(%s).",
            funcName, itemBuf, itemBuf+SERVICE_ITEM_LEN);
        if (strcmp(itemBuf, PKG_GETSERVICE) == RET_OK) {
            string ipport =
                iAgentService->GetIpPortByService(
                string(itemBuf+SERVICE_ITEM_LEN));
            char respBuf[PKG_MAX_LEN] = {0};
            if (((CStatisticInfos *)0)->MakePkg(respBuf, ipport, PKG_CMD_ACK) == RET_ERROR)
            {
                LOG(LL_ERROR, "%s:make pkg failed. ip info:(%s).",
                    funcName, ipport.c_str());
                return;
            }
            LOG(LL_VARS, "%s:dst sockid:(%d),respstr:(%s).",
                funcName, aPeerInfo->iSockId, respBuf);
            iSocket->UdpSend(aPeerInfo->iSockId,
                respBuf, strlen(respBuf), iSocketRWTimeOut, &aPeerInfo->iSockUnion);
            IncrOutPks();
        } else {
            // TODO: for service GETALLSERVICE.
        }
    } else if (memcmp(iPkgBuffer+PKG_HEADFLAG_LEN+PKG_CMDLEN_LEN,
        PKG_CMD_CANCEL, PKG_CMDLEN_LEN) == RET_OK) {
        /*
        *                 LOGOUT {HN}   {UN}      {SN}  {IP}        {port} {PID}
        * msg:'FY'003bCANCLOGOUT {v890} {wangyf5} {BMS} {10.1.1.10} {8081} {1234}F0D3BE9D
        */
        CStatisticInfos service;
        ret = sscanf(iPkgBuffer+PKG_HEADFLAG_LEN+PKG_CMDLEN_LEN,
            "%*[^{]{%200[^}]%*[^{]{%200[^}]"
            "%*[^{]{%200[^}]%*[^{]{%200[^}]"
            "%*[^{]{%d%*[^{]{%d",
            itemBuf, itemBuf+SERVICE_ITEM_LEN,
            itemBuf+SERVICE_ITEM_LEN*2, itemBuf+SERVICE_ITEM_LEN*3,
            &service.iPort, &service.iPid);
        if (ret != 6) {
            LOG(LL_ERROR, "%s:session cancel:(%s),ret:(%d),decode failed.",
                funcName, iPkgBuffer, ret);
            return;
        }
        service.iServiceType = itemBuf;
        service.iLoginName = itemBuf + SERVICE_ITEM_LEN;
        service.iService  = itemBuf + SERVICE_ITEM_LEN*2;
        service.iIpAddress = itemBuf + SERVICE_ITEM_LEN*3;
        LOG(LL_VARS, "%s:cancel:(%s)", funcName, service.iService.c_str());
        LOG(LL_VARS, "%s:decode->type:(%s),user:(%s),service:(%s),"
            "ip:(%s),port:(%d),pid:(%d)",
            funcName, service.iServiceType.c_str(), service.iLoginName.c_str(),
            service.iService.c_str(), service.iIpAddress.c_str(),
            service.iPort, service.iPid);
        service.iSecs = CThreadTimer::instance()->getNowSecs();
        iAgentService->DelService(service);
    } else {
        LOG(LL_WARN, "%s:Bad request:(%s)",
            funcName, iPkgBuffer);
    }
    DEBUG(LL_ALL, "%s:End", funcName);
}
// end of local file.
