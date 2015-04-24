/*
** Copyright (C) 2014 Wang Yaofu
** All rights reserved.
**
**Author:Wang Yaofu voipman@qq.com
**Description: The header file of class CPeerFilter.
*/

#ifndef _COMMON_CPEERFILTER_H_
#define _COMMON_CPEERFILTER_H_
#include <algorithm>
#include <functional>
#include <map>
#include <string>
using std::string;
enum EFilterOption {
    kPeerAddrAllow = 0,
    kPeerAddrDeny
} ;
// ---------------------------------------------------------------------------
// class CPeerFilter
//
// this class is a peer host filter class.
//
// ---------------------------------------------------------------------------
//
class CPeerFilter {
 public:
    /*
    * peer ip is checked.
    * allow return 0, deny return 1.
    */
    EFilterOption PeerAddrCheck(const string& aPeerAddr) {
        if (iPeerMap.find(aPeerAddr) != iPeerMap.end()) {
            return iPeerMap[aPeerAddr];
        } else {
            return kPeerAddrDeny;
        }
    }

    /*
    * set key-value into map_str_int.
    * aValue = 0 allow, aValue = 1 deny.
    */
    void SetPeerAddr(const string& aKey, EFilterOption aValue) {
        iPeerMap[aKey] = aValue;
    }

    // clear peer map
    void ClearPeerAddrs() {
        iPeerMap.clear();
    }

    // erase peer address from map
    void EraseByAddrs(const string& aKey) {
        iPeerMap.erase(aKey);
    }

    // clear deny or allow peer address from map
    void EraseByValue(EFilterOption aValue) {
        for (typeof(iPeerMap.end()) it = iPeerMap.begin();
            it != iPeerMap.end(); ++it) {
            if (it->second == aValue) {
                typeof(it) tmpit = it;
                ++tmpit;
                iPeerMap.erase(it);
                it = tmpit;
            }
        }
    }

    /*
    * constructor.
    *
    */
    explicit CPeerFilter() {
        iPeerMap.clear();
    }

    /*
    * Destructor.
    *
    */
    virtual ~CPeerFilter() {
        iPeerMap.clear();
    }

 private:
    /*
    * configuration list for peer addr checked string key, int value.
    *
    */
    std::map<string, EFilterOption > iPeerMap;
};
#endif  // _COMMON_CPEERFILTER_H_
