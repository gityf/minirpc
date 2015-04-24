/*
** Copyright (C) 2014 Wang Yaofu
** All rights reserved.
**
**Author:Wang Yaofu voipman@qq.com
**Description: The header file of class CLogWriter.
*/
#ifndef _COMMON_CLOGWRITER_H_
#define _COMMON_CLOGWRITER_H_
#include <iostream>
#include <fstream>
#include <string>
#include "common/cthread.h"
#include "common/csingleton.h"
using std::fstream;
using std::ios;
using std::string;
// max log size is 1G
#define MAX_LOG_SIZE 1073741824L
// max file name path
#define MAX_FILE_PATH_SIZE 256
/*
* log level comments.
* LL_ERROR: error
* LL_WARN:  warning
* LL_NOTICE:  notice
* LL_INFO:  important information
* LL_VARS:  variable info
* LL_DBG:  debug info
* LL_ALL:   all info, lowest level
*/
enum ELogLevel {
    LL_NONE = 0,
    LL_ERROR = 1,
    LL_WARN,
    LL_NOTICE,
    LL_INFO,
    LL_VARS,
    LL_DBG,
    LL_ALL
};

struct CLogConfig {
    string iLogFileName;
    ELogLevel iLogLevel;
    unsigned int iLogOption;
    CLogConfig() {
        iLogLevel = LL_WARN;
        iLogFileName = "log";
        iLogOption = 1;
    }
};
class CLogWriter
{
public:
    /*
    * init data for writer.
    *
    */
    bool Init(const struct CLogConfig& aLogConfig);

    /*
    * log level is set.
    *
    */
    void SetLogLevel(const int aLevel);

    /*
    * log option is set.
    *
    */
    void SetLogOption(const unsigned int aOption);

    /*
    * log option is cleared.
    *
    */
    void ClearLogOption(const unsigned int aOption);

    /*
    * log file name is set.
    *
    */
    void SetLogFileName(const string& aLogFileName);

    /*
    * log to file by va_list format.
    *
    */
    bool Log(ELogLevel aLogLevel, const char* aFormat, ...);

    /*
    * log to file by raw.
    *
    */
    bool LogRaw(ELogLevel aLogLevel, const char* aMsgRaw);

    /*
    * log buffer with len.
    *
    */
    bool LogHexWithLen(ELogLevel aLogLevel, const char* aMsgRaw, int aLen);

    /*
    * log buffer will be loged by hex and ASCII format.
    *
    */
    void LogHexAsc(ELogLevel aLogLevel, const void *aBuf, int aBufLen);

    /*
    * constructor.
    *
    */
    CLogWriter();

    /*
    * destructor.
    *
    */
    ~CLogWriter();

private:
    /*
    * local file name is generated.
    *
    */
    void GenLogFileName();

    /*
    * init log file and open log file.
    *
    */
    bool InitLogFile();

    /*
    * check whether log file exists.
    *
    */
    bool CheckLogFile();

    /*
    * check whether writable of log file.
    *
    */
    bool CheckWritable();

    /*
    * the mutex for local class.
    *
    */
    CMutex iMutex;

    /*
    * log file stream.
    *
    */
    fstream iOutPutPtrace;

    /*
    * index of the local thread.
    *
    */
    int iIndex;

    /*
    * the day for iIndex reset.
    *
    */
    int iDay;

    /*
    * pid off process.
    *
    */
    int iProcessId;

    /*
    * host name.
    *
    */
    char iHostName[MAX_FILE_PATH_SIZE];

    /*
    * local log file name.
    *
    */
    char iLocalLogFileName[MAX_FILE_PATH_SIZE];

    /*
    * the configuration of local class.
    *
    */
    struct CLogConfig iLogConfig;
};
#ifdef DEBUG_LOG
#define LOG         CSingleton<CLogWriter>::Instance()->Log
#define LOG_RAW     CSingleton<CLogWriter>::Instance()->LogRaw
#define LOG_HEX_LEN CSingleton<CLogWriter>::Instance()->LogHexWithLen
#define LOG_HEX_ASC CSingleton<CLogWriter>::Instance()->LogHexAsc
#else
#define LOG //
#define LOG_RAW //
#define LOG_HEX_LEN //
#define LOG_HEX_ASC //
#endif

#ifdef _DEBUG
#define DEBUG         CSingleton<CLogWriter>::Instance()->Log
#define DEBUG_RAW     CSingleton<CLogWriter>::Instance()->LogRaw
#define DEBUG_HEX_LEN CSingleton<CLogWriter>::Instance()->LogHexWithLen
#define DEBUG_HEX_ASC CSingleton<CLogWriter>::Instance()->LogHexAsc
#else
#define DEBUG //
#define DEBUG_RAW //
#define DEBUG_HEX_LEN //
#define DEBUG_HEX_ASC //
#endif

#define DEBUG_FUNC_NAME(name) \
    const char* funcName = name;

#endif // _COMMON_CLOGWRITER_H_
