/*
** Copyright (C) 2014 Wang Yaofu
** All rights reserved.
**
**Author:Wang Yaofu voipman@qq.com
**Description: The source file of class CStatisticInfos.
*/
#include <stdio.h>
#include <string.h>
#include "common/cstatisticinfo.h"
#include "common/localdef.h"

// ---------------------------------------------------------------------------
// making statistic Informations.
// ---------------------------------------------------------------------------
//
void CStatisticInfos::MakeStatisticInfos() {
    // LOGIN {HostName} {UserName} {ServiceName}
    // {IP} {port} {PID}
    // {CurrectConns} {HandledConns} {InputPks)
    // {OutputPks} {StartTime} {RunTime}
    long timeInterval = iRunningTime - iLastTpsSecs;
    iLastTpsSecs = iRunningTime;
    iRunningTime -= iStartTime;
    iInputPkgTps = timeInterval > 0 ?
        (iInputPkgs-iLastInputPkgs) / timeInterval : 0;
    iLastInputPkgs = iInputPkgs;
    char tmpBuff[1024] = {0};
    snprintf(tmpBuff, sizeof(tmpBuff), "LOGIN {%s} {%s} {%s} "
        "{%s} {%d} {%d} {%lu} {%lld} {%lld} {%lld} {%lld} {%lu} {%lu} {%s}",
        iServiceType.c_str(),
        iLoginName.c_str(),
        iService.c_str(),
        iIpAddress.c_str(),
        iPort,
        iPid,
        iCurConns,
        iHandledConns,
        iInputPkgs,
        iOutputPkgs,
        iInputPkgTps,
        iStartTime,
        iRunningTime,
        iVectorInfo.c_str());
    iFormatStr = tmpBuff;
}

// ---------------------------------------------------------------------------
// make a session package.
// ---------------------------------------------------------------------------
//
int CStatisticInfos::MakePkg(char *aBuffer, const string& aCmd, const string& aCmdType) {
    if (aBuffer == NULL) {
        return RET_ERROR;
    }
    // warning:: do not call any members of CStatisticInfos in local function.
    int chkSumLen = aCmd.length()+PKG_HEADFLAG_LEN+PKG_CMDLEN_LEN+PKG_CMDLEN_LEN;
    sprintf(aBuffer, "%s%04x%s%s",
        PKG_UDP_HEAD_FLAG, aCmd.length()+PKG_CMDLEN_LEN, aCmdType.c_str(), aCmd.c_str());
    ChkSum(aCmd.length()+PKG_CMDLEN_LEN, aBuffer+PKG_HEADFLAG_LEN+PKG_CMDLEN_LEN, aBuffer+chkSumLen);
    aBuffer[chkSumLen+PKG_CHKSUM_LEN] = 0x00;
    return RET_OK;
}


// ---------------------------------------------------------------------------
// check sum.
// ---------------------------------------------------------------------------
//
void CStatisticInfos::ChkSum(int len, const char* buf, char* res) {
    // warning:: do not call any members of CStatisticInfos in local function.
    int i = 0;
    memset(res, 0, 8);
    for(i=0; i<len; i++) {
        res[i%4]^=(buf+i)[0];
    }
    res[0]=~res[0];
    res[1]=~res[1];
    res[2]=~res[2];
    res[3]=~res[3];

    // 0xE8--->"E8"
    for(i = 7; i >= 0; --i) {
        if (i % 2)  {
            res[i] = (res[i/2] & 0x0F) + '0';
            if (res[i] > '9') {
                res[i] = res[i] + 'A' - '0' - 10;
            }
        } else {
            res[i] = ((res[i/2] >> 4) & 0x0F) + '0';
            if (res[i] > '9') {
                res[i] = res[i] + 'A' - '0' - 10;
            }
        }
    }
}
// end of local file.
