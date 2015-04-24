/*
** Copyright (C) 2014 Wang Yaofu
** All rights reserved.
**
**Author:Wang Yaofu voipman@qq.com
**Description: The header file of class CDateTime.
*/

#ifndef _COMMON_CDATETIME_H_
#define _COMMON_CDATETIME_H_

#include <string>
#include <ctime>
#include <stdint.h>
#include <sys/time.h>
using namespace std;

namespace wyf {

    class CDateTime {
    public:

        // Constructor, from localtime now.
        CDateTime() {
            this->set();
        }

        CDateTime(const time_t &tt) {
            this->set(tt);
        }

        CDateTime(const int year, const int mon, const int mday,
            const int hour=0, const int min=0, const int sec=0) {
            this->set(year, mon, mday, hour, min, sec);
        }

        // param tm structure
        CDateTime(const tm &st) {
            this->set(st);
        }

        CDateTime(const string &datetime, const string &datemark = "-",
            const string &dtmark = " ", const string &timemark = ":") {
            this->set(datetime, datemark, dtmark, timemark);
        }

        CDateTime (const CDateTime &date) {
            this->set(date);
        }

        // Destructor
        virtual ~CDateTime() {}

        CDateTime& operator=(const CDateTime &date);
        CDateTime& operator=(const time_t &tt);
        CDateTime& operator+=(const CDateTime &date);
        CDateTime& operator+=(const time_t &tt);
        CDateTime& operator-=(const CDateTime &date);
        CDateTime& operator-=(const time_t &tt);

        // return YYYY
        inline int year() const {return _tm.tm_year+1900;}
        // return MM 1~12
        inline int month() const {return _tm.tm_mon+1;}
        // day of the month 1~31
        inline int m_day() const {return _tm.tm_mday;}
        // days of the month 1~31
        int m_days() const;
        // week day monday-Saturday 1~6£¬sunday 0
        inline int w_day() const {return _tm.tm_wday;}
        // which day of this year 0~365
        inline int y_day() const {return _tm.tm_yday;}
        // hour 0~23
        inline int hour() const {return _tm.tm_hour;}
        // min 0~59
        inline int min() const {return _tm.tm_min;}
        // secs 0~59
        inline int sec() const {return _tm.tm_sec;}

        // return TimeStampInMs from 1970-1-1 0:0:0 to now.
        int64_t msecs() {
            int64_t time = static_cast<int64_t>(usecs() * 0.001 + 0.01);
            return time;
        }
        // return TimeStampInUs from 1970-1-1 0:0:0 to now.
        int64_t usecs() {
            struct timeval tv;
            ::gettimeofday(&tv, NULL);
            int64_t time = tv.tv_sec * 1000000 + tv.tv_usec;
            return time;
        }
        void getTime(long *seconds, long *milliseconds)
        {
            struct timeval tv;
            ::gettimeofday(&tv, NULL);
            *seconds = tv.tv_sec;
            *milliseconds = tv.tv_usec/1000;
        }
        // seconds from 1970-1-1 0:0:0 to now.
        inline time_t secs() const {return _time;}
        // mins from  1970-1-1 0:0:0 to now.
        inline time_t mins() const {return (_time/60);}
        // hours from  1970-1-1 0:0:0 to now.
        inline time_t hours() const {return (_time/3600);}
        // days from  1970-1-1 0:0:0 to now.
        inline time_t days() const {return (_time/86400);}
        // weeks from  1970-1-1 0:0:0 to now.
        inline time_t weeks() const {return (_time/604800);}

        // set by now
        void set();
        // set by time_t
        void set(const time_t &tt);
        // set by tm
        void set(const tm &st);
        // set by year mon day...
        void set(const int year, const int mon, const int mday,
            const int hour=0, const int min=0, const int sec=0);
        // set by cdatetime
        void set(const CDateTime &date);
        // set by format "YYYY-MM-DD HH:MM:SS"
        void set(const string &datetime, const string &datemark = "-",
            const string &dtmark = " ", const string &timemark = ":");

        // return time_t
        inline time_t value() const {return this->secs();}
        // return struct tm
        inline tm struct_tm() const {return _tm;}

        // YYYY-MM-DD
        string date(const string &datemark = "-",
            const bool leadingzero = true) const;

        // HH:MI:SS
        string time(const string &timemark = ":",
            const bool leadingzero = true) const;

        // YYYY-MM-DD HH:MI:SS
        string datetime(const string &datemark = "-", const string &dtmark = " ",
            const string &timemark = ":", const bool leadingzero = true) const;

        // format of GMT
        string gmt_datetime() const;

    private:
        time_t _time;
        struct tm _tm;
    };

    CDateTime operator+(const CDateTime &date1, const CDateTime &date2);
    CDateTime operator+(const CDateTime &date, const time_t &tt);
    CDateTime operator-(const CDateTime &date1, const CDateTime &date2);
    CDateTime operator-(const CDateTime &date, const time_t &tt);
    inline bool operator==(const CDateTime &left, const CDateTime &right) {
        return (left.value() == right.value());
    }
    inline bool operator==(const CDateTime &left, const time_t &right) {
        return (left.value() == right);
    }
    inline bool operator!=(const CDateTime &left, const CDateTime &right) {
        return (left.value() != right.value());
    }
    inline bool operator!=(const CDateTime &left, const time_t &right) {
        return (left.value() != right);
    }
    inline bool operator>(const CDateTime &left, const CDateTime &right) {
        return (left.value() > right.value());
    }
    inline bool operator>(const CDateTime &left, const time_t &right) {
        return (left.value() > right);
    }
    inline bool operator<(const CDateTime &left, const CDateTime &right) {
        return (left.value() < right.value());
    }
    inline bool operator<(const CDateTime &left, const time_t &right) {
        return (left.value() < right);
    }
    inline bool operator>=(const CDateTime &left, const CDateTime &right) {
        return (left.value() >= right.value());
    }
    inline bool operator>=(const CDateTime &left, const time_t &right) {
        return (left.value() >= right);
    }
    inline bool operator<=(const CDateTime &left, const CDateTime &right) {
        return (left.value() <= right.value());
    }
    inline bool operator<=(const CDateTime &left, const time_t &right) {
        return (left.value() <= right);
    }

} // namespace

#endif //_COMMON_CDATETIME_H_

