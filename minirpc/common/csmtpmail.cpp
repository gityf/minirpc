/*
** Copyright (C) 2014 Wang Yaofu
** All rights reserved.
**
**Author:Wang Yaofu voipman@qq.com
**Description: The source file of class CSmtpMail.
*/
#include <unistd.h>
#include <netdb.h>
#include <sys/types.h>
#include "common/csmtpmail.h"
#include "common/localdef.h"
#include "common/csocket.h"
#include "common/clogwriter.h"
#include "common/cconfig.h"
#include "common/base64.h"

// ---------------------------------------------------------------------------
// CSmtpMail::CSmtpMail()
//
// constructor
// ---------------------------------------------------------------------------
//
CSmtpMail::CSmtpMail() {
    iSocket = NULL;
    iIsInitialed = false;
}

// ---------------------------------------------------------------------------
// CSmtpMail::~CSmtpMail()
//
// destructor.
// ---------------------------------------------------------------------------
//
CSmtpMail::~CSmtpMail() {
    if (iSocket != NULL) {
        delete iSocket;
        iSocket = NULL;
    }
}

// ---------------------------------------------------------------------------
// int CSmtpMail::InitData()
//
// init operation.
// ---------------------------------------------------------------------------
//
int CSmtpMail::InitData() {
    if (!iSocket) {
        iSocket = new CSocket();
    }
    iIsInitialed = true;
    iSocketTimeOut = 5;
    iCurSockId = 0;
    return RET_OK;
}

// ---------------------------------------------------------------------------
// void CSmtpMail::SendMail(struct SEmailInfos aMailIffo)
//
// send email.
// ---------------------------------------------------------------------------
//
void CSmtpMail::SendMail(struct SEmailInfos& aMailIffo) {
    const char* funcName = "CSmtpMail::SendMail";
    if (!iIsInitialed) {
        InitData();
    }

    if (aMailIffo.iEmailServer.size() <= 0) {
        aMailIffo.iEmailServer = "mail.AsiaInfo.com";
    }
    char emailBuffer[MAX_SIZE_2K*2] = {0};
    if (!iSocket->Name2IPv4(aMailIffo.iEmailServer.c_str(), emailBuffer)) {
        LOG(LL_ERROR, "%s:bad email server name.", funcName);
        return;
    }
    LOG(LL_VARS, "IP of %s is : %s\n",
        aMailIffo.iEmailServer.c_str(), emailBuffer);

    struct SAddrInfo addrInfo;
    addrInfo.iAddr = emailBuffer;
    addrInfo.iPort = 25;
    addrInfo.iProtocol = TCP;
    // receiving welcome informations.
    int tryTime = 3;
    int ret = 0;
    do {
        if (RET_ERROR == iSocket->CreateSocket(&addrInfo)) {
            LOG(LL_ERROR, "%s:CreateSocket error.", funcName);
            return;
        }
        if (RET_ERROR == iSocket->TcpConnect(addrInfo)) {
            LOG(LL_ERROR, "%s:TcpConnect error.", funcName);
            return;
        }
        ret = iSocket->SelectSockId(addrInfo.iSockId, 5, FD_READABLE);
        if (ret & FD_READABLE) {
            ret = iSocket->TcpRecvOnce(addrInfo.iSockId, emailBuffer, MAX_SIZE_2K);
        }
    } while (ret == RET_ERROR && tryTime-- > 0);
    iCurSockId = addrInfo.iSockId;
    emailBuffer[ret] = 0x00;
    LOG(LL_INFO, "%s:Welcome Informations:\n%s", funcName, emailBuffer);

    // EHLO
    string tmpOnlyUser;
    if (aMailIffo.iUserName.find_last_of('@') > 0) {
        tmpOnlyUser = aMailIffo.iUserName.substr(0, aMailIffo.iUserName.find_last_of('@'));
    } else {
        tmpOnlyUser = aMailIffo.iUserName;
    }
    sprintf(emailBuffer, "EHLO %s\r\n", tmpOnlyUser.c_str());
    if (RET_ERROR == MailSendRecv(emailBuffer)) {
        LOG(LL_ERROR, "%s:MailSendRecv EHLO error.", funcName);
        return;
    }
    LOG(LL_INFO, "%s:EHLO REceive:%s", funcName, emailBuffer);

    // AUTH LOGIN
    sprintf(emailBuffer, "AUTH LOGIN\r\n");
    if (RET_ERROR == MailSendRecv(emailBuffer)) {
        LOG(LL_ERROR, "%s:MailSendRecv AUTH LOGIN error.", funcName);
        return;
    }
    LOG(LL_INFO, "%s:Auth Login Receive:%s", funcName, emailBuffer);

    // USER
    memset(emailBuffer, 0, sizeof(emailBuffer));
    size_t tmplen = 0;
    Base64Encode(aMailIffo.iUserName, emailBuffer, &tmplen);
    strcat(emailBuffer, "\r\n");
    LOG(LL_INFO, "%s:Base64 UserName:%s", funcName, emailBuffer);
    if (RET_ERROR == MailSendRecv(emailBuffer)) {
        LOG(LL_ERROR, "%s:MailSendRecv USER error.", funcName);
        return;
    }
    LOG(LL_INFO, "%s:User Login Receive:%s", funcName, emailBuffer);

    // PASSWORD
    memset(emailBuffer, 0, sizeof(emailBuffer));
    Base64Encode(aMailIffo.iUserPass, emailBuffer, &tmplen);
    strcat(emailBuffer, "\r\n");
    LOG(LL_INFO, "%s:Base64 Password:%s", funcName, emailBuffer);
    if (RET_ERROR == MailSendRecv(emailBuffer)) {
        LOG(LL_ERROR, "%s:MailSendRecv PASSWORD error.", funcName);
        return;
    }
    LOG(LL_INFO, "%s:Send Password Receive:%s", funcName, emailBuffer);

    // MAIL FROM
    sprintf(emailBuffer, "MAIL FROM: <%s>\r\n", aMailIffo.iUserName.c_str());
    if (RET_ERROR == MailSendRecv(emailBuffer)) {
        LOG(LL_ERROR, "%s:MailSendRecv MAIL FROM error.", funcName);
        return;
    }
    LOG(LL_INFO, "%s:set Mail From Receive:%s", funcName, emailBuffer);

    // RCPT TO
    int ii = 0, pos = 0, offset = 0;
    while (ii < aMailIffo.iEmailTo.length()) {
        if (aMailIffo.iEmailTo.at(ii) == ',') {
            offset = sprintf(emailBuffer+offset,
                "RCPT TO:<%s>\r\n",
                aMailIffo.iEmailTo.substr(pos, ii - pos).c_str());
            pos = 1 + ii;
        }
        ii++;
    }
    if (ii > pos) {
        offset = sprintf(emailBuffer+offset,
            "RCPT TO:<%s>\r\n",
            aMailIffo.iEmailTo.substr(pos, ii - pos).c_str());
    }

    if (RET_ERROR == MailSendRecv(emailBuffer)) {
        LOG(LL_ERROR, "%s:MailSendRecv RCPT TO error.", funcName);
        return;
    }
    LOG(LL_INFO, "%s:Tell Sendto Receive:%s", funcName, emailBuffer);

    // DATA
    sprintf(emailBuffer, "DATA\r\n");
    if (RET_ERROR == MailSendRecv(emailBuffer)) {
        LOG(LL_ERROR, "%s:MailSendRecv DATA error.", funcName);
        return;
    }
    LOG(LL_INFO, "%s:Send Mail Prepare Receive:%s", funcName, emailBuffer);

    // CONTENTS
    snprintf(emailBuffer,
        MAX_SIZE_2K,
        "From:%s\r\n"
        "To:%s\r\n"
        "Subject:%s\r\n"
        "Content-Type: text/plain;"
        "\r\n\r\n%s\r\n.\r\n",
        aMailIffo.iUserName.c_str(),
        aMailIffo.iEmailTo.c_str(),
        aMailIffo.iSubJect.c_str(),
        aMailIffo.iEmailContent.c_str());
    if (RET_ERROR == MailSendRecv(emailBuffer)) {
        LOG(LL_ERROR, "%s:MailSendRecv CONTENTS error.", funcName);
        return;
    }
    LOG(LL_INFO, "%s:Send Mail Receive:%s", funcName, emailBuffer);

    // QUIT
    sprintf(emailBuffer, "QUIT\r\n");
    if (RET_ERROR == MailSendRecv(emailBuffer)) {
        LOG(LL_ERROR, "%s:MailSendRecv QUIT error.", funcName);
        return;
    }
    LOG(LL_INFO, "%s:Quit Receive:%s", funcName, emailBuffer);

    // clean
    close(addrInfo.iSockId);
    return;
}

// ---------------------------------------------------------------------------
// int CSmtpMail::MailSendRecv(string aMsg)
//
// send email info.
// ---------------------------------------------------------------------------
//
int CSmtpMail::MailSendRecv(char *aMsg) {
    int bufLen = strlen(aMsg);
    if (bufLen != iSocket->TcpSend(iCurSockId, aMsg, bufLen, iSocketTimeOut)) {
        LOG(LL_ERROR, "MailSendRecv:TcpSend %s error.", aMsg);
        return RET_ERROR;
    }
    int ret = iSocket->SelectSockId(iCurSockId, iSocketTimeOut, FD_READABLE);
    if (ret & FD_READABLE) {
        ret = iSocket->TcpRecvOnce(iCurSockId, aMsg, MAX_SIZE_2K);
        if (ret > 0) {
            aMsg[ret] = 0x00;
        }
    }
    return ret;
}
// end of local file.
