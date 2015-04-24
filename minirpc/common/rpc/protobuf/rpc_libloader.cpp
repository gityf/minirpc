/*
** Copyright (C) 2014 Wang Yaofu
** All rights reserved.
**
**Description: The source file of class RpcClient echo demo test.
*/

#include "rpc_client.h"
#include "pbext.h"
#include "basic.pb.h"
#include <unistd.h>
#include <string>
#include <iostream>
using namespace std;

class Test {
public:
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
        std::cout << "load success\n";
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
            std::cout << "unload success\n";
    }
};

int main(int argc, char *argv[]) {
    if (argc < 4) {
        std::cout << "Usage:whichServer whichLibrary...\n";
        std::cout << "e.g: argv[0]"
                  << " BasicServer load libecho.so\n";
        std::cout << "e.g: argv[0]"
                  << " BasicServer unload libecho.so\n";
        return -1;
    }
    Test test;
    ::google::protobuf::RpcChannel* client = new RpcClient(argv[1]);
    basic::LibLoadService_Stub::Stub* libloadstub;
    if (!((RpcClient*)client)->InitData() == -1) {
        delete client;
        exit(0);
    } else {
        std::cout << "create success\n";
    }
    libloadstub = new basic::LibLoadService_Stub::Stub(client);
    for (int a = 3; argv[a]; ++a) {
        basic::LibLoadRequest* loadrequest = new basic::LibLoadRequest();
        loadrequest->set_library(argv[a]);
        basic::LibLoadResponse* loadresponse = new basic::LibLoadResponse();
        ::google::protobuf::Closure* callback_callback = NULL;
        ::google::protobuf::Closure *loadcallback;
        std::string cmd = "load";
        if (cmd == argv[2]) {
            loadcallback = pbext::NewCallback(&test,
            &Test::libload_done,
            //&Test::libunload_done,
            loadrequest,
            loadresponse,
            callback_callback);
            // load library
            libloadstub->Load(NULL, loadrequest, loadresponse, loadcallback);
        } else {
            loadcallback = pbext::NewCallback(&test,
            &Test::libunload_done,
            loadrequest,
            loadresponse,
            callback_callback);
            // unload library
            libloadstub->UnLoad(NULL, loadrequest, loadresponse, loadcallback);
        }
    }
    int t = 5;
    do {
        sleep(1);
        std::cout << "It will exit at " << t << " seconds.\n";
    } while(t-- > 0);
    delete client;
    delete libloadstub;
    return 0;
}
