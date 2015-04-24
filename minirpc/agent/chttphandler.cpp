/*
** Copyright (C) 2014 Wang Yaofu
** All rights reserved.
**
**Author:Wang Yaofu voipman@qq.com
**Description: The source file of class CHttpHandler.
*/
#include <unistd.h>
#include "agent/cagentservice.h"
#include "agent/chttphandler.h"
#include "agent/csessionsync.h"
#include "agent/csessionrs.h"
#include "agent/version.h"
#include "common/cconfig.h"
#include "common/clogwriter.h"
#include "common/csingleton.h"
#include "common/chttputils.h"
#include "common/cstringutils.h"
#include "common/ev/ceventhandler.h"
#include "common/ev/ceventloop.h"
#include "common/base64.h"

// ---------------------------------------------------------------------------
// CHttpHandler::CHttpHandler()
//
// constructor.
// ---------------------------------------------------------------------------
//
CHttpHandler::CHttpHandler() : iStopRequested(false) {
    const char *funcName = "CHttpHandler::CHttpHandler()";
    DEBUG(LL_ALL, "%s:Begin", funcName);
    iSocket = NULL;
    iEventLoop = NULL;
    DEBUG(LL_ALL, "%s:End", funcName);
}

// ---------------------------------------------------------------------------
// CHttpHandler::~CHttpHandler()
//
// Destructor.
// ---------------------------------------------------------------------------
//
CHttpHandler::~CHttpHandler() {
    const char *funcName = "CHttpHandler::~CHttpHandler()";
    DEBUG(LL_ALL, "%s:Begin", funcName);
    if (iSocket != NULL) {
        delete iSocket;
        iSocket = NULL;
    }
    if (iEventLoop != NULL) {
        delete iEventLoop;
        iEventLoop = NULL;
    }
    DEBUG(LL_ALL, "%s:End", funcName);
}

// ---------------------------------------------------------------------------
// int CHttpHandler::InitData()
//
// Initial operation.
// ---------------------------------------------------------------------------
//
int CHttpHandler::InitData() {
    const char *funcName = "CHttpHandler::InitData()";
    DEBUG(LL_ALL, "%s:Begin", funcName);
    iConfig = CSingleton<CConfig>::Instance();
    iSocketRWTimeOut   = iConfig->GetConfig(CFG_SOCK_RW_TIME_OUT);
    iSocketTimeOut = iConfig->GetConfig(CFG_SOCK_TIME_OUT);
    DEBUG(LL_INFO, "%s:try start http server.", funcName);
    iSocket = new CSocket();
    if (iSocket == NULL) {
        LOG(LL_ERROR, "%s:create CSocket failed.", funcName);
        return RET_ERROR;
    }
    iTcpSerAddrInfo.iProtocol = TCP;
    iTcpSerAddrInfo.iPort = iConfig->GetConfig(CFG_TCP_BIND_PORT);
    if (iSocket->TcpServer(&iTcpSerAddrInfo) == RET_ERROR) {
        LOG(LL_ERROR, "%s::create CSocket.socket failed.", funcName);
        return RET_ERROR;
    }
    DEBUG(LL_INFO, "%s:start http server success.", funcName);
    iEventLoop = new CEventLoop();
    if (iEventLoop == NULL) {
        LOG(LL_ERROR, "%s:create CEventLoop failed.", funcName);
        return RET_ERROR;
    }
    if (iEventLoop->CreateEventLoop(HTTP_ONLINE_MAX_SIZE) == RET_ERROR) {
        LOG(LL_ERROR, "%s:CEventLoop.CreateEventLoop failed.", funcName);
        return RET_ERROR;
    }
    if (iEventLoop->AddFileEvent(iTcpSerAddrInfo.iSockId, AE_READABLE, this) == RET_ERROR) {
        LOG(LL_ERROR, "%s:CEventLoop.AddFileEvent failed.", funcName);
        return RET_ERROR;
    }
    string iReFreshStr;
    if (iConfig->GetConfig(CFG_HTTP_PAGE_REFRESH_FLAG) == 1) {
        iReFreshStr = "<meta http-equiv=\"refresh\" content=\"5\">\n";
    } else {
        iReFreshStr = "<meta http-equiv=\"content-type\" content=\"text/html; charset=gb2312\">\n";
    }

    iCssStr = "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.01 Transitional//EN\"\n"
        "\"http://www.w3.org/TR/html4/loose.dtd\">\n"
        "<html><head><title>Statistics Report for " AGENT_FULL_VERSION "</title>\n";
    iCssStr += iReFreshStr;
    iCssStr += "<style type=\"text/css\"><!--\n"
        "body {"
        " font-family: arial, helvetica, sans-serif;"
        " font-size: 12px;"
        " font-weight: normal;"
        " color: black;"
        " background: white;"
        "}\n"
        "th,td {"
        " font-size: 10px;"
        "}\n"
        "h1 {"
        " font-size: x-large;"
        " margin-bottom: 0.5em;"
        "}\n"
        "h2 {"
        " font-family: helvetica, arial;"
        " font-size: x-large;"
        " font-weight: bold;"
        " font-style: italic;"
        " color: #6020a0;"
        " margin-top: 0em;"
        " margin-bottom: 0em;"
        "}\n"
        "h3 {"
        " font-family: helvetica, arial;"
        " font-size: 16px;"
        " font-weight: bold;"
        " color: #b00040;"
        " background: #e8e8d0;"
        " margin-top: 0em;"
        " margin-bottom: 0em;"
        "}\n"
        "li {"
        " margin-top: 0.25em;"
        " margin-right: 2em;"
        "}\n"
        ".hr {margin-top: 0.25em;"
        " border-color: black;"
        " border-bottom-style: solid;"
        "}\n"
        ".in    {color: #6020a0; font-weight: bold; text-align: left;}\n"
        ".frontend {background: #e8e8d0;}\n"
        ".s   {background: #e0e0e0;}\n"
        ".a0  {background: #ff9090;}\n"
        ".a1  {background: #ffd020;}\n"
        ".a2  {background: #ff80ff;}\n"
        ".a3  {background: #c0ffc0;}\n"
        ".a4  {background: #ffffa0;}\n"  /* NOLB state shows same as going down */
        ".a5  {background: #b0d0ff;}\n"  /* NOLB state shows darker than up */
        ".a6  {background: #e0e0e0;}\n"
        ".maintain {background: #c07820;}\n"
        ".rls      {letter-spacing: 0.2em; margin-right: 1px;}\n" /* right letter spacing (used for grouping digits) */
        "\n"
        "a.px:link {color: #ffff40; text-decoration: none;}"
        "a.px:visited {color: #ffff40; text-decoration: none;}"
        "a.px:hover {color: #ffffff; text-decoration: none;}"
        "a.lfsb:link {color: #000000; text-decoration: none;}"
        "a.lfsb:visited {color: #000000; text-decoration: none;}"
        "a.lfsb:hover {color: #505050; text-decoration: none;}"
        "\n"
        "table.tbl { border-collapse: collapse; border-style: none;}\n"
        "table.tbl td { text-align: right; border-width: 1px 1px 1px 1px; border-style: solid solid solid solid; padding: 2px 3px; border-color: gray; white-space: nowrap;}\n"
        "table.tbl td.ac { text-align: center;}\n"
        "table.tbl th { border-width: 1px; border-style: solid solid solid solid; border-color: gray;}\n"
        "table.tbl th.pxname { background: #b00040; color: #ffff40; font-weight: bold; border-style: solid solid none solid; padding: 2px 3px; white-space: nowrap;}\n"
        "table.tbl th.empty { border-style: none; empty-cells: hide; background: white;}\n"
        "table.tbl th.desc { background: white; border-style: solid solid none solid; text-align: left; padding: 2px 3px;}\n"
        "\n"
        "table.lgd { border-collapse: collapse; border-width: 1px; border-style: none none none solid; border-color: black;}\n"
        "table.lgd td { border-width: 1px; border-style: solid solid solid solid; border-color: gray; padding: 2px;}\n"
        "table.lgd td.noborder { border-style: none; padding: 2px; white-space: nowrap;}\n"
        "u {text-decoration:none; border-bottom: 1px dotted black;}\n"
        "-->\n"
        "</style></head>\n";
    int ii = 0;
    while (ii < HTTP_ONLINE_MAX_SIZE) {
        iSockIdTimer[ii] = 0;
        iIsAuthed[ii] = false;
        iMsgType[ii++] = kMsgTypeNone;
    }
    // get http auth string.
    string authStr = iConfig->GetConfig(CFG_HTTP_AUTH_STR);
    if (authStr != CFG_HTTP_AUTH_STR) {
        // need to http auth.
        iIsAuthed[0] = true;
        Base64Encode(authStr, &iHttpAuthBase64Str);
        iHttpAuthBase64Str += "\r\n";
    }
    DEBUG(LL_ALL, "%s:End", funcName);
    return RET_OK;
}

// ---------------------------------------------------------------------------
// void CHttpHandler::Request(int aSockId)
//
// read event is occurred.
// ---------------------------------------------------------------------------
//
void CHttpHandler::Request(int aSockId) {
    const char *funcName = "CHttpHandler::Request";
    DEBUG(LL_ALL, "%s:Begin", funcName);
    LOG(LL_WARN, "%s:sockid:(%d) read set is ok.", funcName, aSockId);
    if (aSockId == iTcpSerAddrInfo.iSockId) {
        struct SAddrInfo clntAddInfo;
        if (iSocket->AcceptConnection(aSockId, &clntAddInfo) == RET_OK) {
           iSocket->SetLinger(clntAddInfo.iSockId);
           if (iEventLoop->AddFileEvent(clntAddInfo.iSockId,
               AE_READABLE, this) == RET_ERROR) {
                close(clntAddInfo.iSockId);
           }
        }
    } else {
        if (RecvHttpHeader(aSockId) == RET_OK) {
            iEventLoop->DeleteFileEvent(aSockId, AE_READABLE);
            iEventLoop->AddFileEvent(aSockId, AE_WRITABLE, this);
            iSockIdTimer[aSockId] =
                CThreadTimer::instance()->getNowSecs() + iSocketTimeOut;
        } else {
            close(aSockId);
            iEventLoop->DeleteFileEvent(aSockId, AE_READABLE|AE_WRITABLE|0x0f);
        }
    }
    DEBUG(LL_ALL, "%s:End", funcName);
}

// ---------------------------------------------------------------------------
// void CHttpHandler::Response(int aSockId)
//
// write event is occurred.
// ---------------------------------------------------------------------------
//
void CHttpHandler::Response(int aSockId) {
    const char *funcName = "CHttpHandler::Response";
    DEBUG(LL_ALL, "%s:Begin", funcName);
    LOG(LL_WARN, "%s:sockid:(%d) write set is ok.", funcName, aSockId);
    if (aSockId != iTcpSerAddrInfo.iSockId) {
        if (RET_ERROR ==
            SendRespBody(aSockId,
            CSingleton<CAgentService>::Instance()->GetRespInfos(iServiceNameStr[aSockId],
            iMsgType[aSockId]))) {
            LOG(LL_WARN, "%s:sockid:(%d) write error:(%s).",
                funcName, aSockId, strerror(errno));
            ErrSocket(aSockId);
        } else {
            //close(aSockId);
            iEventLoop->AddFileEvent(aSockId, AE_READABLE, this);
            iEventLoop->DeleteFileEvent(aSockId, AE_WRITABLE);
        }
    }
    DEBUG(LL_ALL, "%s:End", funcName);
}

// ---------------------------------------------------------------------------
// void ErrSocket(int aSockId)
//
// error event is occurred.
// ---------------------------------------------------------------------------
//
void CHttpHandler::ErrSocket(int aSockId) {
    const char *funcName = "CHttpHandler::ErrSocket";
    DEBUG(LL_ALL, "%s:Begin", funcName);
    LOG(LL_WARN, "%s:sockid:(%d) err set is ok.",
        funcName, aSockId);
    if (aSockId != iTcpSerAddrInfo.iSockId) {
        iSockIdTimer[aSockId] = 0;
        iMsgType[aSockId] = kMsgTypeNone;
        close(aSockId);
        LOG(LL_DBG, "%s:try delete sockid:(%d) from poll.",
            funcName, aSockId);
        iEventLoop->DeleteFileEvent(aSockId, AE_READABLE|AE_WRITABLE|0x0f);
        LOG(LL_DBG, "%s:delete sockid:(%d) from poll sucess.",
            funcName, aSockId);
    }
    DEBUG(LL_ALL, "%s:End", funcName);
}

// ---------------------------------------------------------------------------
// void CHttpHandler::run()
//
// this is a runner of local thread.
// ---------------------------------------------------------------------------
//
void CHttpHandler::run() {
    long lastSecs = CThreadTimer::instance()->getNowSecs();
    while (!iStopRequested) {
        //iEventLoop->MainLoop(AE_WAIT_MS);
        iEventLoop->MainLoop(10);
        long nowSecs = CThreadTimer::instance()->getNowSecs();
        if (nowSecs > lastSecs + 3) {
            lastSecs = nowSecs;
            // 超时检查
            for(int i = 3; i <= iEventLoop->iMaxSockId; i++) {
                //  检查Socket是否有错误
                if (i != iTcpSerAddrInfo.iSockId &&
                    iSockIdTimer[i] != 0 &&
                    iSockIdTimer[i] <= nowSecs) {
                    iSockIdTimer[i] = 0;
                    iMsgType[i] = kMsgTypeNone;
                    close(i);
                    iEventLoop->DeleteFileEvent(i, AE_READABLE|AE_WRITABLE|0x0f);
                }
            }
            iEventLoop->AddFileEvent(iTcpSerAddrInfo.iSockId, AE_READABLE, this);
        }
    }
}

// ---------------------------------------------------------------------------
// void CHttpHandler::on_stop()
//
// stop the local thread.
// ---------------------------------------------------------------------------
//
void CHttpHandler::on_stop() {
    LOG(LL_ALL, "CHttpHandler::on_stop()::Begin");
    iStopRequested = true;
    LOG(LL_ALL, "CHttpHandler::on_stop()::End");
}

// ---------------------------------------------------------------------------
// int CHttpHandler::RecvHttpHeader(int aSockId)
//
// http pack is added into buffer.
// ---------------------------------------------------------------------------
//
int CHttpHandler::RecvHttpHeader(int aSockId) {
    const char *funcName = "CHttpHandler::RecvHttpHeader()";
    DEBUG(LL_ALL, "%s:Begin", funcName);
    if (aSockId >= HTTP_ONLINE_MAX_SIZE || aSockId < 0) {
        LOG(LL_ERROR, "%s:bad sockid:(%d).", funcName, aSockId);
        return RET_ERROR;
    }

    char httpBuffer[MAX_SIZE_1K+1] = {0};
    // receive http request line.
    if (RET_ERROR == iSocket->TcpRecvHttpHeader(aSockId,
        httpBuffer, MAX_SIZE_1K, iSocketRWTimeOut)) {
        LOG(LL_ERROR, "%s:recv http request line failed.", funcName);
        return RET_ERROR;
    }
    // check method:GET POST HEAD.
    if (strncasecmp(httpBuffer, "GET ", 4) != 0) {
        if (strncasecmp(httpBuffer, "HEAD ", 5) == 0) {
            SendErrorBody(aSockId, 200, "OK");
        } else {
            SendErrorBody(aSockId, 501, "Not Implemented");
        }
        LOG(LL_ERROR, "%s:not GET http request line.", funcName);
        return RET_ERROR;
    }
    if (iIsAuthed[0]) {
        iIsAuthed[aSockId] = (strstr(httpBuffer,
            iHttpAuthBase64Str.c_str()) != NULL) ? true : false;
    } else {
        iIsAuthed[aSockId] = true;
    }
    // DONE: get statistics info by service sub-page.
    CSingleton<wyf::CHttpUtils>::Instance()->DecodeUri(httpBuffer, httpBuffer);
    char *p = strchr(httpBuffer+4, ' ');
    iServiceNameStr[aSockId] = " ";
    iMsgType[aSockId] = kMsgTypeStatAny;
    if (p != NULL) {
        *p++ = 0x00;
        iServiceNameStr[aSockId] = httpBuffer+5;
        ParseConfig(aSockId);
    }
    DEBUG(LL_ALL, "%s:End", funcName);
    return RET_OK;
}

// ---------------------------------------------------------------------------
// int CHttpHandler::ParseConfig(int aSockId)
//
// http config.
// ---------------------------------------------------------------------------
//
int CHttpHandler::ParseConfig(int aSockId) {
    const char *funcName = "CHttpHandler::RecvHttpHeader()";
    DEBUG(LL_ALL, "%s:Begin", funcName);
    iMsgType[aSockId] = kMsgTypeStatAll;
    if (iServiceNameStr[aSockId].compare(0, 3, "set") == 0) {
        iMsgType[aSockId] = kMsgTypeConfSet;
        return RET_OK;
    }
    int off_set = iServiceNameStr[aSockId].find_first_of('/');
    if (iServiceNameStr[aSockId].compare(0, 5, "gets/") == 0) {
        // orbexec get service
        iServiceNameStr[aSockId].erase(0, off_set+1);
        if (iServiceNameStr[aSockId] == MSG_TEXT_ALL) {
            iMsgType[aSockId] = kMsgTypeGetsAll;
        } else {
            iMsgType[aSockId] = kMsgTypeGetsAny;
        }
    } else if (iServiceNameStr[aSockId].compare(0, 5, "sync/") == 0) {
        // sync all agent service item.
        if (iServiceNameStr[aSockId].length() == 5 ||
             iServiceNameStr[aSockId] == "sync/all") {
            // sync all service when 'sync/' or 'sync/all'
            iServiceNameStr[aSockId] = MSG_TEXT_ALL;
            iMsgType[aSockId] = kMsgTypeSyncAll;
        } else {
            iServiceNameStr[aSockId].erase(0, off_set+1);
            iMsgType[aSockId] = kMsgTypeSyncAny;
        }
    } else if (iServiceNameStr[aSockId].compare(0, 4, "conf") == 0) {
        // add/del sync agent address.
        CSyncAddrInfo syncAddrInfo;
        char ipStr[65] = {0};
        char serviceList[1025] = {0};
        int ip1=0,ip2=0,ip3=0,ip4=0;
        int ret = sscanf(iServiceNameStr[aSockId].c_str(),
            "conf/conf?addsyncip=%d.%d.%d.%d&port=%d&interval=%d&services=%1024s",
            &ip1, &ip2, &ip3, &ip4, &syncAddrInfo.iPort,
            &syncAddrInfo.iInterval, serviceList);
        if (ret == 7) {
            sprintf(ipStr, "%d.%d.%d.%d", ip1, ip2, ip3, ip4);
            if (ip1 > 255 || ip1 <= 0 || ip2 > 255 || ip2 < 0 ||\
                 ip3 > 255 || ip3 < 0 || ip4 >= 255 || ip4 < 0) {
                LOG(LL_ERROR, "%s:bad sync ip:(%s)", funcName, ipStr);
                return RET_ERROR;
            }
            char *pAt = strrchr(serviceList, '&');
            if (pAt != NULL) {
                *pAt = 0x00;
            }
            LOG(LL_WARN, "%s:add sync config:ip:(%s),port:(%d),"
                "interval:(%d),services:(%s)",
                funcName, ipStr, syncAddrInfo.iPort,
                syncAddrInfo.iInterval,
                serviceList);
            char tmplocalip[65] = {0};
            int tmplocalipport = 0;
            if (RET_OK == iSocket->NetLocalSockName(aSockId,
                tmplocalip, 64, &tmplocalipport)) {
                LOG(LL_VARS, "%s:try to add local ip:(%s),port:(%d).",
                    funcName, tmplocalip, tmplocalipport);
                //CSessionRS::instance()->SetLocalIP(tmplocalip);
                if (strcmp(ipStr, tmplocalip) == 0 &&
                    tmplocalipport == syncAddrInfo.iPort) {
                    LOG(LL_WARN, "%s:can not add local ip:(%s),port:(%d).",
                        funcName, ipStr, syncAddrInfo.iPort);
                    return RET_ERROR;
                }
            }
            iConfig->SetConfig(CFG_SYNC_IP, ipStr);
            iConfig->SetConfig(CFG_SYNC_PORT, syncAddrInfo.iPort);
            if (syncAddrInfo.iInterval > HB_TIME_OUT) {
                syncAddrInfo.iInterval = HB_TIME_OUT;
            } else if (syncAddrInfo.iInterval < 3) {
                syncAddrInfo.iInterval = 3;
            }
            iConfig->SetConfig(CFG_SYNC_INTERVAL, syncAddrInfo.iInterval);
            syncAddrInfo.iIpAddress = ipStr;
            if (serviceList[0] != 0x00) {
                syncAddrInfo.iServices = serviceList;
            }
            syncAddrInfo.iStartSecs = time(NULL);
            CSingleton<CSessionSync>::Instance()->AddSyncAddr(syncAddrInfo);
        } else {
            ret = sscanf(iServiceNameStr[aSockId].c_str(),
                "conf/conf?delsyncip=%64[^&]&port=%d",
                ipStr, &syncAddrInfo.iPort);
            if (ret == 2) {
                syncAddrInfo.iInterval = 0;
                LOG(LL_WARN, "%s:del sync config:ip:(%s),port:(%d),interval:(%d)",
                    funcName, ipStr, syncAddrInfo.iPort, syncAddrInfo.iInterval);
                syncAddrInfo.iIpAddress = ipStr;
                CSingleton<CSessionSync>::Instance()->DelSyncAddr(syncAddrInfo);
            }
        }
        if (ret > 0) {
            iServiceNameStr[aSockId] = "conf";
        }
        iMsgType[aSockId] = kMsgTypeConf;
    }
    DEBUG(LL_ALL, "%s:End", funcName);
    return RET_OK;
}

// ---------------------------------------------------------------------------
// int CHttpHandler::SendErrorBody()
//
// send result to client.
// ---------------------------------------------------------------------------
//
int CHttpHandler::SendErrorBody(int aSockId, int aErrorCode, const char* aErrorStr) {
    const char *funcName = "CHttpHandler::SendErrorBody()";
    DEBUG(LL_ALL, "%s:Begin", funcName);
    if (aErrorStr == NULL) {
        LOG(LL_ERROR, "%s:Bad error string.", funcName);
        return RET_ERROR;
    }
    string headerStr;
    wyf::CStrUitls::Format(headerStr,
        "HTTP/1.1 %d %s\r\n"
        "Server: %s\r\n"
        "Connection: Close\r\n\r\n",
        aErrorCode, aErrorStr, AGENT_FULL_VERSION);
    if (iSocket->TcpSend(aSockId, headerStr.c_str(),
        headerStr.length(), iSocketRWTimeOut, true) != headerStr.length()) {
        LOG(LL_ERROR, "%s:TcpSend response failed.", funcName);
        return RET_ERROR;
    }
    DEBUG(LL_ALL, "%s:End", funcName);
    return RET_OK;
}

// ---------------------------------------------------------------------------
// int CHttpHandler::SendStatisticBody()
//
// send result to client.
// ---------------------------------------------------------------------------
//
int CHttpHandler::SendStatisticBody(int aSockId, const string& aSendStr) {
    const char *funcName = "CHttpHandler::SendStatisticBody()";
    DEBUG(LL_ALL, "%s:Begin", funcName);
    // Connection is set to Keep-Alive instead of Close to avoid CLOSE_WAIT state.
    string headerStr;
    wyf::CStrUitls::Format(headerStr,
        "HTTP/1.1 200 OK\r\n"
        "Cache-Control: no-cache\r\n"
        "Content-Length: %d\r\n"
        "Content-Type: text/html;charset=gb2312\r\n"
        "Connection: Keep-Alive\r\n\r\n",
        iCssStr.length() + aSendStr.length());
    INIT_IOV(3);
    SET_IOV_LEN(headerStr.c_str(), headerStr.length());
    SET_IOV_LEN(iCssStr.c_str(), iCssStr.length());
    SET_IOV_LEN(aSendStr.c_str(), aSendStr.length());
    int sendBufLen = headerStr.length() + iCssStr.length() + aSendStr.length();

    if (iSocket->TcpSendV(aSockId, iovs, 3, iSocketRWTimeOut, true) != sendBufLen) {
        LOG(LL_ERROR, "%s:TcpSend http header failed.", funcName);
        return RET_ERROR;
    }
    DEBUG(LL_ALL, "%s:End", funcName);
    return RET_OK;
}

// ---------------------------------------------------------------------------
// int CHttpHandler::SendSyncBody()
//
// send sync result to client.
// ---------------------------------------------------------------------------
//
int CHttpHandler::SendSyncBody(int aSockId, const string& aSendStr) {
    const char *funcName = "CHttpHandler::SendSyncBody()";
    DEBUG(LL_ALL, "%s:Begin", funcName);
    // Connection is set to Keep-Alive instead of Close to avoid CLOSE_WAIT state.
    string headerStr;
    wyf::CStrUitls::Format(headerStr,
        "HTTP/1.1 200 OK\r\n"
        "Date: %ld\r\n"
        "Content-Length: %d\r\n"
        "Connection: Keep-Alive\r\n\r\n",
        CThreadTimer::instance()->getNowSecs(), aSendStr.length());
    INIT_IOV(2);
    SET_IOV_LEN(headerStr.c_str(), headerStr.length());
    SET_IOV_LEN(aSendStr.c_str(), aSendStr.length());
    int sendBufLen = headerStr.length() + aSendStr.length();
    if (iSocket->TcpSendV(aSockId, iovs, 2, iSocketRWTimeOut, true) != sendBufLen) {
        LOG(LL_ERROR, "%s:TcpSend http header failed.", funcName);
        return RET_ERROR;
    }
    DEBUG(LL_ALL, "%s:End", funcName);
    return RET_OK;
}

// ---------------------------------------------------------------------------
// int CHttpHandler::SendHttpAuth()
//
// send http auth to client.
// ---------------------------------------------------------------------------
//
int CHttpHandler::SendHttpAuth(int aSockId, const string& aSendStr) {
    const char *funcName = "CHttpHandler::SendHttpAuth()";
    DEBUG(LL_ALL, "%s:Begin", funcName);
    // Connection is set to Keep-Alive instead of Close to avoid CLOSE_WAIT state.
    string httpAuthStr = "HTTP/1.0 401 Unauthorized\r\n"
        "Cache-Control: no-cache\r\n"
        "Connection: close\r\n"
        "Content-Type: text/html\r\n"
        "Content-Length: 111\r\n"
        "WWW-Authenticate: Basic realm=\""
        AGENT_FULL_VERSION
        "\"\r\n\r\n"
        "<html><body><h1>401 Unauthorized</h1>\n"
        "You need a valid user and password to access this content.\n</body></html>";

    if (iSocket->TcpSend(aSockId, httpAuthStr.c_str(),
        httpAuthStr.length(), iSocketRWTimeOut)
        != httpAuthStr.length()) {
        LOG(LL_ERROR, "%s:TcpSend http header failed.", funcName);
        return RET_ERROR;
    }
    DEBUG(LL_ALL, "%s:End", funcName);
    return RET_OK;
}

// ---------------------------------------------------------------------------
// int CHttpHandler::SendRespBody()
//
// send sync or statistic result to client.
// ---------------------------------------------------------------------------
//
int CHttpHandler::SendRespBody(int aSockId, const string& aSendStr) {
     return (iMsgType[aSockId] == kMsgTypeSyncAll ||\
         iMsgType[aSockId] == kMsgTypeSyncAny ||\
         iMsgType[aSockId] == kMsgTypeGetsAll ||\
         iMsgType[aSockId] == kMsgTypeGetsAny) ?\
         SendSyncBody(aSockId, aSendStr) : \
         (iIsAuthed[aSockId] ?\
         SendStatisticBody(aSockId, aSendStr) :\
         SendHttpAuth(aSockId, aSendStr));
}
// end of local file.
