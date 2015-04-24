/*
** Copyright (C) 2014 Wang Yaofu
** All rights reserved.
**
**Author:Wang Yaofu voipman@qq.com
**Description: The header file of class CHttpClient.
*/
#ifndef _COMMON_CHTTPCLIENT_H_
#define _COMMON_CHTTPCLIENT_H_

#include <string>
#include <vector>
#include <map>
#include "common/csocket.h"
using namespace std;

namespace wyf {
    class CHttpUtils;
    class CHttpClient {
    public:

        // enum error message.
        enum EHttpErrCode {
            // OK OK
            kOkHttpOK = 0,
            // socket error
            kErrCreateSocket,
            // can not connect to server.
            kErrConnServer,
            // send request failed to server.
            kErrSendToServer,
            // time-out is coming.
            kErrRespTimeout,
            // bad http server address.
            kErrBadHttpAddress,
            // http request error.
            kErrRequNull,
            // http response is null.
            kErrRespNull,
            // bad style of response.
            kErrInvalidResponse,
            // bad http response status.
            kErrRespStatus,
            // unknown error.
            kErrUnknown
        };
        // constructor.
        CHttpClient(){
            iIsInited = false;
        };
        // construct and run HTTP request.
        CHttpClient(const string &url, const string &method = "GET",
                    const int timeout = 5) {
            iIsInited = false;
            Request(url, method, timeout);
        }
        // destructor.
        virtual ~CHttpClient();
        bool InitData();
        bool SimpleHttpRequ(SAddrInfo* aAddrInfo,
                            string &response, const int timeout);
        // execute HTTP request.
        bool Request(const string &url,
                     const string &method = "GET", const int timeout = 5);
        // URL is exist or not.
        bool IsUrlExist(const string &url);
        // HTTP request is Done.
        bool Done() const;
        // get the error message.
        string ErrDesc() const;
        // get the error message.
        int ErrCode() const;
        // get httputils pointer
        CHttpUtils* GetHttpUtils() {
            return iHttpUtils;
        }
        CHttpUtils *iHttpUtils;
    private:
        EHttpErrCode iErrCode;           // current error code
        CSocket    *iSocket;
        bool iIsInited;
    };

} // namespace

#endif //_COMMON_CHTTPCLIENT_H_

