/*
** Copyright (C) 2014 Wang Yaofu
** All rights reserved.
**
**Author:Wang Yaofu voipman@qq.com
**Description: The header file of class CAgentMonitor.
*/

#ifndef _AGENT_CAGENTMONITOR_H_
#define _AGENT_CAGENTMONITOR_H_

#include "common/cthread.h"
#include "common/csmtpmail.h"
#define MAX_MON_QUEUE_SIZE 1024
#define DEFAULT_EMAIL_FROM     "wangyf5@asiainfo.com"
#define DEFAULT_EMAIL_TO       "wangyf5@asiainfo.com"
#define DEFAULT_EMAIL_SERVER   "mail.asiainfo.com"
#define DEFAULT_EMAIL_PASSWORD "123456"
#define MONITOR_TYPE_EMAIL (0x01)
#define MONITOR_TYPE_SQS   (0x02)

struct SEmailObj {
    string iSubject;
    string iEmailContent;
};
class CConfig;
class CAgentMonitor : public CThread
{
public:
    /*
    * init data when socket start.
    *
    */
    int InitData();

    /*
    * Add request into queue.
    *
    */
    int AddNotifyMsg(string aSubject, string aMsg);

    /*
    * constructor.
    *
    */
    CAgentMonitor();

    /*
    * Destructor.
    *
    */

    ~CAgentMonitor();
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
    * get request from queue and handle it.
    *
    */
    void EmailNotify();

    /*
    * get request from queue and put into sqs queue.
    *
    */
    void PushIntoSQS();

    /*
    * the condition of thread to start or stop.
    *
    */
    bool iStopRequested;

    /*
    * the condition to run.
    *
    */
    CCondition<bool> iRunCondition;

    /*
    * queue for request.
    *
    */
    std::queue<struct SEmailObj> iMonMsgQueue;

    /*
    * the instance of local class.
    *
    */
    struct SEmailInfos iEmailInfos;

    /*
    * mutex of email list.
    *
    */
    CMutex iEmailMutex;

    /*
    * monitor sqs name.
    *
    */
    string iMonSqsName;

    /*
    * monitor socket id.
    *
    */
    int iSocketFd;

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
    * buffer for socket.
    *
    */
    char iRecvBuffer[MAX_MON_QUEUE_SIZE];
};

#endif  // _AGENT_CAGENTMONITOR_H_
