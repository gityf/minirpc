/*
** Copyright (C) 2014 Wang Yaofu
** All rights reserved.
**
**Author:Wang Yaofu voipman@qq.com
**Description: The header file of class CConfig.
*/

#ifndef _COMMON_CCFG_H_
#define _COMMON_CCFG_H_
#include <algorithm>
#include <functional>
#include <map>
#include <string>
using std::less;
using std::string;
// multicast address is used for udp session.
#define CFG_UDP_BIND_IP         "224.0.1.1"
//whether agent is added into multicast membership.
#define CFG_ADD_MULTICAST_FLAG  1
#define CFG_CMD_WARN_RUNTIME    2
#define CFG_LOG_LEVEL           3
#define CFG_CMD_GIVE_UP_RUNTIME 4
#define CFG_SESSION_COUNTS      5
#define CFG_SOCK_RW_TIME_OUT    7
#define CFG_COUNT_HANDLER   10
#define CFG_MAX_REQU_PER_CHILD  10000
// decimal or hex when interface receiving mode is RECV_MODE_BY_LEN_VALUE.
#define CFG_RECV_LV_TYPE        16
#define CFG_SOCK_TIME_OUT       60
#define CFG_UDP_BIND_PORT       15218
// tcp address is used for getting session by http proto.
#define CFG_TCP_BIND_IP         "127.0.0.1"
#define CFG_TCP_BIND_PORT       15219
// tcl encoding
#define CFG_TCL_ENCODING        "encoding"
// ---------------------------------------------------------------------------
// class CConfig
//
// this class is a configuration class,
// the configurations will be got from xxx.ini.
// ---------------------------------------------------------------------------
//
class CConfig {
 public:
    /*
    * get the configuration from ini file xxx.ini
    *
    */
    int InitConfig(const string& aConfigFile);

    /*
    * get the configuration ini file name xxx.ini
    *
    */
    const string GetConfigFile() const;

    /*
    * get string value by string key.
    *
    */
    string GetConfig(const string& aKey);
    // operator [string]
    string operator[](const string& aKey) {
        return GetConfig(aKey); 
    }

    /*
    * set key-value into map_str_str.
    *
    */
    void SetConfig(const string& aKey, const string& aValue);

    /*
    * get int value by int key.
    *
    */
    int GetConfig(int aKey);
    // operator [char *]
    int operator[](int aKey) {
        return GetConfig(aKey); 
    }

    /*
    * set key-value into map_str_int.
    *
    */
    void SetConfig(int aKey, int aValue);

    /*
    * get all the configs from file xxx.ini.
    *
    */
    int ReadAllConfig();

    /*
    * dump all config from ini file.
    *
    */
    void DumpAllConfig();

    /*
    * get the integer of key from map.
    *
    */
    int GetCfgInt(const string& aSectionKey, int aDefault) {
        if (iMapStrStr.find(aSectionKey) != iMapStrStr.end()) {
            int ret = atoi(iMapStrStr[aSectionKey].c_str());
            return (ret < 0 ? aDefault : ret);
        }
        return aDefault;
    }

    /*
    * get the integer of key from map.
    *
    */
    int GetCfgInt(const char* aSectionKey, int aDefault) {
        if (iMapStrStr.find(aSectionKey) != iMapStrStr.end()) {
            int ret = atoi(iMapStrStr[aSectionKey].c_str());
            return (ret < 0 ? aDefault : ret);
        }
        return aDefault;
    }

    /*
    * get string value of key from map.
    *
    */
    string GetCfgStr(const string& aSectionKey, const string& aSDefault) {
        if (iMapStrStr.find(aSectionKey) != iMapStrStr.end()) {
           return iMapStrStr[aSectionKey];
        }
        return aSDefault;
    }

    /*
    * get string value of key from map.
    *
    */
    string GetCfgStr(const char* aSectionKey, const char* aSDefault) {
        if (iMapStrStr.find(aSectionKey) != iMapStrStr.end()) {
            return iMapStrStr[aSectionKey];
        }
        return aSDefault;
    }

    /*
    * get the integer of key from file xxx.ini.
    *
    */
    int GetPrivateProfileInt(const char* aSection,
                             const char* aKey,
                             int aDefault);

    /*
    * get value of key from file xxx.ini
    *
    */
    char* GetPrivateProfileString(const char* aSection,
                                  const char* aKey,
                                  const char* aSDefault,
                                  char *aOutString,
                                  int aNSize);

    /*
    * write string for key into file xxx.ini.
    *
    */
    int WritePrivateProfileString(const char* aAppName,
                                  const char* aKeyName,
                                  const char* aString,
                                  const char* aFileName);

    /*
    * constructor.
    *
    */
    CConfig();

    /*
    * Destructor.
    *
    */
    ~CConfig();
private:
    /*
    * this is a extern method to check blank line in configure file xxx.ini.
    *
    */
    int IsBlankLine(const char* aLine);

    /*
    * the space in string of source will be trimmed
    * and the result will be saved into dest.
    */
    void TrimSpace(char* aDest, const char* aSource);

    /*
    * match the section in symbol '[]'.
    *
    */
    int  MatchSection(const char* aStr, const char* aSection);

    /*
    * match the key in symbol '[]'.
    *
    */
    int  MatchKey(const char* aStr, const char* aKey);
    /*
    * the source string will be copied into string dest.
    *
    */
    char * Mystrncpy(char *aDest, char *aSource, int aCount);

 private:
    /*
    * flag to check whether local class is initialed.
    *
    */
    bool iIsInitialed;

    /*
    * configuration list for string key,string value.
    *
    */
    std::map<string, string, less<string> > iMapStrStr;

    /*
    * configuration list for int key,int value.
    *
    */
    std::map<int, int, less<int> > iMapIntInt;

    /*
    * configuration file name.
    *
    */
    string iConfigFile;
};
#endif  // _COMMON_CCFG_H_
