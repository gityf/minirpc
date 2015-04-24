/*
** Copyright (C) 2014 Wang Yaofu
** All rights reserved.
**
**Author:Wang Yaofu voipman@qq.com
**Description: The header file of class CProcess.
*/

#ifndef _COMMON_CPROCESS_H_
#define _COMMON_CPROCESS_H_

#include <string>
using std::string;

class CProcess {
 public:
    /*
    * constructor.
    *
    */
    CProcess();

    /*
    * Destructor.
    *
    */
    ~CProcess();

    /*
    * run shell command.
    *
    */
    string RunShell(const string& aShell, const string& aResFile, int aTimeout = 5);
private:
};
#endif  // _COMMON_CPROCESS_H_
