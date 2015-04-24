/*
** Copyright (C) 2014 Wang Yaofu
** All rights reserved.
**
**Author:Wang Yaofu voipman@qq.com
**Description: The source file of class CDateTime.
*/

#include <cstdio>
#include <cstring>
#include <climits>
#include "common/cdatetime.h"

using namespace std;

namespace wyf {

    // construct by local time now.
    void CDateTime::set() {
        _time = ::time(0);
        localtime_r(&_time, &_tm);
    }

    // in param: time_t
    void CDateTime::set(const time_t &tt) {
        time_t _tt = tt;
        if (tt < 0) _tt = 0;
        if (tt > LONG_MAX) _tt = LONG_MAX;

        _time = _tt;
        localtime_r(&_time, &_tm);
    }

    // year
    // mon 
    // mday
    // hour deflaut:0
    // min  deflaut:0
    // sec  deflaut:0
    void CDateTime::set(const int year, const int mon, const int mday,
        const int hour, const int min, const int sec) {
        int _year = year;
        int _mon = mon;
        int _mday = mday;
        int _hour = hour;
        int _min = min;
        int _sec = sec;

        // confirm
        if (_year<1 || _year>2038)    _year = 1970;
        if (_mon<1  || _mon>12)       _mon  = 1;
        if (_mday<1 || _mday>31)      _mday = 1;
        if (_hour<0 || _hour>23)      _hour = 0;
        if (_min<0  || _min>59)       _min  = 0;
        if (_sec<0  || _sec>59)       _sec  = 0;

        _tm.tm_year = _year-1900;
        _tm.tm_mon = _mon-1;
        _tm.tm_mday = _mday;
        _tm.tm_hour = _hour;
        _tm.tm_min = _min;
        _tm.tm_sec = _sec;
        _tm.tm_isdst = -1;
        _time = mktime(&_tm);
    }

    // in param: tm
    void CDateTime::set(const tm &st) {
        this->set(st.tm_year+1900, st.tm_mon+1, st.tm_mday,
            st.tm_hour, st.tm_min, st.tm_sec);
    }

    // in param: CDateTime
    void CDateTime::set(const CDateTime &date) {
        this->set(date.value());
    }

    // in format: "YYYY-MM-DD HH:MM:SS"
    void CDateTime::set(const string &datetime, const string &datemark,
        const string &dtmark, const string &timemark) {
        // init struct tm
        struct tm tm;
        tm.tm_isdst = -1;

        // init format
        string fmt;
        if (datetime.find(dtmark) != datetime.npos)
            fmt = "%Y" + datemark + "%m" + datemark + "%d" + dtmark +
            "%H" + timemark + "%M" + timemark + "%S";
        else
            fmt = "%Y" + datemark + "%m" + datemark + "%d";

        // invoke strptime()
        if (strptime(datetime.c_str(),fmt.c_str(),&tm) != NULL)
            this->set(tm);
        else
            this->set();
    }

    // return format YYYY-MM-DD
    string CDateTime::date(const string &datemark, const bool leadingzero) const {
        char date_str[32];
        if (leadingzero)
            snprintf(date_str, 32, "%04d%s%02d%s%02d",
            this->year(), datemark.c_str(), this->month(), datemark.c_str(), this->m_day());
        else
            snprintf(date_str, 32, "%d%s%d%s%d",
            this->year(), datemark.c_str(), this->month(), datemark.c_str(), this->m_day());

        return string(date_str);
    }

    // return format HH:MM:SS
    string CDateTime::time(const string &timemark, const bool leadingzero) const {
        char time_str[32];
        if (leadingzero)
            snprintf(time_str, 32, "%02d%s%02d%s%02d",
            this->hour(), timemark.c_str(), this->min(), timemark.c_str(), this->sec());
        else
            snprintf(time_str, 32, "%d%s%d%s%d",
            this->hour(), timemark.c_str(), this->min(), timemark.c_str(), this->sec());

        return string(time_str);
    }

    // return format YYYY-MM-DD HH:MM:SS
    string CDateTime::datetime(const string &datemark, const string &dtmark,
        const string &timemark, const bool leadingzero) const
    {
        string datetime = this->date(datemark,leadingzero) + dtmark +
            this->time(timemark,leadingzero);
        return datetime;
    }

    // return GMT format time string.
    string CDateTime::gmt_datetime() const {
        char gmt[50];
        struct tm gmt_tm;

        gmtime_r (&_time, &gmt_tm);
        strftime(gmt, 50, "%A,%d-%B-%Y %H:%M:%S GMT", &gmt_tm);
        return string(gmt);
    }

    CDateTime& CDateTime::operator=(const CDateTime &date) {
        if (this == &date) return *this;
        this->set(date);
        return *this;
    }
    CDateTime& CDateTime::operator=(const time_t &tt) {
        this->set(tt);
        return *this;
    }

    CDateTime& CDateTime::operator+=(const CDateTime &date) {
        this->set(value() + date.value());
        return *this;
    }
    CDateTime& CDateTime::operator+=(const time_t &tt) {
        this->set(value() + tt);
        return *this;
    }

    CDateTime& CDateTime::operator-=(const CDateTime &date) {
        this->set(value() - date.value());
        return *this;
    }
    CDateTime& CDateTime::operator-=(const time_t &tt) {
        this->set(value() - tt);
        return *this;
    }

    // day of the month :1~31
    int CDateTime::m_days() const {
        int m = this->month();
        if (m==1 || m==3 || m==5 || m==7 || m==8 || m==10 || m==12) {
            return 31;
        } else if (m == 2) {
            int leap = (this->year()) % 4;
            if (leap == 0) {
                return 29;
            } else {
                return 28;
            }
        } else {
            return 30;
        }
    }

    CDateTime operator+(const CDateTime &date1, const CDateTime &date2) {
        CDateTime newdate;
        newdate.set(date1.value() + date2.value());
        return newdate;
    }
    CDateTime operator+(const CDateTime &date, const time_t &tt) {
        CDateTime newdate;
        newdate.set(date.value() + tt);
        return newdate;
    }

    CDateTime operator-(const CDateTime &date1, const CDateTime &date2) {
        CDateTime newdate;
        newdate.set(date1.value() - date2.value());
        return newdate;
    }
    CDateTime operator-(const CDateTime &date, const time_t &tt) {
        CDateTime newdate;
        newdate.set(date.value() - tt);
        return newdate;
    }

} // namespace

