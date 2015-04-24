/*
** Copyright (C) 2014 Wang Yaofu
** All rights reserved.
**
**Author:Wang Yaofu voipman@qq.com
**Description: The header file of class CFileEventHandler.
*/
#ifndef _COMMON_RPC_SERVER_OBSERVER_H
#define _COMMON_RPC_SERVER_OBSERVER_H
#include <string>
#include <google/protobuf/descriptor.h>
#include <google/protobuf/service.h>
#include <google/protobuf/stubs/common.h>
#include <google/protobuf/generated_message_util.h>
#include <google/protobuf/repeated_field.h>
#include <google/protobuf/extension_set.h>
#include <google/protobuf/generated_message_reflection.h>
using std::string;

#define RPCREGI(x, y)                                \
    ::google::protobuf::Service *service = new y();  \
    x->AddService(service);

#define RPCREGI_BASIC(x, y, z)                        \
    ::google::protobuf::Service *service = new y(z); \
    x.AddService(service);

class CRpcSerObserver {
 public:
     CRpcSerObserver() {}
     ~CRpcSerObserver() {}
    // rpc request.
    virtual int RpcRequest(int aSockId, const string& aMethod,
                           const char* aRequBuf, int aRequLen) = 0;
    // load library and add service.
    virtual int ReLoadService(const string& aLibList) = 0;
    // add pb service into the rpc server.
    virtual bool AddService(::google::protobuf::Service *service) = 0;
    // delete pb service from the rpc server.
    virtual bool DelService(::google::protobuf::Service *service) = 0;
};

// library must have one extern 'C' function named 
typedef int (*FuncPtr_LibEntry)(CRpcSerObserver *aRpcServer);
#endif  // _COMMON_RPC_SERVER_OBSERVER_H
