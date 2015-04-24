/*
** Copyright (C) 2014 Wang Yaofu
** All rights reserved.
**
**Author:Wang Yaofu voipman@qq.com
**Description: The source file of class CProcess.
*/

#include <unistd.h>
#include <csignal>
#include "common/cprocess.h"
#include "common/clogwriter.h"
CProcess::CProcess() {
}

CProcess::~CProcess() {
}

static void ProcessSigAlarm(int aSinid) {
    return;
}

string CProcess::RunShell(const string& aShell, const string& aResFile, int aTimeout) {
    const char *funcName = "CProcess::RunShell()";
    DEBUG(LL_ALL, "%s:Begin.", funcName);
    LOG(LL_VARS, "%s:shell cmd=(%s)", funcName, aShell.c_str());

    FILE *pFile = popen(aShell.c_str(), "r");
    string resultStr = "";
    bool isSysCall = false;
    if (pFile != NULL) {
        LOG(LL_NOTICE, "%s:in mode 1.", funcName);
    } else {
        resultStr = aShell;
        resultStr += " > ";
        resultStr += aResFile;
        system(resultStr.c_str());
        pFile = fopen(aResFile.c_str(), "r");
        if (pFile != NULL) {
            isSysCall = true;
            LOG(LL_NOTICE, "%s:in mode 2.", funcName);
        }
    }
    resultStr = "";
    if (pFile != NULL) {
        char resbuf[1024] = {0};
        signal(SIGALRM, ProcessSigAlarm);
        alarm(aTimeout);
        while(fgets(resbuf, sizeof(resbuf), pFile) != NULL) {
            resultStr += resbuf;
        }
        alarm(0);
        if (isSysCall) {
            fclose(pFile);
            unlink(aResFile.c_str());
            remove(aResFile.c_str());
        } else {
            pclose(pFile);
        }
    }    
    DEBUG(LL_ALL, "%s:End.", funcName);
    return resultStr;
}
