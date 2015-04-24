/*
** Copyright (C) 2014 Wang Yaofu
** All rights reserved.
**
**Author:Wang Yaofu voipman@qq.com
**Description: The header file of class CStringUitls.
*/

#ifndef _COMMON_CSTRINGUITLS_H_
#define _COMMON_CSTRINGUITLS_H_
#include <string>
#include <sstream>
#include <vector>
#include <map>
#include <set>
#include <cctype>
namespace wyf {
    enum TrimOption {
        kTrimNone = 0,
        kTrimLeading = 1 << 0,
        kTrimTrailing = 1 << 1,
        kTrimAll = kTrimLeading | kTrimTrailing,
    };
    class CStrUitls {
    public:
        static void TrimWhitespace(const std::string& input,
            TrimOption option,
            std::string* output);

        static void ReplaceString(const std::string& input,
            const std::string& old_val,
            const std::string& new_val,
            std::string* output);

        static std::string ReplaceString(const std::string& input,
            const std::string& old_val,
            const std::string& new_val) {
            std::string output;
            ReplaceString(input, old_val, new_val, &output);
            return output;
        }

        // ABC => abc
        static std::string ToLowerCase(const std::string& input);
        // abc => ABC
        static std::string ToUpperCase(const std::string& input);
        // 11&22&33 +& =>11 22 33 + &
        static void SplitString(const std::string& str,
            const std::string& s,
            std::vector<std::string>* r);
        // string to split string
        static bool Split(const std::string& s,
          char sep,
          std::string& first,
          std::string& second);
        // any to string.
        template <typename INT_TYPE>
        static std::string ToStringT(const INT_TYPE& aValue) {
            std::stringstream ss;
            ss << aValue;
            return ss.str();
        }

        template <typename K, typename V>
        static std::string ToStringT(const typename std::pair<K, V>& v) {
            std::ostringstream o;
            o << ToStringT(v.first) << ": " << ToStringT(v.second);
            return o.str();
        }

        template <typename T>
        static std::string ToStringT(const T& beg, const T& end)
        {
            std::ostringstream o;
            for (T it = beg; it != end; ++it) {
                if (it != beg)
                    o << ", ";
                o << ToStringT(*it);
            }
            return o.str();
        }

        template <typename T>
        static std::string ToStringT(const std::vector<T>& t) {
            std::ostringstream o;
            o << "[" << ToStringT(t.begin(), t.end()) << "]";
            return o.str();
        }

        template <typename K, typename V>
        static std::string ToStringT(const std::map<K, V>& m) {
            std::ostringstream o;
            o << "{" << ToStringT(m.begin(), m.end()) << "}";
            return o.str();
        }

        template <typename T>
        static std::string ToStringT(const std::set<T>& s) {
            std::ostringstream o;
            o << "{" << ToStringT(s.begin(), s.end()) << "}";
            return o.str();
        }

        template <class Type>
        static Type stringToNum(const std::string& str)
        {
            istringstream iss(str);
            Type num;
            iss >> num;
            return num;
        }

        /// string to long
        static long Str2Long(const std::string &str, const int base = 10);
        static int Str2Int(const std::string &str);
        // 11 22 33 + & => 11&22&33
        static std::string JoinStr(std::vector<std::string>& parts, char delimiter);
        // 11 22 33 + %% => 11%%22%%33
        static std::string JoinStr(std::vector<std::string>& parts, const std::string& delimiter);

        // 19AaZz\n => 31 39 41 61 5a 7a 0a
        static std::string HexFormatStr(const char* cpSrcStr, int len, bool isUpper = false);
        static std::string HexFormatStr(const std::string& srcStr, bool isUpper = false);
        static std::string HexFormatStr(const char ch, bool isUpper = false);
        // "ABCD" to 43981
        static int HexStr2Int(const char* aStr, int len);
        static int HexStr2Int(const std::string& srcStr);
        // 1234 to 0x1234 (4660)
        static int Int2HexInt(int src);
        static bool IsNumber(char ch);
        static bool IsLetter(char ch);
        static bool IsUnderLine(char ch);
        static bool IsGbk(const unsigned char c1, const unsigned char c2);
        static bool Format(std::string &outStr, const char *format, ...);
        static unsigned int HfIp(const char* aStr, unsigned int aHashMod);
        static unsigned int hf(const char* aStr, unsigned int aHashMod);
        // System - v hash method, it is nice.
        static unsigned int ELFhash(const char* aStr, unsigned int aHashMod);
        // {1} 2 {3} 4 {5 6} to 1 2 3 4 '5 6'
        static int StrIndex(char *aStrIn, int aIndex, char *aStrOut);
    };
} // namespace wyf
#endif  // _COMMON_CSTRINGUITLS_H_
