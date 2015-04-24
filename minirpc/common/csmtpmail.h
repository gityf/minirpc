/*
** Copyright (C) 2014 Wang Yaofu
** All rights reserved.
**
**Author:Wang Yaofu voipman@qq.com
**Description: The header file of class CSmtpMail.
*/
#ifndef _COMMON_SMTP_MAIL_H_
#define _COMMON_SMTP_MAIL_H_
#include <string>
using std::string;
struct SEmailInfos{
    string iEmailServer;
    string iEmailTo;
    string iUserName;
    string iSubJect;
    string iUserPass;
    string iEmailContent;
};
class CSocket;
class CSmtpMail
{
public:
    /*
    * Initial operation.
    *
    */
    int InitData();

    /*
    * send email.
    *
    */
    void SendMail(struct SEmailInfos& aMailIffo);

    /*
    * constructor.
    *
    */
    CSmtpMail();

    /*
    * destructor.
    *
    */
    ~CSmtpMail();

    /*
    * communication with mail server.
    *
    */
    int MailSendRecv(char *aMsg);

private:
    /*
    * socket instance pointer to CSocket.
    *
    */
    CSocket *iSocket;

    /*
    * is initializing.
    *
    */
    bool iIsInitialed;

    /*
    * current socket id.
    *
    */
    int iCurSockId;

    /*
    * socket timeout.
    *
    */
    int iSocketTimeOut;
};

#endif // _COMMON_SMTP_MAIL_H_
