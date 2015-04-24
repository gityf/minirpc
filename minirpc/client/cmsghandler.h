/*
** Copyright (C) 2014 Wang Yaofu
** All rights reserved.
**
**Author:Wang Yaofu voipman@qq.com
**Description: The header file of class CMsgHandler.
*/

#ifndef _CLIENT_CMSGHANDLER_H_
#define _CLIENT_CMSGHANDLER_H_
#include <string>
#include "common/cthread.h"
#include "common/localdef.h"
#define MAX_SESSION_QUEUE_SIZE 1024
#define SERVICE_ITEM_LEN 250
class CSocket;
class CClientSession;
class CConfig;
struct SAddrInfo;
class CMsgHandler {
 public:
    /*
    * init data when socket start.
    *
    */
    int InitData();

    /*
    * configuration service is initialized.
    *
    */
    int ConfigInitial();

    /*
    * execute command.
    *
    */
    int CmdExec(const char *aCmd, const char* aService);

    /*
    * execute command.
    *
    */
    int CmdExecV(const char *aCmd, int aLen, const char *aService);

    int SyncRun(const string& aCmd,
                const string& aService,
                const string& aSubService);

    // kick off service that is not working well.
    void KickoffService(const string& aService, int aSockId);

    /*
    * return result after exec cmd.
    *
    */
    char *GetResult();

    /*
    * return error string.
    *
    */
    const string& GetErrString() const {
        return iErrString;
    }

    /*
    * constructor.
    *
    */
    CMsgHandler();

    /*
    * Destructor.
    *
    */
    ~CMsgHandler();

 private:
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
    * client session instance pointer to CClientSession.
    *
    */
    CClientSession *iClientSession;

    /*
    * socket reading or writing time-out.
    *
    */

    int iSocketRWTimeOut;

    /*
    * the buffer for receiving data from client.
    *
    */
    char *iRecvBuffer;

    /*
    * error string feed back to client.
    *
    */
    string iErrString;

    /*
    * is initializing.
    *
    */
    bool iIsInitialed;
};
#endif  // _CLIENT_CMSGHANDLER_H_
