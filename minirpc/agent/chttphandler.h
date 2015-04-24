/*
** Copyright (C) 2014 Wang Yaofu
** All rights reserved.
**
**Author:Wang Yaofu voipman@qq.com
**Description: The header file of class CHttpHandler.
*/

#ifndef _AGENT_CHTTPHANDLER_H_
#define _AGENT_CHTTPHANDLER_H_
#include "common/csocket.h"
#include "common/cthread.h"
#include "common/ev/ceventhandler.h"
#define HTTP_ONLINE_MAX_SIZE 1024
class CEventLoop;
class CConfig;
class CHttpHandler : public CFileEventHandler, public CThread
{
public:
    /*
    * constructor.
    *
    */
    CHttpHandler();

    /*
    * destructor.
    *
    */
    ~CHttpHandler();

    /*
    * Initial operation.
    *
    */
    int InitData();

    /*
    * read event is occurred.
    *
    */
    void Request(int aSockId);

    /*
    * write event is occurred.
    *
    */
    void Response(int aSockId);

    /*
    * error event is occurred.
    *
    */
    void ErrSocket(int aSockId);

private:
    /*
    * start the thread.
    *
    */
    void run();

    /*
    * stop the thread.
    *
    */
    void on_stop();

    /*
    * http header is received.
    *
    */
    int RecvHttpHeader(int aSockId);

    /*
    * http configuration.
    *
    */
    int ParseConfig(int aSockId);

    /*
    * send error message to client.
    *
    */
    int SendErrorBody(int aSockId, int aErrorCode, const char* aErrorStr);

    /*
    * send statistic message to client.
    *
    */
    int SendStatisticBody(int aSockId, const string& aSendStr);

    /*
    * send sync message to client.
    *
    */
    int SendSyncBody(int aSockId, const string& aSendStr);

    /*
    * send sync or statistic message to client.
    *
    */
    int SendRespBody(int aSockId, const string& aSendStr);

    /*
    * send http auth to client.
    *
    */
    int SendHttpAuth(int aSockId, const string& aSendStr);

    /*
    * the condition of thread to start or stop.
    *
    */
    bool iStopRequested;

    /*
    * socket instance pointer to CSocket.
    *
    */
    CSocket *iSocket;

    /*
    * config instance pointer to CConfig.
    *
    */
    CConfig *iConfig;

    /*
    * socket loop pointer to CEventLoop.
    *
    */
    CEventLoop *iEventLoop;

    /*
    * socket address port infos.
    *
    */
    struct SAddrInfo iTcpSerAddrInfo;

    /*
    * css buffer.
    *
    */
    string iCssStr;

    /*
    * get service by sub-page.
    *
    */
    string iServiceNameStr[HTTP_ONLINE_MAX_SIZE+1];

    /*
    * get service sub-page by msg type.
    *
    */
    EMsgType iMsgType[HTTP_ONLINE_MAX_SIZE+1];

    /*
    * http auth flag.
    *
    */
    bool iIsAuthed[HTTP_ONLINE_MAX_SIZE+1];

    /*
    * http auth string.
    *
    */
    string iHttpAuthBase64Str;

    /*
    * socket reading or writing time-out.
    *
    */
    int iSocketRWTimeOut;

    /*
    * socket channel time-out.
    *
    */
    int iSocketTimeOut;

    /*
    * array of socket time out.
    *
    */
    long iSockIdTimer[HTTP_ONLINE_MAX_SIZE+1];
};

#endif  // _AGENT_CHTTPHANDLER_H_
