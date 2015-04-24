/*
** Copyright (C) 2014 Wang Yaofu
** All rights reserved.
**
**Author:Wang Yaofu voipman@qq.com
**Description: The header file of class CClientSession.
*/

#ifndef _CLIENT_CLIENTSESSION_H_
#define _CLIENT_CLIENTSESSION_H_
#include <unistd.h>
#include <list>
#include <map>
#include <string>
#include <functional>
#include "common/localdef.h"
#include "common/csocket.h"
#include "common/cthread.h"
using std::less;
class CConfig;
class CClientSession {
 public:
    /*
    * Initial operation.
    *
    */
    int InitData();

    /*
    * close socket id from one service.
    *
    */
    int EraseServiceSock(int aSockId);

    int GetSockAddrInfo(SAddrInfo* aAddr);

    /*
    * get ip port by service name from agent with HTTP protocol.
    *
    */
    int GetIpPortByHttpProto(const string& aService);

    /*
    * getting service request from agent node.
    *
    */
    int GetIpFromAgent(const string& aService, SAddrInfo* aAddr);

    /*
    * login and logout
    *
    */
    int LoginAndLogout(const string& aRequest, const string& aFlag);

    /*
    * getting agent center address list.
    *
    */
    int GetAgentCenterList();

    // get ip.port by service from agent server.
    int ReNewService(const string& aService);

    /*
    * getting socket id by service from map.
    *
    */
    int GetSockIdByService(const string& aService);

    /*
    * setting socket id by service to map.
    *
    */
    int AddSockIdByService(const string& aService, int aSockId);

    /*
    * constructor.
    *
    */
    CClientSession();

    /*
    * destructor.
    *
    */
    ~CClientSession();
 private:
    /*
    * close socket and set to 0.
    *
    */
    inline void CloseResetSock(int* aSockId) {
        if (*aSockId > 0) {
            close(*aSockId);
            *aSockId = 0;
        }
    }

 private:
    /*
    * socket instance pointer to CSocket.
    *
    */
    CSocket *iSocket;

    /*
    * config instance pointer to CConfig.
    *
    */
    CConfig *iConfig;

    /*
    * socket address port infos for getting session by UDP.
    *
    */
    struct SAddrInfo iUdpClntAddrInfo;

    /*
    * socket address port infos for getting session by HTTP.
    *
    */
    struct SAddrInfo iTcpClntAddrInfo;

    /*
    * current socket id.
    *
    */
    int iCurSocketId;

    /*
    * socket reading or writing time-out.
    *
    */
    int iSocketRWTimeOut;

    /*
    * map for service storing. <key:service,value:ip>
    *
    */
    std::map<string, std::queue<int> > iService2SockId;

    std::map<int, struct SAddrInfo > iSockId2Addr;

    /*
    * list for getting all agent center address.
    *
    */
    std::list<struct SAddrInfo> iAgentCenterList;

    /*
    * session buffering.
    *
    */
    char iSessionBuffer[PKG_MAX_LEN];
    /*
    * mutex of server to ip pair.
    *
    */
    CMutex iAddrMutex;
};

#endif  // _CLIENT_CLIENTSESSION_H_
