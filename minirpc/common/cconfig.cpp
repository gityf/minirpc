/*
** Copyright (C) 2014 Wang Yaofu
** All rights reserved.
**
**Author:Wang Yaofu voipman@qq.com
**Description: The source file of class CConfig.
*/
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "common/cconfig.h"
#include "common/localdef.h"
#include "common/clogwriter.h"

// ---------------------------------------------------------------------------
// CConfig::CConfig()
//
// constructor.
// ---------------------------------------------------------------------------
//
CConfig::CConfig() {
    DEBUG(LL_ALL, "CConfig()::Begin");
    DEBUG(LL_ALL, "CConfig()::End");
}

// ---------------------------------------------------------------------------
// CConfig::~CConfig()
//
// destructor.
// ---------------------------------------------------------------------------
//
CConfig::~CConfig() {
    DEBUG(LL_ALL, "~CConfig()::Begin");
    iIsInitialed = false;
    DEBUG(LL_ALL, "~CConfig()::End");
}

// ---------------------------------------------------------------------------
// int CConfig::InitConfig(struct CConfig aConfigInfo);
//
// get the configuration from ini file xxx.ini
// ---------------------------------------------------------------------------
//
int CConfig::InitConfig(const string& aConfigFile) {
    DEBUG_FUNC_NAME("CConfig::InitConfig");
    DEBUG(LL_ALL, "%s:Begin.", funcName);
    DEBUG(LL_VARS, "%s:config-file:(%s)", funcName, aConfigFile.c_str());
    iConfigFile = aConfigFile;
    iIsInitialed = true;
    DEBUG(LL_ALL, "%s:End.", funcName);
    return RET_OK;
}

const string CConfig::GetConfigFile() const {
    return (iIsInitialed ? iConfigFile : "./syslog.ini");
}

// ---------------------------------------------------------------------------
// string CConfig::GetConfig(string aKey)
//
// get string value by string key.
// ---------------------------------------------------------------------------
//
string CConfig::GetConfig(const string& aKey) {
    DEBUG_FUNC_NAME("CConfig::GetConfig");
    DEBUG(LL_ALL, "%s:Begin.", funcName);
    DEBUG(LL_VARS, "%s:key:(%s)", funcName, aKey.c_str());
    if (iMapStrStr.find(aKey) != iMapStrStr.end()) {
        DEBUG(LL_VARS, "%s:key:(%s),value:(%s)",
            funcName, aKey.c_str(), iMapStrStr[aKey].c_str());
        return iMapStrStr[aKey];
    } else {
        LOG(LL_WARN, "%s:key:(%s),value is null.", funcName, aKey.c_str());
        return string(" ");
    }
}

// ---------------------------------------------------------------------------
// void CConfig::SetConfig(string aKey, string aValue)
//
// set key-value into map_str_str.
// ---------------------------------------------------------------------------
//
void CConfig::SetConfig(const string& aKey, const string& aValue) {
    LOG(LL_VARS, "CConfig::SetConfig:key:(%s),value:(%s)",
        aKey.c_str(), aValue.c_str());
    iMapStrStr[aKey] = aValue;
}

// ---------------------------------------------------------------------------
// int CConfig::GetConfig(int aKey)
//
// get int value by int key.
// ---------------------------------------------------------------------------
//
int CConfig::GetConfig(int aKey) {
    DEBUG_FUNC_NAME("CConfig::GetConfig");
    DEBUG(LL_ALL, "%s:Begin.", funcName);
    DEBUG(LL_VARS, "%s:key:(%d)", funcName, aKey);
    if (iMapIntInt.find(aKey) != iMapIntInt.end()) {
        DEBUG(LL_VARS, "%s:key:(%d),value:(%d)",
            funcName, aKey, iMapIntInt[aKey]);
        return iMapIntInt[aKey];
    } else {
        LOG(LL_WARN, "%s:key:(%d),value is nil -1.", funcName, aKey);
        return RET_ERROR;
    }
}

// ---------------------------------------------------------------------------
// void CConfig::SetConfig(int aKey, int aValue)
//
// set key-value into map_str_int.
// ---------------------------------------------------------------------------
//
void CConfig::SetConfig(int aKey, int aValue) {
    LOG(LL_VARS, "CConfig::SetConfig:key:(%d),value:(%d)",
        aKey, aValue);
    iMapIntInt[aKey] = aValue;
}

// ---------------------------------------------------------------------------
// int CConfig::IsBlankLine(char *aLine)
//
// this is a extern method to check blank line in configure file xxx.ini.
// ---------------------------------------------------------------------------
//
int CConfig::IsBlankLine(const char* aLine) {
    for(int i=0; aLine[i] && aLine[i]!='\r' && aLine[i]!='\n'; i++)
        if(aLine[i] != ' ' && aLine[i] != '\t')
            return 0;
    return 1;
}

// ---------------------------------------------------------------------------
// void CConfig::TrimSpace(char *aDest, const char* aSource)
//
// the space in string of source will be trimmed and the result will be
// saved into dest.
// ---------------------------------------------------------------------------
//
void CConfig::TrimSpace(char *aDest, const char* aSource) {
    int i=0;
    if(!aSource) return;
    if(aSource[0] == 0) {
        aDest[0] = 0;
        return;
    }
    while(aSource[i++] == ' ') ;
    strcpy(aDest, aSource+i-1);
    int len = strlen(aDest);
    if(len == 0) return;
    while(aDest[--len] == ' ') ;
    aDest[len+1] = 0;
}

// ---------------------------------------------------------------------------
// int CConfig::MatchSection(const char* aStr, const char* aSection)
//
// match the section in symbol '[]'.
// ---------------------------------------------------------------------------
//
int CConfig::MatchSection(const char* aStr, const char* aSection) {
    int p = 0;
    char s1[256];

    if(aStr[0] != '[')
        return 0;

    for(int i=1; i<255; i++) {
        if(aStr[i] == ']') break;
        s1[p++] = aStr[i];
    }
    s1[p] = 0;
    return strcmp(s1, aSection) ? 0 : 1;
}

// ---------------------------------------------------------------------------
// int CConfig::MatchKey(const char* aStr, const char* aKey)
//
// match the key in symbol '[]'.
// ---------------------------------------------------------------------------
//
int CConfig::MatchKey(const char* aStr, const char* aKey) {
    int p = 0;
    char s1[256];
    char s2[256];

    if(aStr[0] == '[') return 0;

    for(int i=0; i<254; i++) {
        if(aStr[i] == '=') break;
        s1[p++] = aStr[i];
    }

    s1[p] = 0;
    TrimSpace(s2, s1);
    return strcmp(s2, aKey) ? 0 : 1;
}

// ---------------------------------------------------------------------------
// int CConfig::ReadAllConfig
//
// get all the configs from file xxx.ini.
// ---------------------------------------------------------------------------
//
int CConfig::ReadAllConfig() {
    FILE *fp = fopen(iConfigFile.c_str(), "r");

    //cannot open file
    if(fp == NULL)
        return RET_ERROR;
    char buf[1024] = {0};
    char bufNew[1024] = {0};
    string section;
    int flagSection = 0, ii = 0;
    while(fgets(buf, 1023, fp)) {
        if(buf[0] == ';' || buf[0] == '#')
            continue;

        int len = strlen(buf);
        for(int i=0; i<len; i++) {
            if(buf[i] == 0x0A || buf[i] == 0x0D) {
                buf[i] = 0;
            }
        }

        if(buf[0] == '[') {
            flagSection = 0;
            ii = 0;
            while(ii < 1023 && buf[ii] != ']') {
                ii++;
            }
            if (ii >= 1023 || ii == 1) {
                continue;
            }
            buf[ii] = 0x00;
            section = buf+1;
            flagSection = 1;
        } else if(flagSection) {
            // get key
            int keyLen = strlen(buf);
            for(ii=0; ii<keyLen; ii++) {
                if(buf[ii] == '=') {
                    buf[ii] = 0x00;
                    TrimSpace(bufNew, buf);
                    string ItemKey = section;
                    ItemKey += ".";
                    ItemKey += bufNew;
                    TrimSpace(bufNew, buf+ii+1);
                    SetConfig(ItemKey, bufNew);
                }
            }
        }
    }

    fclose(fp);
    return RET_OK;
}

// ---------------------------------------------------------------------------
// void CConfig::DumpAllConfig
//
// dump all the config.
// ---------------------------------------------------------------------------
//
void CConfig::DumpAllConfig() {
    typeof(iMapStrStr.end()) it = iMapStrStr.begin();
    while(it != iMapStrStr.end()) {
        LOG(LL_INFO, "CConfig::DumpAllConfig:%s=%s",
            it->first.c_str(), it->second.c_str());
        ++it;
    }
}

// ---------------------------------------------------------------------------
// int CConfig::GetPrivateProfileInt
//
// get the integer of key from file xxx.ini.
// ---------------------------------------------------------------------------
//
int CConfig::GetPrivateProfileInt(const char* aSection,
                                  const char* aKey,
                                  int aDefault) {
    FILE *fp = fopen(iConfigFile.c_str(), "r");

    //cannot open file
    if(fp == NULL)
        return aDefault;

    char buf[512] = {0};
    int flagSection = 0;
    while(fgets(buf, 511, fp)) {
        if(buf[0] == ';' || buf[0] == '#')
            continue;

        int len = strlen(buf);
        for(int i=0; i<len; i++)
            if(buf[i] == 0x0A || buf[i] == 0x0D)
                buf[i] = 0;

        if(buf[0] == '[') {
            if(flagSection) {
                fclose(fp);
                //key not found when section finished
                return aDefault;
            }

            if(MatchSection(buf, aSection))
                flagSection = 1;
        } else if(flagSection && MatchKey(buf, aKey)) {
            //find the key
            for(int i=strlen(aKey); i<strlen(buf); i++) {
                if(buf[i] == '=') {
                    fclose(fp);
                    int result = atoi(buf+i+1);
                    if(result < 0)
                        return 0;
                    else
                        return result;
                }
                else if(buf[i] == 0)
                    break; //for
            }
        }
    }

    fclose(fp);
    //key is not found
    return aDefault;

}

// ---------------------------------------------------------------------------
// char* CConfig::Mystrncpy
//
// the source string will be copied into string dest.
// ---------------------------------------------------------------------------
//
char* CConfig::Mystrncpy(char *aDest, char *aSource, int aCount) {
    //return the count
    int size = strlen(aSource);
    if(size >= aCount) {
        size = aCount-1;
        memcpy(aDest, aSource, size);
        aDest[size] = 0;
    } else {
        strcpy(aDest, aSource);
    }
    return aDest;
}

// ---------------------------------------------------------------------------
// char* CConfig::GetPrivateProfileString
//
// get value of key from file xxx.ini
// ---------------------------------------------------------------------------
//
char* CConfig::GetPrivateProfileString(const char* aSection,
                                       const char* aKey,
                                       const char* aSDefault,
                                       char *aOutString,
                                       int aSize) {
    char outDefault[MAX_SIZE_2K];
    memset(aOutString, 0, MAX_SIZE_2K);
    TrimSpace(outDefault, aSDefault);

    FILE *fp = fopen(iConfigFile.c_str(), "r");

    //cannot open file
    if(fp == NULL)
        return Mystrncpy(aOutString, outDefault, aSize);

    char buf[1024+256];
    int flagSection = 0;
    while(fgets(buf, sizeof(buf), fp)) {
        if(buf[0] == ';' || buf[0] == '#')
            continue;

        int len = strlen(buf);
        for(int i=0; i<len; i++)
            if(buf[i] == 0x0A || buf[i] == 0x0D)
                buf[i] = 0;

        if(buf[0] == '[') {
            if(flagSection) {
                fclose(fp);
                return Mystrncpy(aOutString, outDefault, aSize);
            }

            if(MatchSection(buf, aSection))
                flagSection = 1;
        } else if(flagSection && MatchKey(buf, aKey)) {
            //find the key
            for(int i=strlen(aKey); i<strlen(buf); i++) {
                if(buf[i] == '=') {
                    fclose(fp);
                    TrimSpace(outDefault, buf+i+1);
                    return Mystrncpy(aOutString, outDefault, aSize);
                } else if(buf[i] == 0) {
                    break; //for
                }
            }
        }
    }

    fclose(fp);
    return Mystrncpy(aOutString, outDefault, aSize);
}

// ---------------------------------------------------------------------------
// int CConfig::WritePrivateProfileString
//
// write string for key into file xxx.ini.
// ---------------------------------------------------------------------------
//
int CConfig::WritePrivateProfileString(const char* aAppName,
                                       const char* aKeyName,
                                       const char* aString,
                                       const char* aFileName) {
    FILE *fp = fopen(aFileName, "r+");
    //cannot open file
    if(fp == NULL) {

        //file is not exist
        fp = fopen(aFileName, "w");
        if(fp == NULL) return 0;
        fprintf(fp, "[%s]\n", aAppName);
        fprintf(fp, "%s=%s\n", aKeyName, aString);
        fclose(fp);
        return 1;
    }

    string tmpFileName = aFileName;
    tmpFileName += ".tmp";
    FILE *fpTmp = fopen((const char*)tmpFileName.c_str(), "w");
    if(fpTmp == NULL) {
        fclose(fp);
        return 0;
    }

    char buf[256];
    int buflen;
    // the flag of section is found
    int flagSection = 0;
    int flagKey = 0;
    long currentPos = 0;
    long insertPos = 0;

    //locate insert position
    while(fgets(buf, sizeof(buf), fp)) {
        buflen = strlen(buf);
        currentPos += buflen;

        // the line is comment if it begin with ';'.
        if(buf[0] == ';' || buf[0] == '#')
            continue;


        if(buf[0] == '[') {
            if(flagSection) {
                // section without key is found.
                break;
            }

            if (MatchSection(buf, aAppName))
                flagSection = 1;
        } else if (flagSection && MatchKey(buf, aKeyName)) {
            //find the key
            flagKey = 1;
            if(!IsBlankLine(buf))
                insertPos = currentPos;
            break;
        }

        if(!IsBlankLine(buf))
            insertPos = currentPos;
    }

    //write output file
    if(!flagSection) {
        // section is not found
        fprintf(fp, "[%s]\n", aAppName);
        fprintf(fp, "%s=%s\n", aKeyName, aString);
        fclose(fp);
        fclose(fpTmp);
        //unlink((const char*)tmpFileName);
        remove(tmpFileName.c_str());
    } else {
        fseek(fp, 0, SEEK_SET);
        currentPos = 0;
        while(fgets(buf, sizeof(buf), fp)) {
            buflen = strlen(buf);
            currentPos += buflen;

            if(currentPos == insertPos) {
                if(!flagKey) {
                    fputs(buf, fpTmp);
                    fprintf(fpTmp, "%s=%s\n", aKeyName, aString);
                } else {
                    fprintf(fpTmp, "%s=%s\n", aKeyName, aString);
                }
            } else {
                fputs(buf, fpTmp);
            }
        }
        fclose(fp);
        fclose(fpTmp);

        //unlink(aFileName);
        remove(aFileName);
        rename(tmpFileName.c_str(), aFileName);
    }

    return 1;
}
// end of local file.
