/*
** Copyright (C) 2014 Wang Yaofu
** All rights reserved.
**
**Description: The source file of class RpcClient echo demo test.
*/

#include "rpc_client.h"
#include "pbext.h"
#include "echo.pb.h"
#include "basic.pb.h"
#include "common/tinythread.h"
#include "common/this_thread.h"
#include "common/cstringutils.h"
#include <string>
#include <iostream>
using namespace std;

class Test {
public:
    void echo_done(echo::EchoRequest* request,
                   echo::EchoResponse* response,
                   ::google::protobuf::Closure* done) {
        std::string res = response->response();
        if (!res.empty())
            //cout << "sync resp: " << response->response() << endl;
        //cout << "sync requ: " << request->message() << endl;
        if (done) {
            done->Run();
        }
        delete request;
        delete response;
        //std::cout << "success\n";
    }
    void echo_done(echo::DummyRequest* request,
        echo::DummyResponse* response,
        ::google::protobuf::Closure* done) {
          //  cout << "Dummy sync requ: " << request->message() << endl;
            if (done) {
                done->Run();
            }
            delete request;
            delete response;
         //   std::cout << "success\n";
    }
    void libload_done(basic::LibLoadRequest* request,
                      basic::LibLoadResponse* response,
                   ::google::protobuf::Closure* done) {
        std::string res = response->loadresult();
        if (!res.empty())
            cout << "load resp: " << response->loadresult() << endl;
        cout << "load requ: " << request->library() << endl;
        if (done) {
            done->Run();
        }
        delete request;
        delete response;
        std::cout << "success\n";
    }
    void libunload_done(basic::LibLoadRequest* request,
        basic::LibLoadResponse* response,
        ::google::protobuf::Closure* done) {
            std::string res = response->loadresult();
            if (!res.empty())
                cout << "unload resp: " << response->loadresult() << endl;
            cout << "unload requ: " << request->library() << endl;
            if (done) {
                done->Run();
            }
            delete request;
            delete response;
            std::cout << "success\n";
    }
};

void ThreadRpc()
{
    Test test;
    ::google::protobuf::RpcChannel* client = new RpcClient("BasicServer");
    echo::EchoService_Stub::Stub* stub;
    if (!((RpcClient*)client)->InitData() == -1) {
        delete client;
        exit(0);
    } else {
        std::cout << "create success\n";
    }
    stub = new echo::EchoService_Stub::Stub(client);
    string ptid = wyf::CStrUitls::ToStringT(ThisThread::GetThreadId());
    string echostr = ptid + "Thread client_hello.";
    string dummystr = ptid + "Thread client_hello.dummy.";

    while (1) {
        echo::EchoRequest* request = new echo::EchoRequest();
        request->set_message(echostr);
        echo::EchoResponse* response = new echo::EchoResponse();
        ::google::protobuf::Closure* callback_callback = NULL;
        ::google::protobuf::Closure *callback = pbext::NewCallback(&test,
            &Test::echo_done,
            request,
            response,
            callback_callback);
        stub->Echo(NULL, request, response, callback);
        echo::DummyRequest* dRequ = new echo::DummyRequest();
        dRequ->set_message(dummystr);
        echo::DummyResponse *dResp = new echo::DummyResponse();
        ::google::protobuf::Closure *dcallback = pbext::NewCallback(&test,
            &Test::echo_done,
            dRequ,
            dResp,
            callback_callback);
        stub->Dummy(NULL, dRequ, dResp, dcallback);
    }
    delete client;
    delete stub;
}

int main(int argc, char *argv[]) {
    Test test;
    ::google::protobuf::RpcChannel* client = new RpcClient("BasicServer");
    echo::EchoService_Stub::Stub* stub;
    basic::LibLoadService_Stub::Stub* libloadstub;
    if (!((RpcClient*)client)->InitData() == -1) {
        delete client;
        exit(0);
    } else {
        std::cout << "create success\n";
    }
    libloadstub = new basic::LibLoadService_Stub::Stub(client);
    basic::LibLoadRequest* loadrequest = new basic::LibLoadRequest();
    loadrequest->set_library("libecho.so");
    basic::LibLoadResponse* loadresponse = new basic::LibLoadResponse();
    ::google::protobuf::Closure* callback_callback = NULL;
    ::google::protobuf::Closure *loadcallback = pbext::NewCallback(&test,
        &Test::libload_done,
        //&Test::libunload_done,
        loadrequest,
        loadresponse,
        callback_callback);
    // load library libecho.so
    libloadstub->Load(NULL, loadrequest, loadresponse, loadcallback);
    //libloadstub->UnLoad(NULL, loadrequest, loadresponse, loadcallback);

    stub = new echo::EchoService_Stub::Stub(client);

    for (int i =0; i < 5; ++i) {
        echo::EchoRequest* request = new echo::EchoRequest();
        request->set_message("client_hello");
        echo::EchoResponse* response = new echo::EchoResponse();
        ::google::protobuf::Closure *callback = pbext::NewCallback(&test,
                                                                   &Test::echo_done,
                                                                   request,
                                                                   response,
                                                                   callback_callback);
        stub->Echo(NULL, request, response, callback);
        echo::DummyRequest* dRequ = new echo::DummyRequest();
        dRequ->set_message("client_hello.dummy.");
        echo::DummyResponse *dResp = new echo::DummyResponse();
        ::google::protobuf::Closure *dcallback = pbext::NewCallback(&test,
            &Test::echo_done,
            dRequ,
            dResp,
            callback_callback);
        stub->Dummy(NULL, dRequ, dResp, dcallback);
    }
    delete client;
    delete stub;
    delete libloadstub;
    for(int i = 0; i < 10; i++) {
        Closure<void>* func = NewClosure(&ThreadRpc);
        ThreadHandle handle = CreateThread(func, "234");
        ::sleep(1);
    }
    while (1) {
        sleep(1);
    }

    return 0;
}
