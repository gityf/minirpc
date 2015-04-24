/*
** Copyright (C) 2014 Wang Yaofu
** All rights reserved.
**
**Author:Wang Yaofu voipman@qq.com
**Description: The source file of class CHttpUtils.
*/
#include <cstdlib>
#include <cstring>
#include <set>
#include <iostream>
#include "common/chttputils.h"
#include "common/cstringutils.h"
#include "common/base64.h"

namespace wyf {
    static char sChar2EscapeTmp[] = {
    0x1, 0x2, 0x3, 0x4, 0x5, 0x6, 0x7, 0x8, 0x9, 0xA,
    0xB, 0xC, 0xD, 0xE, 0xF, 0x10, 0x11, 0x12, 0x13,
    0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1A, 0x1B,
    0x1C, 0x1D, 0x1E, 0x1F, '"', '#', '%', '\'', '*',
    '/', ':', '=', '?', '\\', 0x7F, '{', '[', ']'};
    //*/
    static std::set<char> sChar2Escape(sChar2EscapeTmp, sChar2EscapeTmp + sizeof(sChar2EscapeTmp));
    // http encode.
    std::string CHttpUtils::EecodeUri(const std::string& srcStr) {
        std::string ret;
        for (size_t i = 0; i < srcStr.size(); ++i) {
            const char& c = srcStr[i];
            if (c >= 0 && c < 0x7F && (sChar2Escape.find(c) != sChar2Escape.end()))
            {
                ret.append("%" + CStrUitls::HexFormatStr(c));
            } else {
                ret.append(&c, 1);
            }
        }
        return ret;
    }

    // http decode.
    void CHttpUtils::DecodeUri(char* to, const char* from) {
        for (; *from != '\0'; from++) {
            if (from[0] == '+') {
                *to++ = ' ';
            } else if (from[0] == '%' && isxdigit(from[1]) && isxdigit(from[2])) {
                if (from[1] == '0' && (from[2] == 'D' || from[2] == 'd')) {
                    *to = 0x0d;
                } else if(from[1] == '0' && (from[2] == 'A' || from[2] == 'a')) {
                    *to = 0x0a;
                } else {
                    *to = HexIt(from[1]) * 16 + HexIt(from[2]);
                }
                to++;
                from += 2;
            } else {
                *to++ = *from;
            }
        }
        *to = '\0';
    }

    // http hexadecimal decodes.
    int CHttpUtils::HexIt(char c) {
        if (c >= '0' && c <= '9')
            return c - '0';
        if (c >= 'a' && c <= 'f')
            return c - 'a' + 10;
        if (c >= 'A' && c <= 'F')
            return c - 'A' + 10;
        return 0;
    }

    // param name Header name and value
    void CHttpUtils::SetHeader(const string &name, const string &value) {
        if (!name.empty()) {
            iSetMap[name] = value;
        }
    }

    // param referer Referer Header value
    void CHttpUtils::SetReferer(const string &referer) {
        if (referer.empty()) {
            SetHeader("Referer", referer);
        }
    }

    // set HTTP request Authorization Header
    void CHttpUtils::SetBasicAuth(const string &username, const string &password) {
        if (!username.empty()) {
            string auth = username + ":" + password;
            string authOut;
            Base64Encode(auth, &authOut);
            auth = "Basic " + authOut;
            SetHeader("Authorization", auth);
        }
    }

    // set HTTP request Proxy Authorization Header
    void CHttpUtils::SetProxyAuth(const string &username, const string &password) {
        if (!username.empty()) {
            string auth = username + ":" + password;
            string authOut;
            Base64Encode(auth, &authOut);
            auth = "Basic " + authOut;
            SetHeader("Proxy-Authorization", auth);
        }
    }

    // set HTTP request Cookie Header
    void CHttpUtils::SetCookie(const string &name, const string &value) {
        if (!name.empty()) {
            if (iSetMap.find("Cookie") != iSetMap.end()) {
                iSetMap["Cookie"] += "; ";
            }
            iSetMap["Cookie"] += (EecodeUri(name) + "=" + EecodeUri(value));
        }
    }

    // set HTTP request CGI param.
    void CHttpUtils::SetUrlParam(const string &name, const string &value) {
        if (!name.empty()) {
            if (iUrlParams != "") iUrlParams += "&";
            iUrlParams += (EecodeUri(name) + "=" + EecodeUri(value));
        }
    }

    // Parse HTTP URL from urlstr to aUrlInfo
    bool CHttpUtils::ParseUrl(const string &urlstr, struct SUrlInfo &aUrlInfo) {
        string url;
        CStrUitls::TrimWhitespace(urlstr, kTrimAll, &url);
        if (url.empty()) {
            return false;
        }

        // parse hostname and url
        size_t pos;
        aUrlInfo.iHost = "";
        aUrlInfo.iUrl = url;
        if (url.compare(0, 7, "http://") == 0) {
            // http://...
            if ((pos=url.find("/",7)) != url.npos) {
                // http://hostname/...
                aUrlInfo.iHost = url.substr(7, pos-7);
                aUrlInfo.iUrl = url.substr(pos);
            } else {
                // http://hostname
                aUrlInfo.iHost = url.substr(7);
                aUrlInfo.iUrl = "/";
            }
        }

        // parse param
        aUrlInfo.iUrlParams = "";
        if ((pos=aUrlInfo.iUrl.rfind("?")) != aUrlInfo.iUrl.npos) {
            // cgi?param
            aUrlInfo.iUrlParams = aUrlInfo.iUrl.substr(pos+1);
            aUrlInfo.iUrl.erase(pos);
        }

        // parse port
        aUrlInfo.iPort = 80;
        if ((pos=aUrlInfo.iHost.rfind(":")) != aUrlInfo.iHost.npos) {
            // hostname:post
            aUrlInfo.iPort = atoi(aUrlInfo.iHost.substr(pos+1).c_str());
            aUrlInfo.iHost.erase(pos);
        }
        return true;
    }

    // generate HTTP request string.
    string CHttpUtils::GenerateRequest(const string &url, const string &params, const string &method) {
        string Request;
        Request.reserve(512);

        Request += method + " " + url;
        if (method != "POST" && params != "")
            Request += "?" + params;
        Request += " HTTP/1.1" + HTTP_CRLF;
        Request += "Accept: */*" + HTTP_CRLF;
        Request += "User-Agent: Mozilla/4.0 (compatible; HttpUtils)" + HTTP_CRLF;
        Request += "Pragma: no-cache" + HTTP_CRLF;
        Request += "Cache-Control: no-cache" + HTTP_CRLF;

        map<string,string>::const_iterator i;
        for (i = iSetMap.begin(); i != iSetMap.end(); ++i) {
            if (!i->first.empty()) {
                Request += i->first + ": " + i->second + HTTP_CRLF;
            }
        }

        Request += "Connection: close" + HTTP_CRLF;

        if (method == "POST") {
            // post data
            Request += "Content-Type: application/x-www-form-urlencoded" + HTTP_CRLF;
            Request += "Content-Length: " + CStrUitls::ToStringT(params.length()) + HTTP_CRLF;
            Request += HTTP_CRLF;
            Request += params;
        }

        Request += HTTP_CRLF;
        return Request;
    }

    // parse HTTP response body.
    bool CHttpUtils::ParseResponse(const string &response) {
        // Clear response Status
        iHttpStatus = "";
        iHttpBody = "";
        iGetMap.clear();

        // split header and body
        size_t pos;
        string head;
        string body;
        if ((pos=response.find(DOUBLE_CRLF)) != response.npos) {
            head = response.substr(0, pos);
            body = response.substr(pos+4);
        } else if ((pos=response.find("\n\n")) != response.npos) {
            head = response.substr(0, pos);
            body = response.substr(pos+2);
        } else {
            return false;
        }

        // parse Status
        string status;
        if ((pos=head.find(HTTP_CRLF)) != head.npos) {
            status = head.substr(0, pos);
            head.erase(0, pos+2);

            // HTTP/1.1 status_number description_string
            iGetMap["HTTP_STATUS"] = status;
            if (status.compare(0, 5, "HTTP/") == 0) {
                size_t b1, b2;
                if ((b1=status.find(" "))!=status.npos
                    && (b2=status.find(" ",b1+1))!=status.npos)
                    iHttpStatus = status.substr(b1+1, b2-b1-1);
            }
        }

        // parse header
        string line, name, value;
        vector<string> headList;
        CStrUitls::SplitString(head, "\n", &headList);
        int ii = 0;
        for (ii = 0; ii < headList.size(); ++ii) {
            line = headList[ii];
            CStrUitls::TrimWhitespace(line, kTrimAll, &line);
            // name: value
            if ((pos=line.find(":")) != line.npos) {
                name = line.substr(0, pos);
                CStrUitls::TrimWhitespace(name, kTrimAll, &name);
                value = line.substr(pos+1);
                CStrUitls::TrimWhitespace(value, kTrimAll, &value);

                if (name != "") {
                    if (iGetMap[name] != "")
                        iGetMap[name] += "\n";
                    iGetMap[name] += value;
                }
            }
        }

        // parse body
        if (this->GetHeader("Transfer-Encoding") == "chunked")
            iHttpBody = this->ParseChunked(body);
        else
            iHttpBody = body;
    }

    // get http header by name.
    string CHttpUtils::GetHeader(const string &name) {
        if (!name.empty()) {
            return iGetMap[name];
        } else {
            return string("");
        }
    }

    // get HTTP response Set-Cookie Header
    void CHttpUtils::GetCookie(std::vector<std::string> *r) {
        string ck = this->GetHeader("Set-Cookie");
        CStrUitls::SplitString(ck, "\n", r);
    }

    // get HTTP header message.
    string CHttpUtils::DumpHeader() {
        // Status
        string header = (this->GetHeader("HTTP_STATUS") + "\n");

        // for multi header
        string headers;
        vector<string> headerlist;

        map<string,string>::const_iterator iter;
        for (iter = iGetMap.begin(); iter != iGetMap.end(); ++iter) {
            if (!iter->first.empty() && iter->first != "HTTP_STATUS") {
                if ((iter->second).find("\n") != (iter->second).npos) {
                    headers = iter->second;
                    CStrUitls::SplitString(headers, "\n", &headerlist);
                    for (size_t j=0; j < headerlist.size(); ++j)
                        header += iter->first + ": " + headerlist[j] + "\n";
                } else {
                    header += iter->first + ": " + iter->second + "\n";
                }
            }
        }

        return header;
    }

    // clear all request and response HTTP header and body.
    void CHttpUtils::Clear() {
        // set
        iUrlParams = "";
        iSetMap.clear();

        // get
        iHttpStatus = "";
        iHttpBody = "";
        iGetMap.clear();
    }

    // parse HTTP body content type of chunked.
    string CHttpUtils::ParseChunked(const string &chunkedstr) {
        char crlf[3] = "\x0D\x0A";
        size_t pos, lastpos;
        long size = 0;
        string hexstr;

        // location HTTP_CRLF
        if ((pos=chunkedstr.find(crlf)) != chunkedstr.npos) {
            hexstr = chunkedstr.substr(0, pos);
            size = CStrUitls::Str2Long(hexstr, 16);
        }

        string res;
        res.reserve(chunkedstr.length());

        while (size > 0) {
            // append to Body
            res += chunkedstr.substr(pos+2, size);
            lastpos = pos+size+4;

            // location next HTTP_CRLF
            if ((pos=chunkedstr.find(crlf,lastpos)) != chunkedstr.npos) {
                hexstr = chunkedstr.substr(lastpos, pos-lastpos);
                size = CStrUitls::Str2Long(hexstr, 16);
            } else {
                break;
            }
        }

        return res;
    }

    // header = "Content-Length: 123\r\n", type = "Content-Length"
    // return 123\r\n
    string CHttpUtils::GetHeaderItem(const char* aHeader, const char* aType) {
        const char* p = strcasestr(aHeader, aType);
        if (p == NULL) {
            return "";
        }
        p += (strlen(aType) + 1);
        while (isblank(*p) != 0) {
            p++;
        }
        const char* pEnd = strstr(p, HTTP_CRLF.c_str());
        if (pEnd != NULL) {
            string retstr;
            retstr.assign(p, pEnd);
            return retstr;
        } else {
            return p;
        }
    }

    // header = "Content-Length: 123", type = "Content-Length"
    int CHttpUtils::GetHeaderValue(const char* aHeader, const char* aType) {
        string p = GetHeaderItem(aHeader, aType);
        return (p.empty()) ? -1 : atoi(p.c_str());
    }

    // HTTP body
    const char* CHttpUtils::GetBodyStr(const char* aData) {
        const char* p = strstr(aData, DOUBLE_CRLF.c_str());
        return (p != NULL) ? p + 4 : NULL;
    }
} // namespace wyf
