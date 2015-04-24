/*
** Copyright (C) 2014 Wang Yaofu
** All rights reserved.
**
**Author:Wang Yaofu voipman@qq.com
**Description: This is the header file of colored console UI helper.
*/
#ifndef _COMMON_COLORPRINT_H_
#define _COMMON_COLORPRINT_H_

#include <iostream>

namespace wyf
{
    inline std::ostream& COLOR_BLACK(std::ostream &s) {
        s << "\033[0;30m";
        return s;
    }

    inline std::ostream& COLOR_RED(std::ostream &s) {
        s << "\033[0;31m";
        return s;
    }

    inline std::ostream& COLOR_GREEN(std::ostream &s) {
        s << "\033[0;32m";
        return s;
    }

    inline std::ostream& COLOR_YELLOW(std::ostream &s) {
        s << "\033[0;33m";
        return s;
    }

    inline std::ostream& COLOR_BLUE(std::ostream &s) {
        s << "\033[0;34m";
        return s;
    }

    inline std::ostream& COLOR_PURPLE(std::ostream &s) {
        s << "\033[0;35m";
        return s;
    }

    inline std::ostream& COLOR_CYAN(std::ostream &s) {
        s << "\033[0;36m";
        return s;
    }

    inline std::ostream& COLOR_WHITE(std::ostream &s) {
        s << "\033[0;37m";
        return s;
    }

    inline std::ostream& COLOR_RESTORE(std::ostream &s) {
        s << "\033[0m";
        return s;
    }
} 

#endif  // _COMMON_COLORPRINT_H_
