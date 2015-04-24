/*
** Copyright (C) 2014 Wang Yaofu
** All rights reserved.
**
**Author:Wang Yaofu voipman@qq.com
**Description: The source file of class CRpcMsgHandler.
*/
#include <iostream>
#include "common/cconfig.h"
#include "common/cfilesys.h"
#include "common/clogwriter.h"
#include "common/csetproctitle.h"
#include "common/csingleton.h"
#include "common/cstringutils.h"
#include "common/csocket.h"
#include "common/cdaemon.h"
#include "common/dl/library.h"
#include "common/rpc/rpc_ipserver.h"
#include "common/rpc/rpc_server.h"
#include "common/rpc/version.h"

RpcServer::RpcServer() {
    iRpcIpServer = NULL;
    iIsInitSuccess = false;
    iRpcMethodMap.clear();
    iLibraryMap.clear();
    iLibMethodMap.clear();
}

RpcServer::~RpcServer() {
    if (iRpcIpServer != NULL) {
        delete iRpcIpServer;
    }
}

int RpcServer::InitData(int argc, char *argv[]) {
    spt_init(argc, argv);
    iIsInitSuccess = false;
    bool isRunDeamon = false;
    struct SRpcConfig configInfo;
    configInfo.iHandlerCounts = 5;
    configInfo.iListenPort = 0;
    string configFile = "./rpcserver.ini";
    int ch = 0;
    while ((ch = getopt(argc, argv, "p:l:c:s:dvh")) != -1) {
        switch (ch) {
        case 'p':
            {
                if (optarg) {
                    configInfo.iListenPort = atoi(optarg);
                }
            }
            break;
        case 's':
            {
                if (optarg) {
                    iServerName = optarg;
                }
            }
            break;
        case 'l':
            {
                if (optarg) {
                    configInfo.iLogLevel = atoi(optarg);
                    configInfo.iConfigFlag |= 1;
                    CSingleton<CLogWriter>::Instance()->SetLogLevel(
                        configInfo.iLogLevel);
                }
            }
            break;
        case 'c':
            {
                if (optarg) {
                    configFile = optarg;
                }
            }
            break;
        case 'd':
            {
                isRunDeamon = true;
            }
            break;
        default:
            {
                if (ch == 'v' || (optarg && strcmp(optarg, "-v") == 0)) {
                    fprintf(stderr, "%s version: %s, %s\n",
                        argv[0], FullVersion, CompileInfo);
                } else if (ch == 'h' || (optarg && strcmp(optarg, "-h") == 0)) {
                    fprintf(stderr, "%s\n", HelpMsg);
                } else {
                    fprintf(stderr, "\t%s -h for help.\n", RPC_SERVER_NAME);
                }
                fflush(stderr);
                fflush(stdout);
                _exit(0);
            }
            break;
        }
    }
    if (configInfo.iListenPort == 0) {
        fprintf(stderr, "%s\n", HelpMsg);
        return RET_ERROR;
    }
    // run by daemon mode.
    if (isRunDeamon) {
        wyf::DaemonMode();
    }
    setproctitle("%s:Master.%d", argv[0], configInfo.iListenPort);
    if (iRpcIpServer == NULL) {
        iRpcIpServer = new CRpcIpServer();
    }
    iRpcIpServer->IngoreSignal();
    configInfo.iServerName = iServerName;
    CSingleton<CConfig>::Instance()->InitConfig(configFile);
    iRpcIpServer->AddRpcObserver(this);
    if (iRpcIpServer->InitData(configInfo) == RET_ERROR) {
        return RET_ERROR;
    }
    iIsInitSuccess = true;
    return RET_OK;
}

bool RpcServer::Start() {
    if (!iIsInitSuccess) {
        return false;
    }
    iRpcIpServer->MainStart();
    return true;
}

int RpcServer::RpcRequest(int aSockId, const string& aMethod,
                          const char* aRequBuf, int aRequLen) {
    if (aRequBuf == NULL) {
        return RET_ERROR;
    }
    typeof(iRpcMethodMap.end()) it = iRpcMethodMap.find(aMethod);
    if (it != iRpcMethodMap.end()) {
        RpcMethod *rpc_method = it->second;
        const ::google::protobuf::MethodDescriptor *method = rpc_method->method_;
        ::google::protobuf::Message *request = rpc_method->request_->New();
        ::google::protobuf::Message *response = rpc_method->response_->New();
        if (!request->ParseFromArray(aRequBuf, aRequLen)) {
            delete request;
            delete response;
            LOG(LL_ERROR, "%s decode error.", aMethod.c_str());
            return RET_ERROR;
        }
        HandleServiceEntry *entry = new HandleServiceEntry(method,
            request,
            response, aSockId);
        ::google::protobuf::Closure *done =
            ::google::protobuf::NewCallback(this,
            &RpcServer::HandleServiceDone,
            entry);
        rpc_method->service_->CallMethod(method, NULL, request, response, done);
    }
    return RET_OK;
}

int RpcServer::LoadService(const string& aLibName) {
    if (aLibName.empty()) {
        return RET_ERROR;
    }
    DEBUG(LL_WARN, "LoadService %s.", aLibName.c_str());
    if (iLibraryMap.find(aLibName) != iLibraryMap.end()) {
        // library has loaded as before.
        return RET_OK;
    }
    string curLibNameBak = iCurLibName;
    try {
        Library *lib = new Library(aLibName);
        string loadlibEntry = wyf::CFileSys::OnlyLibName(aLibName);
        loadlibEntry += "_Init";
        DEBUG(LL_WARN, "LoadService Entry %s.", loadlibEntry.c_str());
        FuncPtr_LibEntry entryFunc =
            reinterpret_cast<FuncPtr_LibEntry>(lib->resolve(loadlibEntry.c_str()));
        DEBUG(LL_WARN, "LoadService Entry addr:%p.", loadlibEntry.c_str());
        if (entryFunc == NULL) {
            LOG(LL_ERROR, "library %s without %s entry function.",
                aLibName.c_str(), loadlibEntry.c_str());
            return RET_ERROR;
        }
        iCurLibName = aLibName;
        DEBUG(LL_WARN, "LoadService Begin Entry %s.", loadlibEntry.c_str());
        if (entryFunc(this) == RET_ERROR) {
            LOG(LL_ERROR, "library %s entry function call error.",
                aLibName.c_str());
            return RET_ERROR;
        } 
        DEBUG(LL_WARN, "LoadService End Entry %s.", loadlibEntry.c_str());
        iLibraryMap[aLibName] = lib;
    } catch (const std::string& e) {
        LOG(LL_ERROR, "LoadService %s error %s.", aLibName.c_str(), e.c_str());
        iCurLibName = curLibNameBak;
        return RET_ERROR;
    } catch (...) {
        LOG(LL_ERROR, "LoadService %s unknown error.", aLibName.c_str());
        iCurLibName = curLibNameBak;
        return RET_ERROR;
    }
    iRpcIpServer->AddVectorDesc(DumpAllLibNames());
    return RET_OK;
}

int RpcServer::UnLoadService(const string& aLibName) {
    if (aLibName.empty()) {
        return RET_ERROR;
    }
    // unload service is deny, the action is good or not?
    return RET_OK;
    typeof(iLibraryMap.end()) it = iLibraryMap.find(aLibName);
    if (it != iLibraryMap.end()) {
        // library name -- Library class.
        Library *lib = it->second;
        if (lib != NULL) {
            delete lib;
            lib = NULL;
        }
        string libname = it->first;
        // library name -- method full name list in service.
        typeof(iLibMethodMap.end()) mapiter = iLibMethodMap.find(libname);
        if (mapiter != iLibMethodMap.end()) {            
            for (typeof(mapiter->second.end()) listiter = mapiter->second.begin();
                listiter != mapiter->second.end();
                ++listiter) {
                // erase all the method full name from RPC method map.
                // listiter is method full name.
                DelService(*listiter);
            }
            // erase library method map.
            iLibMethodMap.erase(mapiter);
        }
        // erase Library class by library name.
        iLibraryMap.erase(it); 
    } else {
	return RET_OK;
    }    
    iRpcIpServer->AddVectorDesc(DumpAllLibNames());
    return RET_OK;
}

int RpcServer::ReLoadService(const string& aLibList) {
    std::vector<string> libList;
    // libx.so+liby.so+...
    wyf::CStrUitls::SplitString(aLibList, "+", &libList);
    if (libList.size() > 0) {
        for (int ii = 0; ii < libList.size(); ++ii) {
	    LoadService(libList[ii]);
        }        
    }
    return RET_OK;
}

bool RpcServer::AddService(::google::protobuf::Service *service) {
    DEBUG(LL_WARN, "AddService %p.", service);
    const ::google::protobuf::ServiceDescriptor *descriptor =
       service->GetDescriptor();
    DEBUG(LL_WARN, "AddService count:%d.", descriptor->method_count());
    for (int i = 0; i < descriptor->method_count(); ++i) {
        const ::google::protobuf::MethodDescriptor *method =
            descriptor->method(i);
        const google::protobuf::Message *request =
            &service->GetRequestPrototype(method);
        const google::protobuf::Message *response =
            &service->GetResponsePrototype(method);
        RpcMethod *rpc_method = new RpcMethod(service, request,
                                              response, method);
        std::string methodName = method->full_name();
        DEBUG(LL_WARN, "AddService fullname:%s.", methodName.c_str());
        iRpcMethodMap[methodName] = rpc_method;
        iLibMethodMap[iCurLibName].push_back(methodName);
    }
    DEBUG(LL_WARN, "AddService End");
    return true;
}

bool RpcServer::DelService(::google::protobuf::Service *service) {
    const ::google::protobuf::ServiceDescriptor *descriptor =
        service->GetDescriptor();
    for (int i = 0; i < descriptor->method_count(); ++i) {
        const ::google::protobuf::MethodDescriptor *method =
            descriptor->method(i);
        std::string methodName = method->full_name();
        typeof(iRpcMethodMap.end()) it = iRpcMethodMap.find(methodName);
        if (it != iRpcMethodMap.end()) {
            RpcMethod *rpc_method = it->second;
            if (rpc_method != NULL) {
                if (rpc_method->service_ != NULL) {
                    delete rpc_method->service_;
                }                
                delete rpc_method;
            }
            iRpcMethodMap.erase(it);
        }
    }
    return true;
}

bool RpcServer::DelService(const string& aMethodFullName) {
    typeof(iRpcMethodMap.end()) it = iRpcMethodMap.find(aMethodFullName);
    if (it != iRpcMethodMap.end()) {
        RpcMethod *rpc_method = it->second;
        if (rpc_method != NULL) {
            /*
            if (rpc_method->service_ != NULL) {
                delete rpc_method->service_;
            } */ 
            delete rpc_method;
        }
        iRpcMethodMap.erase(it);
    }
    return true;
}

void RpcServer::HandleServiceDone(HandleServiceEntry *entry) {
    std::string content;
    entry->response_->SerializeToString(&content);
    char header[10] = {0};
    CSingleton<CSocket>::Instance()->CreatePkgHeader(content.length(), header);
    INIT_IOV(2);
    SET_IOV_LEN(header, PKG_HEADER_LEN);
    SET_IOV_LEN(content.c_str(), content.length());
    CSingleton<CSocket>::Instance()->TcpSendV(entry->iSockId, iovs, 2, 2, true);
    delete entry->request_;
    delete entry->response_;
    delete entry;
    iRpcIpServer->IncrOutPks();
}

string RpcServer::DumpAllLibNames() {
    string libNames = "";
    for (typeof(iLibraryMap.end()) it = iLibraryMap.begin();
        it != iLibraryMap.end();
        ++it) {
        libNames += "+";
        libNames += it->first;
    }
    return libNames;    
}

string RpcServer::DumpAllMethodsInLib(const string& aLibName) {
    string allMethods = "";
    if (iLibMethodMap.find(aLibName) != iLibMethodMap.end()) {
        std::list<string >::iterator it = iLibMethodMap[aLibName].begin();
        for (; it != iLibMethodMap[aLibName].end(); ++it) {
            allMethods += " ";
            allMethods += *it;
        }
    }
    return allMethods;
}
// end of local file.
