/*
** Copyright (C) 2014 Wang Yaofu
** All rights reserved.
**
**Author:Wang Yaofu voipman@qq.com
**Description: The header file of class CStatisticInfos.
*/

#ifndef _COMMOM_CSTATISTICINFO_H_
#define _COMMOM_CSTATISTICINFO_H_
#include <string>
using std::string;
class CStatisticInfos {
public:
    CStatisticInfos() {}
    CStatisticInfos(string aIp, int aPort, int aPid, long aNow, int aFlag)
        : iPort(aPort), iPid(aPid), iSecs(aNow), iFlag(aFlag) {
        iIpAddress = aIp;
    }
    const CStatisticInfos& operator = (const CStatisticInfos &m) {

        this->iIpAddress   = m.iIpAddress;
        this->iPort = m.iPort;
        this->iPid  = m.iPid;
        this->iStartTime  = m.iStartTime;
        this->iStartTimeStr  = m.iStartTimeStr;
        this->iRunningTime  = m.iRunningTime;
        this->iCurConns  = m.iCurConns;
        this->iSecs   = m.iSecs;
        this->iFlag   = m.iFlag;
        this->iServiceType = m.iServiceType;
        this->iLoginName   = m.iLoginName;
        this->iService    = m.iService;
        this->iInputPkgs  = m.iInputPkgs;
        this->iOutputPkgs = m.iOutputPkgs;
        this->iInputPkgTps = m.iInputPkgTps;
        this->iHandledConns = m.iHandledConns;
        this->iVectorInfo = m.iVectorInfo;
        return *this;
    }
    bool operator < (const CStatisticInfos &m) const
    {
        return iCurConns < m.iCurConns;
    }
    bool operator () (const CStatisticInfos &m) const
    {
        if (iFlag == 0) {
            return iIpAddress == m.iIpAddress && iPort == m.iPort && iPid == m.iPid;
        } else if (iFlag == 1) {
            // no heart beat package is from iIpPort before timeout.
            // m.iSecs <= iSecs + timeout.
            return m.iSecs < iSecs;
        }
        return false;
    }
    void MakeStatisticInfos();

    /*
    * make a session package.
    *
    */
    int MakePkg(char *aBuffer, const string& aCmd, const string& aCmdType);

    /*
    * check sum.
    *
    */
    void ChkSum(int len, const char*  buf, char* res);

    /*
    * msg LOGIN {HN}   {UN}      {SN}  {IP}       {port} {PID}  {CC}{HC} {IS) {OS} {ST}         {RT}
    * msg:LOGIN {v890} {wangyf5} {BMS} {10.1.1.1} {8081} {1234} {0} {20} {10} {10} {1380202020} {123}
    */
    // service type
    string iServiceType;
    // login name.
    string iLoginName;
    // service name.
    string iService;
    // local ip address.
    string iIpAddress;
    // local process listen port.
    int iPort;
    // local process id.
    int iPid;
    // current connections.
    long iCurConns;
    // handled connections.
    long long iHandledConns;
    // input packages.
    long long iInputPkgs;
    // input packages.
    long long iLastInputPkgs;
    // input packages tps.
    long long iInputPkgTps;
    // output packages.
    long long iOutputPkgs;
    // start time.
    long iStartTime;
    // running time.
    long iRunningTime;
    //start seconds for package tps.
    long iLastTpsSecs;
    // format string.
    string iFormatStr;
    // wheather monitor log send to agent.
    int iIsUpMonitorInfos;
    // MaxRequestsPerChild 10000
    int iMaxRequestsPerChild;
    // vector information.
    string iVectorInfo;
    // 0 for find_if, 1 for remove_if.
    int iFlag;
    long iSecs;
    string iStartTimeStr;
};
#endif // _COMMOM_CSTATISTICINFO_H_
