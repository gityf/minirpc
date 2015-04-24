/*
** Copyright (C) 2014 Wang Yaofu
** All rights reserved.
**
**Author:Wang Yaofu voipman@qq.com
**Description: The header file of class CRpcMsgHandler.
*/

#ifndef _INTERFACE_CBASEMSGHANDLER_H_
#define _INTERFACE_CBASEMSGHANDLER_H_
#include <sstream>
#include "common/cthread.h"
#include "common/csocket.h"
#include "common/cstatisticinfo.h"
#include "common/localdef.h"
#include "common/ev/ceventhandler.h"
#include "rpc_ipserver.h"

class CEventLoop;
class CConfig;
class CProcMaintain;
class CRpcSerObserver;
class CRpcMsgHandler : public CFileEventHandler
{
public:
    /*
    * constructor.
    *
    */
    CRpcMsgHandler();

    /*
    * destructor.
    *
    */
    ~CRpcMsgHandler();

    /*
    * initial operation when local class start.
    *
    */
    int InitData(const SRpcConfig& aConfig);

    /*
    * make interval operation operation for ipc.
    *
    */
    int IntervalUpdater(const string& aIpcMsg = "Living");

    /*
    *  start the tcl handler.
    *
    */
    int Start();

    /*
    * stop the thread.
    *
    */
    void Stop();

    /*
    * RpcServer observer is added.
    *
    */
    void AddRpcObserver(CRpcSerObserver *aRpcObserver);

    /*
    * add socketid into poll.
    *
    */
    int AddSockidIntoPoll(int aSockId);

    /*
    * handle message.
    *
    */
    int HandleMsg(int aSockId);

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

    /*
    * RpcServer send one pkg, the outPks in handler should be increment.
    *
    */
    void IncrOutPks();

    /*
    * RpcServer add vector desc to handler.
    *
    */
    void AddVectorDesc(const string& aVecterDesc);

    /*
    * sending statistic Informations.
    *
    */
    void SendStatisticInfos();

    /*
    * the condition to start or stop.
    *
    */
    bool iStopRequested;

    /*
    * array of socket time out.
    *
    */
    long iSockIdTimer[MAX_SIZE_1K+1];

    /*
    * the buffer for receiving data from client.
    *
    */
    char *iRecvBuffer;

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
    * IPC pointer to CProcMaintain.
    *
    */
    CProcMaintain *iProcMaintain;

    /*
    * RPC observer to rpc_server.
    *
    */
    CRpcSerObserver *iRpcSerObserver;

    /*
    * status when local pid running.
    *
    */
    bool iIsRunningNow;

    /*
    * size of poll fds.
    *
    */
    int iCurFdSize;

    /*
    * seconds before a cmd is run.
    *
    */
    long iHistorySecs;

    /*
    * socket channel time-out.
    *
    */
    int iSocketTimeOut;

    /*
    * socket read write time-out.
    *
    */
    int iSocketRWTimeOut;

    /*
    * statistic Informations.
    *
    */
    CStatisticInfos iStatisticInfos;

    /*
    * socket address port infos for statistic.
    *
    */
    struct SAddrInfo iUdpClntAddrInfo;

    /*
    * the config.
    *
    */
    struct SRpcConfig iRpcConfig;
};

#endif  // _INTERFACE_CBASEMSGHANDLER_H_
