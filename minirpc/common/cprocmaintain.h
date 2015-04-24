/*
** Copyright (C) 2014 Wang Yaofu
** All rights reserved.
**
**Author:Wang Yaofu voipman@qq.com
**Description: The header file of class CProcMaintain.
*/

#ifndef _COMMON_CPROCMAINTAIN_H_
#define _COMMON_CPROCMAINTAIN_H_
#include <map>
#include <string>
using std::string;
using std::map;

class CProcMaintain {
 public:
    /*
    * constructor.
    *
    */
    CProcMaintain();

    /*
    * Destructor.
    *
    */
    ~CProcMaintain();

    /*
    * reset ipc.
    *
    */
    int IpcReset();

    /*
    * broadcast message or command is sent to childs.
    *
    */
    int IpcBroadcastChilds(const string& aStr);

    /*
    * heart beat message or command is sent to childs.
    *
    */
    int IpcChilds();

    /*
    * heart beat message or command is sent to parent.
    *
    */
    int IpcParent(int aFromPid, long aNowSecs, const string& aStr, string* aOut);

    /*
    * kick out timeout childs.
    *
    */
    int KickTimeoutChilds(int aTimeout = 5);

    // set message queue key.
    inline void SetMaxChilds(int aMaxChilds) {
        iMaxChilds = aMaxChilds;
    }

    // set message queue key.
    inline void SetIpcKey(int aKey) {
        iIpcKey = aKey;
    }
    // get message queue key.
    inline int GetIpcKey() {
        return iIpcKey;
    }
    // how many childs does not running.
    inline int GetNeedCounts() {
        return iMaxChilds - iChildsMap.size();
    }
    // add pid+time.sec
    void AddChilds(int aPid, long aStartSecs);
    // del pid pair.
    void DelChilds(int aPid);
private:
    // IPC key.
    int iIpcKey;
    // childs map. pid,timer.secs
    map<int, long > iChildsMap;
    // max childs.
    int iMaxChilds;
    // history command to new process.
    string iHistoryCmds;
};
#endif  // _COMMON_CPROCMAINTAIN_H_
