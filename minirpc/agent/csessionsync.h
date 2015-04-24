/*
** Copyright (C) 2012 AsiaInfo
** All rights reserved.
**
**Author:Wang Yaofu voipman@qq.com
**Description: The header file of class CSessionSync.
*/

#ifndef _AGENT_CSESSIONSYNC_H_
#define _AGENT_CSESSIONSYNC_H_
#include <cstdlib>
#include <unistd.h>
#include "agent/cagentservice.h"
#include "common/localdef.h"
#include "common/csocket.h"
#include "common/cthread.h"

class CSyncAddrInfo {
public:
    CSyncAddrInfo() {
        iPort = 0;
        iInterval = 5;
        iLastSyncSecs = 0;
        iStartSecs = 0;
        iTryTimes = 0;
        iServices = MSG_TEXT_ALL;
    }
    CSyncAddrInfo(string aIp, int aPort, int aInterval)
        : iPort(aPort), iInterval(aInterval) {
        iIpAddress = aIp;
        iLastSyncSecs = 0;
        iStartSecs = 0;
        iTryTimes = 0;
    }
    string iIpAddress;
    int iPort;
    int iInterval;
    time_t iLastSyncSecs;
    time_t iStartSecs;
    string iLastSyncSecsStr;
    string iStartSecsStr;
    int iTryTimes;
    string iServices;
    const CSyncAddrInfo& operator = (const CSyncAddrInfo &m) {

        this->iIpAddress       = m.iIpAddress;
        this->iPort     = m.iPort;
        this->iInterval = m.iInterval;
        this->iLastSyncSecs = m.iLastSyncSecs;
        this->iStartSecs = m.iStartSecs;
        this->iLastSyncSecsStr = ctime(&m.iLastSyncSecs);
        this->iStartSecsStr = ctime(&m.iStartSecs);
        this->iTryTimes = m.iTryTimes;
        this->iServices = m.iServices;
        return *this;
    }
    bool operator () (const CSyncAddrInfo &m) const
    {
        return iIpAddress == m.iIpAddress && iPort == m.iPort;
    }
};
class CConfig;
class CSessionSync : public CThread
{
public:
    /*
    * init data when socket start.
    *
    */
    int InitData();

    /*
    * close sync error socket id.
    *
    */
    inline int CloseSyncSockId() {
        return (iSyncAddrInfo.iSockId > 0) ? \
            close(iSyncAddrInfo.iSockId) : 0;
    }

    /*
    * add sync address into list.
    *
    */
    inline void AddSyncAddr(CSyncAddrInfo &aSyncAddrInfo) {
        list<CSyncAddrInfo>::iterator it;
        CLock scopeLock(iSyncMutex);
        it = find_if(iSyncAddrList.begin(),
            iSyncAddrList.end(),
            aSyncAddrInfo);
        if (it == iSyncAddrList.end())
        {
            if (iSyncAddrList.size() <= MAX_SYNC_AGENT_COUNTS)
            {
                iSyncAddrList.push_back(aSyncAddrInfo);
            }
        } else {
            *it = aSyncAddrInfo;
        }
    }

    /*
    * delete sync address from list.
    *
    */
    inline void DelSyncAddr(CSyncAddrInfo &aSyncAddrInfo) {
        list<CSyncAddrInfo>::iterator it;
        CLock scopeLock(iSyncMutex);
        it = find_if(iSyncAddrList.begin(),
            iSyncAddrList.end(),
            aSyncAddrInfo);
        if (it != iSyncAddrList.end())
        {
            iSyncAddrList.erase(it);
        }
    }
    /*
    * return sync address string.
    *
    */
    const string& GetSyncString() {
        int now = CThreadTimer::instance()->getNowSecs();
        if (now - 1 >= iDumpSyncInteval)
        {
            iDumpSyncInteval = now;
            DumpSyncAddrList();
        }
        return iSyncAddrStr;
    }

    /*
    * constructor.
    *
    */
    CSessionSync();

    /*
    * Destructor.
    *
    */
    ~CSessionSync();
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
    * dump sync address list into string.
    *
    */
    int DumpSyncAddrList();

    /*
    * sync handler method.
    *
    */
    int SyncHandler(const string& aServices);

    /*
    * get sync response from sync master agent.
    *
    */
    int GetSyncResponse();

    /*
    * handle sync data.
    *
    */
    void SyncDataHandle();


    /*
    * get line.
    *
    */
    inline char *GetLine(char *aBuf) {
        char *p = strstr(aBuf, "\nS {");
        return (p == NULL) ? NULL : p + 1;
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

    /*
    * config instance pointer to CConfig.
    *
    */
    CConfig *iConfig;

    /*
    * socket address port infos.
    *
    */
    struct SAddrInfo iSyncAddrInfo;

    /*
    * list for sync agent addree storing.
    *
    */
    list<CSyncAddrInfo> iSyncAddrList;

    /*
    * socket reading or writing time-out.
    *
    */
    int iSocketRWTimeOut;

    /*
    * single http sync buffer.
    *
    */
    char *iHttpSyncBuffer;

    /*
    * mutex of sync address list.
    *
    */
    CMutex iSyncMutex;
    /*
    * sync addresss from list.
    *
    */
    string iSyncAddrStr;

    /*
    * timestamp for dumping sync address from list.
    *
    */
    long iDumpSyncInteval;
};

#endif  // _AGENT_CSESSIONSYNC_H_
