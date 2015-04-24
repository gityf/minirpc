/*
** Copyright (C) 2014 Wang Yaofu
** All rights reserved.
**
**Author:Wang Yaofu voipman@qq.com
**Description: The source file of class CClientSession.
*/
#include <string>
#include "client/cclientsession.h"
#include "client/cmsghandler.h"
#include "common/cconfig.h"
#include "common/csingleton.h"
#include "common/cstatisticinfo.h"
#include "common/clogwriter.h"

// ---------------------------------------------------------------------------
// CClientSession::CClientSession()
//
// constructor.
// ---------------------------------------------------------------------------
//
CClientSession::CClientSession() {
    iSocket = NULL;
}

// ---------------------------------------------------------------------------
// CClientSession::~CClientSession()
//
// Destructor.
// ---------------------------------------------------------------------------
//
CClientSession::~CClientSession() {
    if (iSocket != NULL) {
        delete iSocket;
        iSocket = NULL;
    }
}

// ---------------------------------------------------------------------------
// int CClientSession::InitData()
//
// Initial operation.
// ---------------------------------------------------------------------------
//
int CClientSession::InitData() {
    DEBUG_FUNC_NAME("CClientSession::InitData");
    DEBUG(LL_ALL, "%s:Begin", funcName);
    iConfig = CSingleton<CConfig>::Instance();
    iSocketRWTimeOut = iConfig->GetConfig(CFG_SOCK_RW_TIME_OUT);
    DEBUG(LL_INFO, "%s:try to create CSocket client.", funcName);
    iSocket = new CSocket();
    if (iSocket == NULL) {
        LOG(LL_ERROR, "%s:create CSocket failed.", funcName);
        return RET_ERROR;
    }
    iUdpClntAddrInfo.iProtocol = UDP;
    iUdpClntAddrInfo.iPort = iConfig->GetConfig(CFG_UDP_BIND_PORT);
    iUdpClntAddrInfo.iAddr = iConfig->GetConfig(CFG_UDP_BIND_IP);
    iTcpClntAddrInfo.iProtocol = TCP;
    iTcpClntAddrInfo.iPort = iConfig->GetConfig(CFG_TCP_BIND_PORT);
    iTcpClntAddrInfo.iAddr = iConfig->GetConfig(CFG_TCP_BIND_IP);
    iAgentCenterList.push_back(iTcpClntAddrInfo);
    LOG(LL_INFO, "%s:try client CreateSocket...", funcName);
    int ret = iSocket->CreateSocket(&iUdpClntAddrInfo);
    if (ret == RET_ERROR) {
        LOG(LL_ERROR, "%s::create CSocket.socket failed.", funcName);
        return RET_ERROR;
    }
    /*
    if (iSocket->SetSockTimeOut(iUdpClntAddrInfo.iSockId,
        FD_SET_SO_RCVTIMEO, iSocketRWTimeOut*1000) == RET_ERROR)
    {
        LOG(LL_ERROR, "%s::call CSocket.SetSockTimeOut failed.", funcName);
        return RET_ERROR;
    }
    */

    // socket is set to unblock mode avoid to wait for ever on recvfrom.
    if (iSocket->NonBlock(iUdpClntAddrInfo.iSockId, true) == RET_ERROR) {
        LOG(LL_ERROR, "%s::call CSocket.NonBlock failed.", funcName);
        return RET_ERROR;
    }

    DEBUG(LL_INFO, "%s:create CSocket client success.", funcName);
    if (GetAgentCenterList() == RET_ERROR) {
        LOG(LL_ERROR, "%s::get all agent center list failed.", funcName);
    }
    DEBUG(LL_ALL, "%s:End", funcName);
    return RET_OK;
}

// ---------------------------------------------------------------------------
// int CClientSession::CloseServiceSockId(string aService)
//
// close socket id from one service.
// ---------------------------------------------------------------------------
//
int CClientSession::EraseServiceSock(int aSockId) {
    CLock scopeLock(iAddrMutex);
    if (iSockId2Addr.find(aSockId) != iSockId2Addr.end()) {
        if (aSockId > 0) {
            close(aSockId);
            iSockId2Addr.erase(aSockId);
        }
    }
    return RET_OK;
}

int CClientSession::GetSockAddrInfo(SAddrInfo* aAddr) {
    CLock scopeLock(iAddrMutex);
    if (iSockId2Addr.find(aAddr->iSockId) != iSockId2Addr.end()) {
        *aAddr = iSockId2Addr[aAddr->iSockId];
        return RET_OK;
    } else {
        return RET_ERROR;
    }
}
// ---------------------------------------------------------------------------
// int CClientSession::GetIpPortByHttpProto
//
// Add getting service request into queue.
// ---------------------------------------------------------------------------
//
int CClientSession::GetIpPortByHttpProto(const string& aService) {
    DEBUG_FUNC_NAME("CClientSession::GetIpPortByHttpProto");
    DEBUG(LL_ALL, "%s:Begin", funcName);
    DEBUG(LL_DBG, "%s:try to create socket.", funcName);
    if (RET_ERROR == iSocket->MakeTcpConn(&iTcpClntAddrInfo)) {
        LOG(LL_ERROR, "%s:MakeTcpConn.ip:(%s),port:(%d),error:(%s).",
            funcName, iTcpClntAddrInfo.iAddr.c_str(),
            iTcpClntAddrInfo.iPort, strerror(errno));
        return RET_ERROR;
    }
    char requBuffer[MAX_SIZE_1K+1] = {0};
    snprintf(requBuffer, sizeof(requBuffer),
        "GET /gets/%s HTTP/1.1\r\n\r\n", aService.c_str());
    if (RET_ERROR == iSocket->TcpSend(iTcpClntAddrInfo.iSockId,
        requBuffer, strlen(requBuffer), iSocketRWTimeOut)) {
        LOG(LL_ERROR, "%s:send sync to agent:(%s:%d) error:(%s).",
            funcName, iTcpClntAddrInfo.iAddr.c_str(),
            iTcpClntAddrInfo.iPort, strerror(errno));
        return RET_ERROR;
    }
    if (RET_ERROR == iSocket->TcpRecvHttpHeader(iTcpClntAddrInfo.iSockId,
        requBuffer, MAX_SIZE_1K, iSocketRWTimeOut)) {
        LOG(LL_ERROR, "%s:sockid:(%d),recv data error:(%s).",
            funcName, iTcpClntAddrInfo.iSockId, strerror(errno));
        return RET_ERROR;
    }
    snprintf(iSessionBuffer, sizeof(iSessionBuffer), "%s", strstr(requBuffer, "\r\n\r\n")+2);
    iSessionBuffer[0] = 'C';
    iSessionBuffer[1] = 'K';
    DEBUG(LL_ALL, "%s:End", funcName);
    return RET_OK;
}

// ---------------------------------------------------------------------------
// int CSessionHandler::GetIpFromAgent(string aService)
//
// Add getting service request into queue.
// ---------------------------------------------------------------------------
//
int CClientSession::GetIpFromAgent(const string& aService,
                                   SAddrInfo* aAddr) {
    DEBUG_FUNC_NAME("CClientSession::GetIpFromAgent");
    DEBUG(LL_ALL, "%s:Begin.", funcName);
    if (aAddr == NULL) {
        LOG(LL_ERROR, "%s:aAddr is null.", funcName);
        return RET_ERROR;
    }
    // send request 'GETSERVICE {SERVICE_NAME}'
    memset(iSessionBuffer, 0, sizeof(iSessionBuffer));
    string serviceReqStr = "GETSERVICE {";
    serviceReqStr += aService;
    serviceReqStr += "}";
    if (((CStatisticInfos *)0)->MakePkg(iSessionBuffer,
        serviceReqStr, PKG_CMD_REQUEST) == RET_ERROR) {
        LOG(LL_ERROR, "%s:make pkg failed. get service info:(%s).",
            funcName, aService.c_str());
        return RET_ERROR;
    }
    string tmpService = "{";
    tmpService += aService;
    tmpService += "}";
    int tryTimes = 3, ret = RET_ERROR;
    do {
        ret = iSocket->UdpSend(iUdpClntAddrInfo.iSockId,
            iSessionBuffer, strlen(iSessionBuffer),
            iSocketRWTimeOut, &iUdpClntAddrInfo.iSockUnion);
        if (ret == RET_ERROR) {
            LOG(LL_ERROR,
                "%s:make request error for service:(%s), try again...",
                funcName, aService.c_str());
            continue;
        }
        int recvTryTimes = 10;
        do {
            ret = iSocket->UdpRecv(iUdpClntAddrInfo.iSockId,
                iSessionBuffer, PKG_MAX_LEN, iSocketRWTimeOut,
                &iUdpClntAddrInfo.iSockUnion);
        } while (recvTryTimes-- > 0 &&
                  strstr(iSessionBuffer, tmpService.c_str()) == NULL);
    } while (ret == RET_ERROR && tryTimes-- > 0);
    if (ret == RET_ERROR) {
        LOG(LL_ERROR, "%s:UDP getting ip-port error for service:(%s).",
            funcName, aService.c_str());
        int index = 0;
        do {
            iTcpClntAddrInfo = iAgentCenterList.front();
            ret = GetIpPortByHttpProto(aService);
            CloseResetSock(&iTcpClntAddrInfo.iSockId);
            if (ret == RET_OK) {
                DEBUG(LL_DBG, "%s:get service:(%s) from agent:(%s:%d) ok.",
                    funcName, aService.c_str(), iTcpClntAddrInfo.iAddr.c_str(),
                    iTcpClntAddrInfo.iPort);
                break;
            } else {
                LOG(LL_WARN, "%s:error get service:(%s) from agent:(%s:%d)",
                    funcName, aService.c_str(), iTcpClntAddrInfo.iAddr.c_str(),
                    iTcpClntAddrInfo.iPort);
                if (iAgentCenterList.size() > 1) {
                    iAgentCenterList.pop_front();
                    iAgentCenterList.push_back(iTcpClntAddrInfo);
                }
            }
        } while (index++ < iAgentCenterList.size());
        if (ret == RET_ERROR) {
            LOG(LL_ERROR, "%s:TCP getting ip-port error for service:(%s).",
                funcName, aService.c_str());
            return RET_ERROR;
        }
    }
    char itemBuf[PKG_MAX_LEN] = {0};
    char protoType[21] = {0};
    // {SERVICE} {IP} {PORT} {TYPE} {PID}
    int pid = 0;
    ret = sscanf(iSessionBuffer,
        "%*[^{]{%200[^}]%*[^{]{%64[^}]%*[^{]{%d%*[^{]{%20[^}]%*[^{]{%d",
        itemBuf, itemBuf+SERVICE_ITEM_LEN, &aAddr->iPort, protoType, &pid);
    if (ret < 3) {
        LOG(LL_ERROR, "%s:session response:(%s),decode failed,ret:(%d).",
            funcName, iSessionBuffer, ret);
        return RET_ERROR;
    }
    if (aService != itemBuf) {
        LOG(LL_ERROR, "%s:recv service name:(%s), need service name:(%s).",
            funcName, itemBuf, aService.c_str());
        return RET_ERROR;
    }
    if (ret == 4 && strcmp(protoType, "YF.FY") != 0) {
        LOG(LL_ERROR, "%s:recv service type:(%s), need service type:(YF.FY).",
            funcName, protoType);
        return RET_ERROR;
    }

    aAddr->iAddr = itemBuf+SERVICE_ITEM_LEN;
    aAddr->iProtocol = TCP;
    DEBUG(LL_ALL, "%s:End", funcName);
    return RET_OK;
}

// ---------------------------------------------------------------------------
// int CSessionHandler::LoginAndLogout(string aRequest, int aFlag)
//
// login and logout
// ---------------------------------------------------------------------------
//
int CClientSession::LoginAndLogout(const string& aRequest,
                                   const string& aFlag) {
    DEBUG_FUNC_NAME("CClientSession::LoginAndLogout");
    DEBUG(LL_ALL, "%s:Begin.", funcName);
    memset(iSessionBuffer, 0, sizeof(iSessionBuffer));
    if (((CStatisticInfos *)0)->MakePkg(iSessionBuffer,
        aRequest, aFlag) == RET_ERROR) {
        LOG(LL_ERROR, "%s:make pkg failed. request info:(%s).",
            funcName, aRequest.c_str());
        return RET_ERROR;
    }
    int tryTimes = 3, ret = RET_ERROR;
    do {
        ret = iSocket->UdpSend(iUdpClntAddrInfo.iSockId,
            iSessionBuffer, strlen(iSessionBuffer),
            iSocketRWTimeOut, &iUdpClntAddrInfo.iSockUnion);
    } while (ret == RET_ERROR && tryTimes-- > 0);
    if (ret == RET_ERROR) {
        LOG(LL_ERROR, "%s:make request:(%s) error, try again...",
            funcName, aRequest.c_str());
        return RET_ERROR;
    }
    DEBUG(LL_ALL, "%s:End", funcName);
    return RET_OK;
}

// ---------------------------------------------------------------------------
// int CSessionHandler::GetAgentCenterList()
//
// getting agent center address list.
// ---------------------------------------------------------------------------
//
int CClientSession::GetAgentCenterList() {
    DEBUG_FUNC_NAME("CClientSession::GetAgentCenterList");
    DEBUG(LL_ALL, "%s:Begin.", funcName);

    if (GetIpPortByHttpProto("all") == RET_ERROR) {
        LOG(LL_ERROR, "%s:getting all agent center ip-port list error.",
            funcName);
        CloseResetSock(&iTcpClntAddrInfo.iSockId);
        return RET_ERROR;
    }
    CloseResetSock(&iTcpClntAddrInfo.iSockId);
    DEBUG(LL_VARS, "%s:iplist:(%s)", funcName, iSessionBuffer);
    char *p = iSessionBuffer;
    bool isClearList = true;
    while (p != NULL) {
        char itemBuf[PKG_MAX_LEN] = {0};
        SAddrInfo addrInfo;
        // {SERVICE} {IP} {PORT}
        int ret = sscanf(p,
            "%*[^{]{%[^}]%*[^{]{%[^}]%*[^{]{%d",
            itemBuf, itemBuf+SERVICE_ITEM_LEN, &addrInfo.iPort);
        DEBUG(LL_DBG, "%s:ret:(%d),Serivce:(%s), ip:(%s), port:(%d)",
            funcName, ret, itemBuf, itemBuf+SERVICE_ITEM_LEN, addrInfo.iPort);
        if (ret == 3) {
            if (isClearList) {
                isClearList = false;
                // clear list at first.
                iAgentCenterList.clear();
            }
            addrInfo.iAddr = itemBuf+SERVICE_ITEM_LEN;
            addrInfo.iProtocol = TCP;
            iAgentCenterList.push_back(addrInfo);
        } else {
            break;
        }
        p = strstr(p+1, "\r\n");
    }

    DEBUG(LL_ALL, "%s:End.", funcName);
    return RET_OK;
}

int CClientSession::ReNewService(const string& aService) {
    DEBUG_FUNC_NAME("CClientSession::ReNewService");
    DEBUG(LL_ALL, "%s:Begin.", funcName);
    struct SAddrInfo addrInfo;
    if (GetIpFromAgent(aService, &addrInfo) == RET_OK) {
        DEBUG(LL_DBG, "%s:try to create socket.", funcName);
        if (RET_ERROR == iSocket->MakeTcpConn(&addrInfo)) {
            LOG(LL_ERROR, "%s:CreateSocket.ip:(%s),port:(%d),error:(%s).",
                funcName, addrInfo.iAddr.c_str(),
                addrInfo.iPort, strerror(errno));
            return RET_ERROR;
        }
        char pidRequBuf[12] = {0};
        int len = iSocket->CreatePkgHeader(0, pidRequBuf, "pid");
        if ( RET_ERROR != iSocket->TcpSend(addrInfo.iSockId,
            pidRequBuf, len, 1, true)) {
            if (RET_ERROR != iSocket->TcpRecv(addrInfo.iSockId,
                iSessionBuffer, PKG_HEADER_LEN, 3)) {
                addrInfo.iPeerKey = iSessionBuffer;
            }
        }
        iSockId2Addr[addrInfo.iSockId] = addrInfo;
        return addrInfo.iSockId;
    }
    DEBUG(LL_ALL, "%s:End", funcName);
    return RET_ERROR;
}
// ---------------------------------------------------------------------------
// int CClientSession::GetSockIdByService
//
// getting socket id by service from map.
// ---------------------------------------------------------------------------
//
int CClientSession::GetSockIdByService(const string& aService) {
    CLock scopeLock(iAddrMutex);
    DEBUG_FUNC_NAME("CClientSession::GetSockIdByService");
    DEBUG(LL_ALL, "%s:Begin.", funcName);
    int retSockId = RET_ERROR;
    typeof(iService2SockId.end()) it = iService2SockId.find(aService);
    if (it != iService2SockId.end()) {
        if (!it->second.empty()) {
            retSockId = it->second.front();
            it->second.pop();
        }
    }
    if (retSockId == RET_ERROR) {
        retSockId = ReNewService(aService);
    }
    LOG(LL_DBG, "%s:service:(%s),sockid:(%d).",
        funcName, aService.c_str(), retSockId);
    DEBUG(LL_ALL, "%s:End", funcName);
    return retSockId;
}

int CClientSession::AddSockIdByService(const string& aService, int aSockId) {
    CLock scopeLock(iAddrMutex);
    DEBUG_FUNC_NAME("CClientSession::AddSockIdByService");
    DEBUG(LL_ALL, "%s:Begin.", funcName);
    typeof(iService2SockId.end()) it = iService2SockId.find(aService);
    if (it != iService2SockId.end()) {
        it->second.push(aSockId);
    } else {
        std::queue<int> sockQueue;
        sockQueue.push(aSockId);
        iService2SockId[aService] = sockQueue;
    }
    DEBUG(LL_ALL, "%s:End.", funcName);
    return RET_OK;
}
// end of local file.
