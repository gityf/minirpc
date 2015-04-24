/*
** Copyright (C) 2014 Wang Yaofu
** All rights reserved.
**
**Description: The header file of class RpcClient.
*/
#ifndef _COMMON_RPC_RPC_CLIENT_H
#define _COMMON_RPC_RPC_CLIENT_H
#pragma once
#include <string>
#include <google/protobuf/descriptor.h>
#include <google/protobuf/service.h>
#include <google/protobuf/stubs/common.h>
#include <google/protobuf/generated_message_util.h>
#include <google/protobuf/repeated_field.h>
#include <google/protobuf/extension_set.h>
#include <google/protobuf/generated_message_reflection.h>

using std::string;
class CMsgHandler;

class RpcClient : public ::google::protobuf::RpcChannel {
public:
    explicit RpcClient(const string& aServer, int aTimeout = 3);

    ~RpcClient();

    int InitData();

    void CallMethod(const ::google::protobuf::MethodDescriptor* method,
                          ::google::protobuf::RpcController* controller,
                    const ::google::protobuf::Message* request,
                          ::google::protobuf::Message* response,
                          ::google::protobuf::Closure* done);
private:
    int iTimeOutSecs;
    // server name to find host server.
    string iServer;
    CMsgHandler *iMsgHandler;
};

#endif // _COMMON_RPC_RPC_CLIENT_H
