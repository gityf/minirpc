/*
** Copyright (C) 2014 Wang Yaofu
** All rights reserved.
**
**Author:Wang Yaofu voipman@qq.com
**Description: The header file of class CRpcServer.
*/
#ifndef _COMMON_RPC_RPC_SERVER_H_
#define _COMMON_RPC_RPC_SERVER_H_
#include <list>
#include <map>
#include "common/dl/library.h"
using wyf::Library;
using wyf::Symbol;
#include "common/rpc/rpc_serverobserver.h"

struct RpcMethod {
public:
    RpcMethod(::google::protobuf::Service *service,
              const ::google::protobuf::Message *request,
              const ::google::protobuf::Message *response,
              const ::google::protobuf::MethodDescriptor *method)
        : service_(service),
          request_(request),
          response_(response),
          method_(method) {
          }

    ::google::protobuf::Service *service_;
    const ::google::protobuf::Message *request_;
    const ::google::protobuf::Message *response_;
    const ::google::protobuf::MethodDescriptor *method_;
};

struct HandleServiceEntry {
    HandleServiceEntry(const ::google::protobuf::MethodDescriptor *method,
                       ::google::protobuf::Message *request,
                       ::google::protobuf::Message *response, int aSockId)
        : method_(method),
          request_(request),
          response_(response),
          iSockId(aSockId) {
          }
    const ::google::protobuf::MethodDescriptor *method_;
    ::google::protobuf::Message *request_;
    ::google::protobuf::Message *response_;
    int iSockId;
};
class CRpcIpServer;
class RpcServer : public CRpcSerObserver {
public:
    RpcServer();
    explicit RpcServer(const string& aServerName) {
        iServerName = aServerName;
        iRpcIpServer = NULL;
        iIsInitSuccess = false;
        iRpcMethodMap.clear();
        iLibraryMap.clear();
        iLibMethodMap.clear();
        iCurLibName = "Basic";
    }
    ~RpcServer();
    int InitData(int argc, char *argv[]);
    // start the rpc server.
    bool Start();
    // rpc request.
    int RpcRequest(int aSockId, const string& aMethod,
                   const char* aRequBuf, int aRequLen);
    int LoadService(const string& aLibName);
    int UnLoadService(const string& aLibName);
    int ReLoadService(const string& aLibList);
    // add pb service into the rpc server.
    bool AddService(::google::protobuf::Service *service);
    // delete pb service from the rpc server.
    bool DelService(::google::protobuf::Service *service);
    // delete pb service by method full name from the rpc server.
    bool DelService(const string& aMethodFullName);
    // closure for request done.
    void HandleServiceDone(HandleServiceEntry *entry);
    // generate library list string desc.
    string DumpAllLibNames();
    // dump all the method full name in library loaded.
    string DumpAllMethodsInLib(const string& aLibName);
private:
    std::map<std::string, RpcMethod*> iRpcMethodMap;
    // library name Library pair.
    std::map<std::string, Library*> iLibraryMap;
    // service in library.
    std::map<std::string, std::list<std::string> > iLibMethodMap;
    // current library name.
    string iCurLibName;
    // server name to public host server.
    string iServerName;
    // pointer to rpc ip server.
    CRpcIpServer *iRpcIpServer;
    // is initialized.
    bool iIsInitSuccess;
};
#endif // _COMMON_RPC_RPC_SERVER_H_
