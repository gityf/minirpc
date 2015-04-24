/*
** Copyright (C) 2014 Wang Yaofu
** All rights reserved.
**
**Author:Wang Yaofu voipman@qq.com
**Description: The header file of class CStackTrace.
*/

#ifndef _COMMON_DEBUG_STACK_TRACE_H
#define _COMMON_DEBUG_STACK_TRACE_H

#include <iosfwd>
#include <string>

namespace wyf {

class CStackTrace
{
public:
    // ctor and dtor
    CStackTrace();
    ~CStackTrace();

    int ToString(std::string *stack) const;
    int Output(std::ostream* os) const;

private:
    static const int kMaxCallStack = 32;
    void* mTrace[kMaxCallStack];
    int mCount;
};

} // namespace wyf

#endif // _COMMON_DEBUG_STACK_TRACE_H
