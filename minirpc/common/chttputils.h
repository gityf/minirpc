/*
** Copyright (C) 2014 Wang Yaofu
** All rights reserved.
**
**Author:Wang Yaofu voipman@qq.com
**Description: The header file of class CHttpUtils.
*/
#ifndef _COMMON_CHTTPUTILS_H_
#define _COMMON_CHTTPUTILS_H_

#include <string>
#include <vector>
#include <map>

using namespace std;

namespace wyf {

    const string HTTP_CRLF = "\r\n";
    const string DOUBLE_CRLF = "\r\n\r\n";

    struct SUrlInfo {
        string iHost;
        string iIpAddress;
        string iUrl;
        string iUrlParams;
        int iPort;
    };
    class CHttpUtils {
    public:
        // Constructor
        CHttpUtils(){};

        // Destructor
        virtual ~CHttpUtils(){};

        // set HTTP header
        void SetHeader(const string &name, const string &value);
        // set HTTP request Referer Header
        void SetReferer(const string &referer);
        // set HTTP request Authorization Header
        void SetBasicAuth(const string &username, const string &password);
        // set HTTP request Proxy Authorization Header
        void SetProxyAuth(const string &username, const string &password);
        // set HTTP request Cookie Header
        void SetCookie(const string &name, const string &value);
        // set HTTP request CGI params
        void SetUrlParam(const string &name, const string &value);

        // get http header by name.
        string GetHeader(const string &name);
        // get HTTP Set-Cookie Header
        void GetCookie(std::vector<std::string> *r);
        string GetUrlParam() {
            return iUrlParams;
        }
        // dump all http message to string.
        string DumpHeader();
        // clear all request and response HTTP header and body.
        void Clear();

        // get HTTP response Status
        inline string Status() const {
            return iHttpStatus;
        }
        // get HTTP response Content.
        inline string Body() const {
            return iHttpBody;
        }
        // get HTTP response length of Content.(Content-Length)
        inline size_t ContentLength() const {
            return iHttpBody.length();
        }

        // HTTP request string.
        inline string RequestPack() const {
            return iRequest;
        }
        // HTTP response string.
        inline string ResponsePack() const {
            return iResonse;
        }
        // Parse HTTP URL string.
        bool ParseUrl(const string &urlstr, struct SUrlInfo &aUrlInfo);
        // generate HTTP request string.
        string GenerateRequest(const string &url, const string &params, const string &method);
        // parse HTTP response
        bool ParseResponse(const string &response);
        // parse HTTP body content type of chunked.
        string ParseChunked(const string &chunkedstr);
        // http encode.
        std::string EecodeUri(const std::string& srcStr);

        // http decode.
        void DecodeUri(char* to, const char* from);

        // http hexadecimal decodes.
        int HexIt(char c);

        static string GetHeaderItem(const char* aHeader, const char* aType);
        static int GetHeaderValue(const char* aHeader, const char* aType);
        static const char* GetBodyStr(const char* aData);
    private:
        // set
        string iRequest;             // generated Request
        string iUrlParams;           // http Request params
        map<string, string> iSetMap; // push http headers

        // get
        string iResonse;             // server response
        string iHttpStatus;          // http response Status
        string iHttpBody;            // http response Body
        map<string, string> iGetMap; // recv http headers
    };

} // namespace

#endif // _COMMON_CHTTPUTILS_H_

