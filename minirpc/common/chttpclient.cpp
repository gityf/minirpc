/*
** Copyright (C) 2014 Wang Yaofu
** All rights reserved.
**
**Author:Wang Yaofu voipman@qq.com
**Description: The source file of class CHttpClient.
*/
#include <unistd.h>
#include "common/chttpclient.h"
#include "common/localdef.h"
#include "common/chttputils.h"
#include "common/cstringutils.h"
using namespace std;

namespace wyf {
    CHttpClient::~CHttpClient() {
        if (iHttpUtils != NULL) {
            delete iHttpUtils;
            iHttpUtils = NULL;
        }
        if (iHttpUtils != NULL) {
            delete iHttpUtils;
            iHttpUtils = NULL;
        }
    }

    bool CHttpClient::InitData() {
        if (!iIsInited) {
            iHttpUtils = new CHttpUtils();
            iSocket = new CSocket();
            if (iHttpUtils == NULL || iSocket == NULL) {
                return false;
            }
            iIsInited = true;
        }
        return iIsInited;
    }
    bool CHttpClient::SimpleHttpRequ(SAddrInfo* aAddrInfo, string &response, const int timeout) {
        if (!iIsInited) {
            if (!InitData()) {
                return false;
            }
        }

        iErrCode = kOkHttpOK;
        do {
            if (RET_ERROR == iSocket->MakeTcpConn(aAddrInfo)) {
                iErrCode = kErrConnServer;
                break;
            }
            int ret = iSocket->TcpSend(aAddrInfo->iSockId, aAddrInfo->iReqCmd.c_str(),
                aAddrInfo->iReqCmd.length(), timeout, false);
            if (RET_ERROR == ret) {
                iErrCode = kErrSendToServer;
                break;
            }
            iSocket->NonBlock(aAddrInfo->iSockId, true);
            response.resize(MAX_SIZE_1M);
            char *buffPtr = const_cast<char *>(response.data());
            ret = iSocket->TcpRecvAll(aAddrInfo->iSockId, buffPtr, MAX_SIZE_1M, timeout);
            if (ret != RET_ERROR) {
                response.resize(ret);
            } else {
                response.clear();
                iErrCode = kErrRespNull;
                break;
            }
        } while(0);
        close(aAddrInfo->iSockId);
        return (iErrCode == kOkHttpOK) ? true : false;
    }

    // 执行HTTP请求
    // \param url HTTP请求URL
    // \param method HTTP请求Method,默认为"GET"
    // \param timeout HTTP请求超时时长,单位为秒,默认为5秒
    // \retval true 执行成功
    // \retval false 执行失败
    bool CHttpClient::Request(const string &url,
        const string &method, const int timeout) {
        iErrCode = kOkHttpOK;
        if (!iIsInited) {
            if (!InitData()) {
                return false;
            }
        }

        struct SUrlInfo urlInfo;
        iHttpUtils->ParseUrl(url, urlInfo);

        // check params
        string urlParams = iHttpUtils->GetUrlParam();
        if (!urlParams.empty()) {
            if (!urlInfo.iUrlParams.empty()) {
                urlInfo.iUrlParams += "&";
            }
            urlInfo.iUrlParams += urlParams;
        }
        // check host
        if (urlInfo.iHost.empty()) {
            return false;
        }
        if (iSocket->IsIPv4Addr(urlInfo.iHost)) {
            urlInfo.iIpAddress = urlInfo.iHost;
        } else {
            char ipstr[64] = {0};
            if (iSocket->Name2IPv4(urlInfo.iHost.c_str(), ipstr)) {
                urlInfo.iIpAddress = ipstr;
            } else {
                return false;
            }
        }


        SAddrInfo addrInfo;
        // generate Request string
        addrInfo.iReqCmd = iHttpUtils->GenerateRequest(urlInfo.iUrl,
            urlInfo.iUrlParams, method);
        addrInfo.iAddr = urlInfo.iIpAddress;
        addrInfo.iPort = urlInfo.iPort;
        addrInfo.iProtocol = TCP;
        // Request
        string response;
        if (!SimpleHttpRequ(&addrInfo, response, timeout)) {
            return false;
        }

        // parse response
        if (response.empty()) {
            iErrCode = kErrRespNull;
            return false;
        }

        iHttpUtils->ParseResponse(response);
        return true;
    }

    // URL 是否有效
    // \param url HTTP请求URL
    // \param server 服务器IP,为空字符串则根据参数1获得,默认为空字符串,
    // 若参数url,server都不包含服务器地址信息,则函数返回失败
    // \param port 服务器端口,默认为80
    // \retval true URL有效
    // \retval false URL已失效
    bool CHttpClient::IsUrlExist(const string &url) {
        bool res = false;

        // check
        if (Request(url, "HEAD", 5) && Done())
            res = true;
        iHttpUtils->Clear();

        return res;
    }

    // 执行HTTP请求是否成功
    // \retval true 成功
    // \retval false 失败
    bool CHttpClient::Done() const {
        string httpStatus = iHttpUtils->Status();
        int ret = CStrUitls::Str2Int(httpStatus);
        if (ret>=100 && ret<300)
            return true;
        return false;
    }

    string CHttpClient::ErrDesc() const {
        return string("TODO");
    }

    int CHttpClient::ErrCode() const {
        return static_cast<int>(iErrCode);
    }

} // namespace wyf
