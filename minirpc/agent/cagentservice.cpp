/*
** Copyright (C) 2014 Wang Yaofu
** All rights reserved.
**
**Author:Wang Yaofu voipman@qq.com
**Description: The source file of class CAgentService.
*/
#include <fcntl.h>
#include <unistd.h>
#include <csignal>
#include <cstdio>
#include <cstdlib>
#include "agent/cagentservice.h"
#include "agent/csessionsync.h"
#include "common/cconfig.h"
#include "common/clogwriter.h"
#include "common/csingleton.h"
#include "common/cstringutils.h"
#ifdef MONITOR_EMAIL
#include "agent/cagentmonitor.h"
#endif
using namespace wyf;
// ---------------------------------------------------------------------------
// CAgentService::CAgentService()
//
// constructor.
// ---------------------------------------------------------------------------
//
CAgentService::CAgentService() : iStopRequested(false) {
}

// ---------------------------------------------------------------------------
// CAgentService::~CAgentService()
//
// Destructor.
// ---------------------------------------------------------------------------
//
CAgentService::~CAgentService() {
}

// ---------------------------------------------------------------------------
// int CAgentService::InitData()
//
// init operation where local class is created.
// ---------------------------------------------------------------------------
//
int CAgentService::InitData() {
    const char *funcName = "CAgentService::InitData";
    LOG(LL_ALL, "%s::Begin.", funcName);
    iStatisticsStr = WELCOME_STR_OF_AGENT;
    iDumpStatisticsInteval = time(NULL);
    iDumpSyncAllInteval = iDumpStatisticsInteval;
    iDumpSyncAnyInteval = iDumpSyncAllInteval;
    iDumpAllAgentInteval = iDumpSyncAllInteval;
    iConfig = CSingleton<CConfig>::Instance();
    if (iConfig == NULL) {
        LOG(LL_ALL, "%s::create CConfig error.", funcName);
        return RET_ERROR;
    }

    ConfigInitial();
#if 0
    // add service for testing...
    CStatisticInfos tst;
    tst.iInputPkgTps = 1;
    tst.iService = "BMS";
    tst.iCurConns = 1;
    tst.iHandledConns = 212;
    tst.iServiceType = "Test";
    tst.iLoginName = "wangyf5";
    tst.iPid = 1111;
    tst.iPort = 8091;
    tst.iIpAddress = "10.1.251.24";
    tst.iInputPkgs = 120;
    tst.iOutputPkgs = 240;
    tst.iSecs = CThreadTimer::instance()->getNowSecs();
    tst.iStartTime = tst.iSecs;
    tst.iStartTimeStr = ctime(&tst.iStartTime);
    tst.iRunningTime = tst.iSecs;
    AddService(tst);
    tst.iPid = 1112;
    tst.iSecs += 10;
    AddService(tst);
    tst.iSecs += 5;
    AddService(tst);
    tst.iService = "BMS_INTERFACE";
    tst.iSecs += 5;
    AddService(tst);
    tst.iService = "BMS_CMCCWLAN";
    tst.iSecs += 5;
    AddService(tst);
    tst.iPid = 1111;
    tst.iSecs += 5;
    AddService(tst);
    tst.iSecs += 5;
    tst.iPid = 1113;
    AddService(tst);
    tst.iPid = 1114;
    AddService(tst);
    tst.iSecs += 5;
    tst.iPid = 1115;
    AddService(tst);
    tst.iPid = 1116;
    AddService(tst);
    tst.iPid = 1117;
    AddService(tst);
#endif // #ifdef _DEBUG
#ifdef MONITOR_EMAIL
    try {
        CSingleton<CAgentMonitor>::Instance()->start();
    } catch (string catchStr) {
        LOG(LL_ERROR, "%s:start agent monitor error:(%s).",
            funcName, catchStr.c_str());
    }
#endif
    MakeConfSetPage();
    LOG(LL_ALL, "%s:End", funcName);
    return 0;
}

void CAgentService::MakeConfSetPage() {
    iConfSetPageStr = "<body><script>\n"
        "function check(){"
        "if(document.getElementById(\"ch\").checked==true) {\n"
        "document.getElementById(\"sub\").value=\"Delete...\";\n"
        "document.getElementById(\"ip\").name=\"delsyncip\";\n"
        "} else {\n"
        "document.getElementById(\"sub\").value=\"Add...\";\n"
        "document.getElementById(\"ip\").name=\"addsyncip\";\n"
        "}}\n</script><h1>Agent Center</h1>"
        "<h2>Statistics Report services setting page.</h2>"
        "<hr><h3>&gt; General service setting information</h3>"
        "<form action=\"/conf/conf\" method=\"get\">"
        "<table class=frontend>"
        "<tr><td><b>Remote Agent IP Address:</b></td>"
        "<td><input class=in type=text id=ip name=addsyncip value=127.0.0.1 /></td></tr>"
        "<tr><td><b>Remote Agent Port:</b></td>"
        "<td><input class=in type=text name=port value=15218 /></td></tr>"
        "<tr><td><b>Synchronous Interval:</b></td>"
        "<td><input class=in type=text name=interval value=5 /></td></tr>"
        "<tr><td><b>What's Services:</b></td>"
        "<td><input class=in type=text name=services value=all /></td></tr>"
        "<tr><td><h3><input class=in type=checkbox id=ch onclick=\"check()\"/>Del?</h3>"
        "<td><input class=in type=submit id=sub value=\"Add...\"/></tr>"
        "</table></form><h3>&gt; Have fun!</h3><hr>"
        "<p width=80% align=center>Copyright (C) 2015 Wang Yaofu</p></body></html>";
}


// ---------------------------------------------------------------------------
// int CAgentService::AddService()
//
// Add service ip info into map.
// ---------------------------------------------------------------------------
//
int CAgentService::AddService(const CStatisticInfos &aServiceInfo) {
    const char *funcName = "CAgentService::AddService";
    DEBUG(LL_ALL, "%s:Begin", funcName);
    LOG(LL_VARS, "%s:service:(%s),"
        "service type:(%s), user name:(%s),"
        "ip:(%s), port:(%d), pid:(%d),"
        "start time:(%lu), running time:(%lu),"
        "current conns:(%lu), handled conns:(%lld),"
        "input pkgs:(%lld), output pkgs:(%lld),"
        "inputpkg tps:(%lld),vectordesc:(%s).",
        funcName, aServiceInfo.iService.c_str(),
        aServiceInfo.iServiceType.c_str(), aServiceInfo.iLoginName.c_str(),
        aServiceInfo.iIpAddress.c_str(), aServiceInfo.iPort, aServiceInfo.iPid,
        aServiceInfo.iStartTime, aServiceInfo.iRunningTime,
        aServiceInfo.iCurConns, aServiceInfo.iHandledConns,
        aServiceInfo.iInputPkgs, aServiceInfo.iOutputPkgs,
        aServiceInfo.iInputPkgTps, aServiceInfo.iVectorInfo.c_str());
    CLock scopeLock(iServiceMutex);
    if (iMsVcS2Addr.size() >= MAX_AGENT_SERVICE_COUNTS) {
        LOG(LL_WARN, "%s:too many service:(%d).", funcName, iMsVcS2Addr.size());
        return RET_ERROR;
    }
    if (iMsVcS2Addr.find(aServiceInfo.iService)
        != iMsVcS2Addr.end()) {
        if (iMsVcS2Addr[aServiceInfo.iService].size()
            >= MAX_PROCESS_EACH_SERVICE) {
            LOG(LL_WARN, "%s:too many process for service:(%s).",
                funcName, aServiceInfo.iService.c_str());
            return RET_ERROR;
        }
        typeof(iMsVcS2Addr[aServiceInfo.iService].end()) it =
            find_if(iMsVcS2Addr[aServiceInfo.iService].begin(),
                    iMsVcS2Addr[aServiceInfo.iService].end(),
                    CStatisticInfos(aServiceInfo.iIpAddress,
                    aServiceInfo.iPort, aServiceInfo.iPid, 0, FIND_IF_FLAG));
        if (it == iMsVcS2Addr[aServiceInfo.iService].end()) {
            LOG(LL_DBG, "%s:new-service:(%s),ip:(%s),port:(%d),pid:(%d)",
                funcName, aServiceInfo.iService.c_str(), aServiceInfo.iIpAddress.c_str(),
                aServiceInfo.iPort, aServiceInfo.iPid);
            iMsVcS2Addr[aServiceInfo.iService].push_back(aServiceInfo);
        } else {
            LOG(LL_DBG, "%s:exist-service:(%s),ip:(%s),port:(%d),pid:(%d)",
                funcName, it->iService.c_str(), it->iIpAddress.c_str(),
                it->iPort, it->iPid);
            if (aServiceInfo.iSecs > it->iSecs ||
                aServiceInfo.iSecs == MAX_AGENT_SECS) {
                // update service item when service state is changed.
                *it = aServiceInfo;
            }
        }
    } else {
        iMsVcS2Addr[aServiceInfo.iService].push_back(aServiceInfo);
    }
    DEBUG(LL_ALL, "%s:End", funcName);
    return RET_OK;
}

// ---------------------------------------------------------------------------
// int CAgentService::DelService()
//
// Delete service ip info from map.
// ---------------------------------------------------------------------------
//
int CAgentService::DelService(CStatisticInfos &aServiceInfo) {
    const char *funcName = "CAgentService::DelService";
    DEBUG(LL_ALL, "%s:Begin", funcName);
    LOG(LL_VARS, "%s:service:(%s),"
        "service type:(%s), user name:(%s),"
        "ip:(%s), port:(%d), pid:(%d)",
        funcName, aServiceInfo.iService.c_str(),
        aServiceInfo.iServiceType.c_str(), aServiceInfo.iLoginName.c_str(),
        aServiceInfo.iIpAddress.c_str(), aServiceInfo.iPort, aServiceInfo.iPid);
    CLock scopeLock(iServiceMutex);
    if (iMsVcS2Addr.size() <= 0) {
        LOG(LL_WARN, "%s:null service list.", funcName);
        return RET_ERROR;
    }
    if (iMsVcS2Addr.find(aServiceInfo.iService)
        != iMsVcS2Addr.end()) {
        typeof(iMsVcS2Addr[aServiceInfo.iService].end()) it =
        find_if(iMsVcS2Addr[aServiceInfo.iService].begin(),
                iMsVcS2Addr[aServiceInfo.iService].end(),
                CStatisticInfos(aServiceInfo.iIpAddress,
                aServiceInfo.iPort, aServiceInfo.iPid, 0, FIND_IF_FLAG));
        if (it != iMsVcS2Addr[aServiceInfo.iService].end()) {
            iMsVcS2Addr[aServiceInfo.iService].erase(it);
        }
    }
    DEBUG(LL_ALL, "%s:End", funcName);
    return RET_OK;
}

// ---------------------------------------------------------------------------
// string CAgentService::GetIpInfoByService(string aService)
//
// IP+Port is got by service name.
// ---------------------------------------------------------------------------
//
const string& CAgentService::GetIpPortByService(const string& aService) {
    const char *funcName = "CAgentService::GetIpPortByService";
    DEBUG(LL_ALL, "%s:Begin", funcName);
    DEBUG(LL_VARS, "%s:service:(%s)", funcName, aService.c_str());
    CLock scopeLock(iServiceMutex);
    char tmpBuf[256] = {0};
    if (iMsVcS2Addr.find(aService) != iMsVcS2Addr.end()) {
        if (iMsVcS2Addr[aService].size() > 0) {
            iMsVcS2Addr[aService].sort();
            iMsVcS2Addr[aService].front().iCurConns++;
            snprintf(tmpBuf, sizeof(tmpBuf),
                "{%s} {%s} {%d} {%s}", aService.c_str(),
                iMsVcS2Addr[aService].front().iIpAddress.c_str(),
                iMsVcS2Addr[aService].front().iPort,
                iMsVcS2Addr[aService].front().iServiceType.c_str());
        } else {
            // maybe no data in list.
            snprintf(tmpBuf, sizeof(tmpBuf), "{%s} {not found}", aService.c_str());
        }
        LOG(LL_VARS, "%s:ret:(%s).", funcName, tmpBuf);
    } else {
        LOG(LL_NOTICE, "%s:not ip-port for service:(%s)",
            funcName, aService.c_str());
        snprintf(tmpBuf, sizeof(tmpBuf), "{%s} {not found}", aService.c_str());
    }
    iOneServiceIpPort = tmpBuf;
    return iOneServiceIpPort;
}

// ---------------------------------------------------------------------------
// string CAgentService::GetAllAgentIpPort()
//
// all AAAAegntStatistic IP+Port is got.
// ---------------------------------------------------------------------------
//
const string& CAgentService::GetAllAgentIpPort() {
    const char *funcName = "CAgentService::GetAllAgentIpPort";
    DEBUG(LL_ALL, "%s:Begin", funcName);
    int now = CThreadTimer::instance()->getNowSecs();
    if (now - 1 >= iDumpAllAgentInteval) {
        iDumpAllAgentInteval = now;
    } else {
        return iAllAgentCenterStr;
    }
    CLock scopeLock(iServiceMutex);
    iAllAgentCenterStr.clear();
    char ipport[SERVICE_ITEM_LEN+1] = {0};
    string agentCenterStr = "AAAAgentStatisticInfo";
    if (iMsVcS2Addr.find(agentCenterStr) != iMsVcS2Addr.end()) {
        int ii = 0, jj = 0;
        typeof(iMsVcS2Addr[agentCenterStr].end()) listIt;
        for (listIt = iMsVcS2Addr[agentCenterStr].begin();
            listIt != iMsVcS2Addr[agentCenterStr].end();
            ++listIt) {
            snprintf(ipport, SERVICE_ITEM_LEN,
                "{%s} {%s} {%d}\r\n",
                agentCenterStr.c_str(), listIt->iIpAddress.c_str(), listIt->iPort);
            iAllAgentCenterStr += ipport;
        }
        LOG(LL_VARS, "%s:ret:(%s).", funcName, ipport);
    } else {
        LOG(LL_NOTICE, "%s:not ip-port for service:(%s)",
            funcName, agentCenterStr.c_str());
        snprintf(ipport, SERVICE_ITEM_LEN, "{%s} {not found}",
            agentCenterStr.c_str());
        iAllAgentCenterStr = ipport;
    }
    DEBUG(LL_ALL, "%s:End", funcName);
    return iAllAgentCenterStr;
}

// ---------------------------------------------------------------------------
// string CAgentService::DumpOneServiceInfos()
//
// Statistic infos dump into string and timeout service node is deleted.
// ---------------------------------------------------------------------------
//
string CAgentService::DumpOneServiceInfos(long aNow, const string& aServiceName) {
    const char *funcName = "CAgentService::DumpOneServiceInfos()";
    DEBUG(LL_ALL, "%s:Begin", funcName);
    DEBUG(LL_ALL, "%s:dump infos at:(%lu)", funcName, aNow);
    if (iMsVcS2Addr.find(aServiceName) != iMsVcS2Addr.end()) {
        char tabletd[SERVICE_TABLE_TD_SIZE] = {0};
        DEBUG(LL_DBG, "%s:start to dump service:(%s).",
            funcName, aServiceName.c_str());
        string serviceItem = "<table class=tbl width=95% align=center>"
            "<tr><th class=s align=left colspan=14><h3><a href=\"/";
        serviceItem += aServiceName;
        serviceItem += "\">";
        serviceItem += aServiceName;
        serviceItem += "</a>";
        serviceItem += (aServiceName == "AAAAgentStatisticInfo") ?
            "</h3><a href=\"/set\">Add More Agents Service Node Here...</a>"
            "|<a href=\"/conf\">Agents List...</a>"
            "|<a href=\"/all\">Agents Basics...</a>"
            "|<a href=\"/sync/all\">Sync Basics...</a>" : "</h3>";
        serviceItem += "</tr>"
            "<tr class=s>"
            "<th>NO.</th>"
            "<th>Type</th>"
            "<th>Login Name</th>"
            "<th>Ip Address</th>"
            "<th>Port</th>"
            "<th>Process Id</th>"
            "<th>Start Time</th>"
            "<th>Running Time</th>"
            "<th>Current Conns</th>"
            "<th>Handled Conns</th>"
            "<th>In Packages</th>"
            "<th>Out Packages</th>"
            "<th>Package TPS</th>"
            "<th>Vector Desc.</th>"
            "</tr>";
        int ii = 0, jj = 0;
        for (typeof(iMsVcS2Addr[aServiceName].end()) listIt =
            iMsVcS2Addr[aServiceName].begin();
            listIt != iMsVcS2Addr[aServiceName].end();
            ++listIt) {
            if (listIt->iSecs > aNow - HB_TIME_OUT) {
                ii = jj++ % 7;
                sprintf(tabletd, "<tr class=a%d>"
                    "<td>%d</td>"
                    "<td>%s</td>"
                    "<td>%s</td>"
                    "<td>%s</td>"
                    "<td>%d</td>"
                    "<td>%d</td>"
                    "<td>%s</td>"
                    "<td>%lu</td>"
                    "<td>%lu</td>"
                    "<td>%lld</td>"
                    "<td>%lld</td>"
                    "<td>%lld</td>"
                    "<td>%lld</td>"
                    "<td>%s</td>"
                    "</tr>",
                    ii, jj,
                    listIt->iServiceType.c_str(),
                    listIt->iLoginName.c_str(),
                    listIt->iIpAddress.c_str(),
                    listIt->iPort,
                    listIt->iPid,
                    listIt->iStartTimeStr.c_str(),
                    listIt->iRunningTime,
                    listIt->iCurConns,
                    listIt->iHandledConns,
                    listIt->iInputPkgs,
                    listIt->iOutputPkgs,
                    listIt->iInputPkgTps,
                    listIt->iVectorInfo.c_str());
                serviceItem += tabletd;
                DEBUG(LL_VARS, "%s:table.td:(%s)", funcName, tabletd);
            } else {
                LOG(LL_WARN, "%s:erase timeout service:(%s),"
                    "ip:(%s),port:(%d),pid:(%d),"
                    "last heart beat time:(%lu).",
                    funcName, aServiceName.c_str(),
                    listIt->iIpAddress.c_str(), listIt->iPort,
                    listIt->iPid, listIt->iSecs);
            #ifdef MONITOR_EMAIL
                // send monitor mail.
                char mailBuffer[MAX_SIZE_2K] = {0};
                sprintf(mailBuffer,
                    "Service: %s.\r\n"
                    "IP: %s.\r\n"
                    "Port: %d.\r\n"
                    "Pid: %d.\r\n"
                    "Handled Connections: %lld.\r\n"
                    "Handled Packages: %lld.\r\n"
                    "Running Seconds: %lu.\r\n"
                    "Start Time: %s"
                    "Dead Time: %s",
                    aServiceName.c_str(),
                    listIt->iIpAddress.c_str(),
                    listIt->iPort,
                    listIt->iPid,
                    listIt->iHandledConns,
                    listIt->iInputPkgs,
                    listIt->iRunningTime,
                    listIt->iStartTimeStr.c_str(),
                    ctime(&listIt->iSecs));
                string subject = aServiceName;
                subject += " service is quit...";
                CSingleton<CAgentMonitor>::Instance()->AddNotifyMsg(subject, mailBuffer);
            #endif
                typeof(listIt) tmplistIt = listIt;
                ++tmplistIt;
                iMsVcS2Addr[aServiceName].erase(listIt);
                listIt = tmplistIt;
                DEBUG(LL_INFO, "%s:erase service:(%s) success.",
                    funcName, aServiceName.c_str());
            }
        }
        serviceItem += "</table><br>";
        if (iMsVcS2Addr[aServiceName].size() == 0) {
            LOG(LL_WARN, "%s:erase service:(%s) node.",
                funcName, aServiceName.c_str());
            iMsVcS2Addr.erase(aServiceName);
        }
        DEBUG(LL_DBG, "%s:dump info:(%s)", funcName, serviceItem.c_str());
        return serviceItem;
    }
    DEBUG(LL_ALL, "%s:End", funcName);
    return string("");
}

// ---------------------------------------------------------------------------
// int CAgentService::DumpAnyServiceInfos()
//
// Statistic infos dump into string and timeout service node is deleted.
// ---------------------------------------------------------------------------
//
int CAgentService::DumpAnyServiceInfos(long aNow, const string& aServiceName) {
    const char *funcName = "CAgentService::DumpAllServiceInfos()";
    DEBUG(LL_ALL, "%s:Begin", funcName);
    DEBUG(LL_ALL, "%s:dump infos at:(%lu)", funcName, aNow);
    CLock scopeLock(iServiceMutex);
    iStatisticsStr.clear();
    string htmlStr = "<body>";
    if (aServiceName == MSG_TEXT_ALL) {
        for (typeof(iMsVcS2Addr.end()) it = iMsVcS2Addr.begin();
            it != iMsVcS2Addr.end();
            ++it) {
            htmlStr += DumpOneServiceInfos(aNow, it->first);
            if (htmlStr.length() > MAX_SIZE_64K + MAX_SIZE_64K) {
                break;
            }
        }
    } else {
        htmlStr += DumpOneServiceInfos(aNow, aServiceName);
    }
    htmlStr += "</body></html>";
    if (htmlStr.length() <= 25) {
        iStatisticsStr = WELCOME_STR_OF_AGENT;
    } else {
        iStatisticsStr = htmlStr;
    }

    LOG(LL_DBG, "%s:dump info:(%s)", funcName, iStatisticsStr.c_str());
    DEBUG(LL_ALL, "%s:End", funcName);
    return RET_OK;
}

// ---------------------------------------------------------------------------
// int CAgentService::DumpAllServiceName()
//
// Statistic infos dump into string and timeout service node is deleted.
// ---------------------------------------------------------------------------
//
int CAgentService::DumpAllServiceName(long aNow) {
    const char *funcName = "CAgentService::DumpAllServiceName()";
    DEBUG(LL_ALL, "%s:Begin", funcName);
    DEBUG(LL_ALL, "%s:dump infos at:(%lu)", funcName, aNow);
    CLock scopeLock(iServiceMutex);
    char trStr[SERVICE_TABLE_TD_SIZE] = {0};
    sprintf(trStr, "<h2>Statistics Report for pid %d with %ld services.</h2>\n",
        getpid(), iMsVcS2Addr.size());
    string htmlStr = "<body><h1>Agent Center</h1>\n";
    htmlStr += trStr;
    htmlStr += "<hr><h3>&gt; General service information</h3>";
    htmlStr += "<table border=0><tr class=a6>"
        "<td>NO.</td><td>Service Name</td>"
        "<td>Instance counts</td></tr>\n";
    int ii = 0, jj = 0;
    for (typeof(iMsVcS2Addr.end()) it = iMsVcS2Addr.begin();
        it != iMsVcS2Addr.end(); ++it) {
        ii = jj++ % 7;
        DEBUG(LL_DBG, "%s:start to dump service:(%s).",
            funcName, it->first.c_str());
        sprintf(trStr, "<tr class=a%d><td>%d</td><td>%s</td><td>%d</td></tr>",
            ii, jj, it->first.c_str(), it->second.size());
        htmlStr += trStr;
        if (htmlStr.length() > MAX_SIZE_64K * 10)
        {
            break;
        }
    }
    htmlStr += "</table><hr><p width=80% align=center>Copyright (C) 2015 Wang Yaofu</p></body></html>";
    iStatisticsStr = htmlStr;

    LOG(LL_DBG, "%s:dump info:(%s)", funcName, iStatisticsStr.c_str());
    DEBUG(LL_ALL, "%s:End", funcName);
    return RET_OK;
}

// ---------------------------------------------------------------------------
// string CAgentService::GetStatisticInfos(string aServiceName)
//
// Statistic infos is return.
// ---------------------------------------------------------------------------
//
const string& CAgentService::GetStatisticInfos(const string& aServiceName) {
    int now = CThreadTimer::instance()->getNowSecs();
    if (now - 1 >= iDumpStatisticsInteval) {
        iDumpStatisticsInteval = now;
        if (aServiceName.size() > 1) {
            if (aServiceName == MSG_TEXT_ALL) {
                DumpAllServiceName(now);
            } else {
                DumpAnyServiceInfos(now, aServiceName);
            }
        } else {
            DumpAnyServiceInfos(now, string(MSG_TEXT_ALL));
        }
    }
    return iStatisticsStr;
}

// ---------------------------------------------------------------------------
// int CAgentService::DumpAllSyncInfos()
//
// sync infos is dump into string.
// ---------------------------------------------------------------------------
//
int CAgentService::DumpAllSyncInfos(long aNow) {
    const char *funcName = "CAgentService::DumpAllSyncInfos()";
    DEBUG(LL_ALL, "%s:Begin", funcName);
    DEBUG(LL_ALL, "%s:dump infos at:(%lu)", funcName, aNow);
    CLock scopeLock(iServiceMutex);
    iSyncAllStr.clear();
    for (typeof(iMsVcS2Addr.end()) it = iMsVcS2Addr.begin();
        it != iMsVcS2Addr.end(); ++it) {
        iSyncAllStr += DumpOneSyncInfos(it->first, aNow);
    }
    if (iSyncAllStr.size() <= 0) {
        iSyncAllStr = "{ERROR} {no data for sync...}\n";
    }
    LOG(LL_DBG, "%s:dump info:(%s)", funcName, iSyncAllStr.c_str());
    DEBUG(LL_ALL, "%s:End", funcName);
    return RET_OK;
}

// ---------------------------------------------------------------------------
// int CAgentService::DumpAnySyncInfos(string aService)
//
// sync infos is dump into string.
// ---------------------------------------------------------------------------
//
int CAgentService::DumpAnySyncInfos(const string& aService, long aNow) {
    const char *funcName = "CAgentService::DumpAnySyncInfos()";
    DEBUG(LL_ALL, "%s:Begin", funcName);
    DEBUG(LL_ALL, "%s:dump infos for services:(%s)",
        funcName, aService.c_str());
    CLock scopeLock(iServiceMutex);
    char tabletd[SERVICE_TABLE_TD_SIZE] = {0};
    iSyncAnyStr.clear();
    int last = 0;
    string substr("");
    do {
        substr = split(aService, "|", last);
        if (iMsVcS2Addr.find(substr) != iMsVcS2Addr.end()) {
            iSyncAnyStr += DumpOneSyncInfos(substr, aNow);
        }
    } while (last > 0);
    if (iSyncAnyStr.size() <= 0) {
        iSyncAnyStr = "{ERROR} {no data for sync...}\n";
    }
    LOG(LL_DBG, "%s:dump info:(%s)", funcName, iSyncAnyStr.c_str());
    DEBUG(LL_ALL, "%s:End", funcName);
    return RET_OK;
}

// ---------------------------------------------------------------------------
// string CAgentService::DumpOneSyncInfos(string aServiceName)
//
// Sync infos is return.
// ---------------------------------------------------------------------------
//
string CAgentService::DumpOneSyncInfos(const string& aServiceName, long aNow) {
    const char *funcName = "CAgentService::DumpOneSyncInfos()";
    DEBUG(LL_ALL, "%s:Begin", funcName);
    DEBUG(LL_DBG, "%s:start to dump service:(%s).",
        funcName, aServiceName.c_str());
    char tabletd[SERVICE_TABLE_TD_SIZE] = {0};
    string serviceItem("");
    for (typeof(iMsVcS2Addr[aServiceName].end()) listIt =
        iMsVcS2Addr[aServiceName].begin();
        listIt != iMsVcS2Addr[aServiceName].end();
        ++listIt) {
        if (listIt->iSecs > aNow - HB_TIME_OUT) {
            /* {ServiceName} {SType} {Uname} {IP}
            *  {port} {PID} {curConns} {HandledConns}
            *  {IS} {OS} {TPS} {RunTime} {StartTime}
            */
            sprintf(tabletd,
                "S {%s} {%s} {%s} {%s} "
                "{%d} {%d} {%lu} {%lld} "
                "{%lld} {%lld} {%lld} {%lu} {%lu} {%lu} {%s}\n",
                listIt->iServiceType.c_str(),
                listIt->iLoginName.c_str(),
                listIt->iService.c_str(),
                listIt->iIpAddress.c_str(),
                listIt->iPort,
                listIt->iPid,
                listIt->iCurConns,
                listIt->iHandledConns,
                listIt->iInputPkgs,
                listIt->iOutputPkgs,
                listIt->iInputPkgTps,
                listIt->iStartTime,
                listIt->iRunningTime,
                (listIt->iSecs == MAX_AGENT_SECS) ? aNow : listIt->iSecs,
                listIt->iVectorInfo.c_str());
            if (serviceItem.size() > 0) {
                serviceItem += tabletd;
            } else {
                serviceItem = tabletd;
            }
            DEBUG(LL_VARS, "%s:table.td:(%s)", funcName, tabletd);
        } else {
            LOG(LL_WARN, "%s:erase timeout service:(%s),"
                "ip:(%s),port:(%d),pid:(%d),"
                "last heart beat time:(%lu).",
                funcName, aServiceName.c_str(),
                listIt->iIpAddress.c_str(), listIt->iPort,
                listIt->iPid, listIt->iSecs);
        #ifdef MONITOR_EMAIL
            // send monitor mail.
            char mailBuffer[MAX_SIZE_2K] = {0};
            sprintf(mailBuffer,
                "Service: %s.\r\n"
                "IP: %s.\r\n"
                "Port: %d.\r\n"
                "Pid: %d.\r\n"
                "Handled Connections: %lld.\r\n"
                "Handled Packages: %lld.\r\n"
                "Running Seconds: %lu.\r\n"
                "Start Time: %s"
                "Dead Time: %s",
                aServiceName.c_str(),
                listIt->iIpAddress.c_str(),
                listIt->iPort,
                listIt->iPid,
                listIt->iHandledConns,
                listIt->iInputPkgs,
                listIt->iRunningTime,
                listIt->iStartTimeStr.c_str(),
                ctime(&listIt->iSecs));
            string subject = aServiceName;
            subject += " service is quit...";
            CSingleton<CAgentMonitor>::Instance()->AddNotifyMsg(subject, mailBuffer);
        #endif
            typeof(listIt) tmplistIt = listIt;
            ++tmplistIt;
            iMsVcS2Addr[aServiceName].erase(listIt);
            listIt = tmplistIt;
            DEBUG(LL_INFO, "%s:erase service:(%s) success.",
                funcName, aServiceName.c_str());
        }
    }
    DEBUG(LL_ALL, "%s:End", funcName);
    return serviceItem;
}

// ---------------------------------------------------------------------------
// string CAgentService::GetSyncInfos(string aServiceName)
//
// Sync infos is return.
// ---------------------------------------------------------------------------
//
const string& CAgentService::GetSyncAllInfos() {
    int now = CThreadTimer::instance()->getNowSecs();
    if (now - 1 >= iDumpSyncAllInteval) {
        iDumpSyncAllInteval = now;
        DumpAllSyncInfos(now);
    }
    return iSyncAllStr;
}

// ---------------------------------------------------------------------------
// string CAgentService::GetSyncAnyInfos(string aServiceName)
//
// Sync infos is return.
// ---------------------------------------------------------------------------
//
const string& CAgentService::GetSyncAnyInfos(const string& aServiceName) {
    int now = CThreadTimer::instance()->getNowSecs();
    if (now - 1 >= iDumpSyncAnyInteval) {
        iDumpSyncAnyInteval = now;
        DumpAnySyncInfos(aServiceName, now);
    }
    return iSyncAnyStr;
}

const string& CAgentService::GetConfSetPage() {
    return iConfSetPageStr;
}
// ---------------------------------------------------------------------------
// string CAgentService::GetRespInfos(string aServiceName)
//
// response infos is return.
// ---------------------------------------------------------------------------
//
const string& CAgentService::GetRespInfos(const string& aServiceName,
                                          EMsgType aFlag) {
    switch (aFlag) {
        case kMsgTypeSyncAll:
            return GetSyncAllInfos();
        case kMsgTypeSyncAny:
            return GetSyncAnyInfos(aServiceName);
        case kMsgTypeConf:
            return CSingleton<CSessionSync>::Instance()->GetSyncString();
        case kMsgTypeConfSet:
            return GetConfSetPage();
        case kMsgTypeGetsAny:
            return GetIpPortByService(aServiceName);
        case kMsgTypeGetsAll:
            return GetAllAgentIpPort();
        case kMsgTypeStatAll:
        case kMsgTypeStatAny:
            return GetStatisticInfos(aServiceName);
            break;
        default:
            return GetStatisticInfos(aServiceName);
    }
    return GetStatisticInfos(aServiceName);
}

// ---------------------------------------------------------------------------
// int CAgentService::ConfigInitial()
//
// configuration service is initialized.
// ---------------------------------------------------------------------------
//
int CAgentService::ConfigInitial() {
    const char *funcName = "CAgentService::ConfigInitial()";
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
    int tmpInt = iConfig->GetCfgInt("GLOBAL.LOG_LEVEL", CFG_LOG_LEVEL);
    iConfig->SetConfig(CFG_LOG_LEVEL, tmpInt);
    CSingleton<CLogWriter>::Instance()->SetLogLevel(tmpInt);
    LOG(LL_VARS, "%s:GLOBAL.LOG_LEVEL=[%d]", funcName, tmpInt);

    tmpStr = iConfig->GetCfgStr("GLOBAL.UDP_AGENT_IP", CFG_UDP_BIND_IP);
    iConfig->SetConfig(CFG_UDP_BIND_IP, tmpStr);
    LOG(LL_VARS, "%s:GLOBAL.UDP_BIND_IP=[%s]", funcName, tmpStr.c_str());

    tmpStr = iConfig->GetCfgStr("GLOBAL.TCP_AGENT_IP", CFG_TCP_BIND_IP);
    iConfig->SetConfig(CFG_TCP_BIND_IP, tmpStr);
    LOG(LL_VARS, "%s:GLOBAL.TCP_AGENT_IP=[%s]", funcName, tmpStr.c_str());

    tmpInt = iConfig->GetCfgInt("GLOBAL.UDP_BIND_PORT", CFG_UDP_BIND_PORT);
    iConfig->SetConfig(CFG_UDP_BIND_PORT, tmpInt);
    LOG(LL_VARS, "%s:GLOBAL.UDP_BIND_PORT=[%d]",
        funcName, tmpInt);

    tmpInt = iConfig->GetCfgInt("GLOBAL.TCP_BIND_PORT", CFG_UDP_BIND_PORT);
    iConfig->SetConfig(CFG_TCP_BIND_PORT, tmpInt);
    LOG(LL_VARS, "%s:GLOBAL.TCP_BIND_PORT=[%d]",
        funcName, tmpInt);

    tmpInt = iConfig->GetCfgInt("GLOBAL.SOCKET_TIME_OUT", CFG_SOCK_TIME_OUT);
    iConfig->SetConfig(CFG_SOCK_TIME_OUT, tmpInt);
    LOG(LL_VARS, "%s:GLOBAL.SOCKET_TIME_OUT=[%d]",
        funcName, tmpInt);

    tmpInt = iConfig->GetCfgInt("GLOBAL.SOCKET_RW_TIME_OUT",
        CFG_SOCK_RW_TIME_OUT);
    iConfig->SetConfig(CFG_SOCK_RW_TIME_OUT, tmpInt);
    LOG(LL_VARS, "%s:GLOBAL.SOCKET_RW_TIME_OUT=[%d]", funcName, tmpInt);

    tmpInt = iConfig->GetCfgInt("GLOBAL.SESSION_COUNT", CFG_SESSION_COUNTS);
    iConfig->SetConfig(CFG_SESSION_COUNTS, tmpInt);
    LOG(LL_VARS, "%s:GLOBAL.SESSION_COUNT=[%d]",
        funcName, tmpInt);

    tmpInt = iConfig->GetCfgInt("GLOBAL.ADD_MULTICAST_FLAG",
        CFG_ADD_MULTICAST_FLAG);
    iConfig->SetConfig(CFG_ADD_MULTICAST_FLAG, tmpInt);
    LOG(LL_VARS, "%s:GLOBAL.ADD_MULTICAST_FLAG=[%d]",
        funcName, tmpInt);

    tmpInt = iConfig->GetCfgInt("GLOBAL.STAT_PAGE_REFRESH_FLAG", 0);
    iConfig->SetConfig(CFG_HTTP_PAGE_REFRESH_FLAG, tmpInt);
    LOG(LL_VARS, "%s:GLOBAL.STAT_PAGE_REFRESH_FLAG=[%d]",
        funcName, tmpInt);

    tmpStr = iConfig->GetCfgStr("GLOBAL.HTTP_AUTH", CFG_HTTP_AUTH_STR);
    iConfig->SetConfig(CFG_HTTP_AUTH_STR, tmpStr);
    LOG(LL_VARS, "%s:GLOBAL.HTTP_AUTH=[%s]", funcName, tmpStr.c_str());
    DEBUG(LL_ALL, "%s:End", funcName);
    return RET_OK;
}
// end of local file.
