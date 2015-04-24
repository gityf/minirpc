/*
** Copyright (C) 2014 Wang Yaofu
** All rights reserved.
**
**Author:Wang Yaofu voipman@qq.com
**Description: The header file of class CUncopyable.
*/

#ifndef _COMMON_UNCOPYABLE_H
#define _COMMON_UNCOPYABLE_H

class CUncopyable {
protected:
    CUncopyable() {}
    ~CUncopyable() {}

private:
    CUncopyable(const CUncopyable& o);
    CUncopyable& operator=(const CUncopyable& o);
};

#endif // _COMMON_UNCOPYABLE_H
