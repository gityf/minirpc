/*
** Copyright (C) 2014 Wang Yaofu
** All rights reserved.
**
**Author:Wang Yaofu voipman@qq.com
**Description: The header file of class CRpcIpServer.
*/

#ifndef _INTERFACE_CIP_SERVER_H_
#define _INTERFACE_CIP_SERVER_H_
#include "common/localdef.h"
#include "common/csocket.h"
#include "common/cthread.h"

struct SRpcConfig {
    /*
    * tcl handler counts.
    *
    */
    int iHandlerCounts;

    /*
    * local host ip address.
    *
    */
    string iLocalHostIP;

    // server name
    string iServerName;

    /*
    * listen port of IPServer.
    *
    */
    int iListenPort;

    /*
    * log level.
    *
    */
    int iLogLevel;

    /*
    * log file name.
    *
    */
    string iLogFileName;

    /*
    * read config from cmd line or cfg file.
    *
    */
    unsigned int iConfigFlag;

    /*
    * main pid
    *
    */
    int iMainPid;
};
class CConfig;
class CRpcMsgHandler;
class CProcMaintain;
class CRpcSerObserver;
class CRpcIpServer : public CThread
{
public:
    /*
    * init the IPServer.
    *
    */
    int InitData(struct SRpcConfig aIpSerConfig);

    // ingnore all the signals.
    void IngoreSignal();

    /*
    * configuration service is initialized.
    *
    */
    int ConfigInitial();

    /*
    * create sub worker
    *
    */
    void ForkWorker();

    /*
    * the worker counts is updated.
    *
    */
    int UpdateConns();

    /*
    * start the IPServer.
    *
    */
    int MainStart();

    /*
    * start the worker thread for socket accept.
    *
    */
    int WorkerAcceptStart();

    /*
    * start the worker process/thread for working.
    *
    */
    int WorkerStart();

    /*
    * RpcServer observer is added.
    *
    */
    void AddRpcObserver(CRpcSerObserver *aRpcObserver);

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
    * send signal to child pid for script updating.
    *
    */
    int SignalToChildForUpdateScript();

    /*
    * constructor.
    *
    */
    CRpcIpServer();

    /*
    * destructor.
    *
    */
    ~CRpcIpServer();
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
    * message handler instance pointer to CSerMsgHandler.
    *
    */
    CRpcMsgHandler *iRpcMsgHandler;

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
    * socket address port infos.
    *
    */
    struct SAddrInfo iTcpSerAddrInfo;

    /*
    * the counts should be forked.
    *
    */
    int iNeedForkCounts;

    /*
    * the main pid.
    *
    */
    int iMainPid;

    /*
    * the config.
    *
    */
    struct SRpcConfig iRpcConfig;

    /*
    * seconds for update script.
    *
    */
    long iLastUpdateScriptSecs;
    // proc title is set success?
    bool iIsSetProcTitleSucc;
};
#endif  // _INTERFACE_CIP_SERVER_H_
