/*
** Copyright (C) 2014 Wang Yaofu
** All rights reserved.
**
**Description: The source file of class RpcClient.
*/
#include "common/rpc/rpc_client.h"
#include "client/cmsghandler.h"
RpcClient::RpcClient(const string& aServer, int aTimeout) {
    iServer = aServer;
    iTimeOutSecs = aTimeout;
    iMsgHandler = NULL;
}

RpcClient::~RpcClient() {
    if (iMsgHandler != NULL) {
        delete iMsgHandler;
    }
}

int RpcClient::InitData() {
    if (iMsgHandler == NULL) {
        iMsgHandler = new CMsgHandler();
    }
    return iMsgHandler->InitData();
}

void RpcClient::CallMethod(const ::google::protobuf::MethodDescriptor* method,
                           ::google::protobuf::RpcController* controller,
                           const ::google::protobuf::Message* request,
                           ::google::protobuf::Message* response,
                           ::google::protobuf::Closure* done) {
    std::string content;
    std::string methodName = method->full_name();
    int tryTimes = 3, ret = RET_ERROR;
    request->SerializeToString(&content);
    do {
        ret = iMsgHandler->SyncRun(content, iServer, methodName);
    } while (ret == RET_ERROR && tryTimes-- > 0);
    if (ret > 0) {
        if (!response->ParseFromArray(iMsgHandler->GetResult(), ret)) {
            // log here
        }
    }
    if (done != NULL) {
        done->Run();
    }
}
// end of local file.

