/*
** Copyright (C) 2014 Wang Yaofu
** All rights reserved.
**
**Author:Wang Yaofu voipman@qq.com
**Description: The header file of class CAgentService.
*/

#ifndef _AGENT_CAGENTSERVICE_H_
#define _AGENT_CAGENTSERVICE_H_
#include <map>
#include <string>
#include <iterator>
#include <algorithm>
#include <list>
#include "common/localdef.h"
#include "common/cthread.h"
#include "common/cstatisticinfo.h"
using std::less;
using std::list;
#define FIND_IF_FLAG   0
#define REMOVE_IF_FLAG 1
#define HB_TIME_OUT    20
#define SERVICE_TABLE_TD_SIZE 1024
#define WELCOME_STR_OF_AGENT "<body><h2><b>Thanks for your visit to AGENT Statistics node!</b></h2></body></html>"
#define CFG_HTTP_PAGE_REFRESH_FLAG (0x9001)
#define CFG_HTTP_AUTH_STR          "agent:httpauth"
// max process counts for each service.
#define MAX_PROCESS_EACH_SERVICE 1000
// max agent service counts.
#define MAX_AGENT_SERVICE_COUNTS 10000
// max sync agent counts.
#define MAX_SYNC_AGENT_COUNTS 10
// sync config
#define CFG_SYNC_IP       "syncip"
#define CFG_SYNC_PORT     (0x8001)
#define CFG_SYNC_INTERVAL (0x8002)
#define MSG_TEXT_ALL         "all"

enum EMsgType {
    kMsgTypeSyncAll = 0x0101,
    kMsgTypeSyncAny,
    kMsgTypeConf,
    kMsgTypeConfSet,
    kMsgTypeGetsAll,
    kMsgTypeGetsAny,
    kMsgTypeStatAll,
    kMsgTypeStatAny,
    kMsgTypeNone
};
// max seconds for agent center.
// 2036-12-30 00:00:00
#define MAX_AGENT_SECS       (2114179200)
class CConfig;
class CAgentService
{
public:
    /*
    * init data when socket start.
    *
    */
    int InitData();

    // add/del sync agent node web page.
    void MakeConfSetPage();

    /*
    * service ip infos are added into map.
    *
    */
    int AddService(const CStatisticInfos &aServiceInfo);

    /*
    * service ip infos are deleted from map.
    *
    */
    int DelService(CStatisticInfos &aServiceInfo);

    /*
    * Statistic infos is dump into string.
    *
    */
    int DumpAnyServiceInfos(long aNow, const string& aServiceName);

    /*
    * IP+Port is got by service name.
    *
    */
    const string& GetIpPortByService(const string& aService);

    /*
    * all AAAAegntStatistic IP+Port is got.
    *
    */
    const string& GetAllAgentIpPort();

    /*
    * Statistic infos is return.
    *
    */
    const string& GetStatisticInfos(const string& aServiceName);

    /*
    * sync all infos is return.
    *
    */
    const string& GetSyncAllInfos();

    /*
    * sync any infos is return.
    *
    */
    const string& GetSyncAnyInfos(const string& aServiceName);

    /*
    * need infos is return.
    * aFlag=1 : sync, 2:config, 3:getservice, 4:statistic.
    */
    const string& GetRespInfos(const string& aServiceName, EMsgType aFlag);

    /*
    * configuration page is return.
    *
    */
    const string& GetConfSetPage();

    /*
    * configuration service is initialized.
    *
    */
    int ConfigInitial();

    /*
    * split service list by '|'.
    *
    */
    inline string split(std::string s, std::string delim, int& last) {
        size_t index=s.find_first_of(delim, last);
        string retstr("");
        if (index > 0)
        {
            retstr = s.substr(last, index-last);
            last=index+1;
        }
        return retstr;
    }

    /*
    * constructor.
    *
    */
    CAgentService();

    /*
    * Destructor.
    *
    */
    ~CAgentService();

private:
    /*
    * Statistic infos is dump into string by service name.
    *
    */
    string DumpOneServiceInfos(long aNow, const string& aServiceName);

    /*
    * Statistic infos is dump into string.
    *
    */
    int DumpAllServiceName(long aNow);

    /*
    * sync infos is dump into string.
    *
    */
    int DumpAllSyncInfos(long aNow);

    /*
    * sync infos is dump into string.
    *
    */
    int DumpAnySyncInfos(const string& aService, long aNow);

    /*
    * dump the serivce item infos.
    *
    */
    string DumpOneSyncInfos(const string& aService, long aNow);

    /*
    * the condition of thread to start or stop.
    *
    */
    bool iStopRequested;

    /*
    * map for service storing. <key:service,value:<key:ipport,value:count>>
    *
    */
    std::map<string, list<CStatisticInfos>, less<string> > iMsVcS2Addr;

    /*
    * Statistics from map.
    *
    */
    string iStatisticsStr;

    /*
    * sync all from map.
    *
    */
    string iSyncAllStr;

    /*
    * sync any from map.
    *
    */
    string iSyncAnyStr;

    /*
    * config set page.
    *
    */
    string iConfSetPageStr;

    /*
    * timestamp for dumping Statistics from map.
    *
    */
    long iDumpStatisticsInteval;

    /*
    * timestamp for dumping sync from map.
    *
    */
    long iDumpSyncAllInteval;

    /*
    * timestamp for dumping sync from map.
    *
    */
    long iDumpSyncAnyInteval;

    /*
    * get all agent centerfrom map.
    *
    */
    string iAllAgentCenterStr;

    /*
    * get all agent centerfrom map.
    *
    */
    string iOneServiceIpPort;

    /*
    * timestamp for dumping all agent center from map.
    *
    */
    long iDumpAllAgentInteval;

    /*
    * mutex of service.
    *
    */
    CMutex iServiceMutex;

    /*
    * config instance pointer to CConfig.
    *
    */
    CConfig *iConfig;
};

#endif  // _AGENT_CAGENTSERVICE_H_
