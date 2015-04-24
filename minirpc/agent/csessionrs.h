/*
** Copyright (C) 2014 Wang Yaofu
** All rights reserved.
**
**Author:Wang Yaofu voipman@qq.com
**Description: The header file of class CSessionRS.
*/

#ifndef _AGENT_CSESSIONRS_H_
#define _AGENT_CSESSIONRS_H_
#include <vector>
#include "agent/cagentservice.h"
#include "common/localdef.h"
#include "common/cthread.h"
#include "common/csocket.h"

using std::vector;
class CConfig;
class CSessionRS : public CThread
{
public:
    /*
    * init data when socket start.
    *
    */
    int InitData();

    /*
    * increase out pkgs.
    *
    */
    inline void IncrOutPks() {
        iStatisticInfos.iOutputPkgs++;
    }

    /*
    * set local ip address.
    *
    */
    inline void SetLocalIP(const string& aIp) {
        iStatisticInfos.iIpAddress = aIp;
    }

    /*
    * constructor.
    *
    */
    CSessionRS();

    /*
    * Destructor.
    *
    */
    ~CSessionRS();
private:
    /*
    * start the thread.
    *
    */
    void run();

    /*
    * stop the thread.
    *
    */
    void on_stop();

    /*
    * session handler method.
    *
    */
    void SessionHandler();

    /*
    * get request from queue and handle it.
    *
    */
    void SessionReqHandle(struct SAddrInfo *aPeerInfo);

    /*
    * constructor.
    *
    */
    inline bool HeadCheck() {
        // 0x27=' 0x46=F 0x59=Y
        return (iPkgBuffer[0] == 0x27 && iPkgBuffer[1] == 0x46 && iPkgBuffer[2] == 0x59);
    }

    /*
    * the condition of thread to start or stop.
    *
    */
    bool iStopRequested;

    /*
    * socket instance pointer to CSocket.
    *
    */
    CSocket *iSocket;
    CAgentService *iAgentService;
    /*
    * config instance pointer to CConfig.
    *
    */
    CConfig *iConfig;

    /*
    * socket address port infos.
    *
    */
    struct SAddrInfo iUdpSerAddrInfo;

    /*
    * udp package buffer.
    *
    */
    char iPkgBuffer[PKG_MAX_LEN+1];

    /*
    * socket reading or writing time-out.
    *
    */
    int iSocketRWTimeOut;

    /*
    * statistic Informations.
    *
    */
    CStatisticInfos iStatisticInfos;
    // input packages.
    long long iLastInputPkgs;
    // last secs.
    long iLastSecs;
};

#endif  // _AGENT_CSESSIONRS_H_
