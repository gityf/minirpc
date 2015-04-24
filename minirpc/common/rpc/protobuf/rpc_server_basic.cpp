#include <string>
#include "rpc_server.h"
#include "basic.pb.h"
using std::string;

class BasicServiceImpl : public basic::LibLoadService {
    virtual void Load(::google::protobuf::RpcController* controller,
                      const ::basic::LibLoadRequest* request,
                      ::basic::LibLoadResponse* response,
                      ::google::protobuf::Closure* done) {
        string loadlib = request->library();
        string loadresult = loadlib;
        if (!loadresult.empty()) {
            if (_rpc_server->LoadService(loadlib) == 0) {
                loadresult += " load success, method list: ";
                loadresult += _rpc_server->DumpAllMethodsInLib(loadlib);

            } else {
                loadresult += " load failed.";
            }
            response->set_loadresult(loadresult);
        } else {
            response->set_loadresult("request library is null.");
        }        
        if (done) {
            done->Run();
        }
    }
    virtual void UnLoad(::google::protobuf::RpcController* controller,
        const ::basic::LibLoadRequest* request,
        ::basic::LibLoadResponse* response,
        ::google::protobuf::Closure* done) {
            string loadresult = request->library();
            if (!loadresult.empty()) {
                if (_rpc_server->UnLoadService(loadresult) == 0) {
                    loadresult += " load success.";
                } else {
                    loadresult += " load failed.";
                }
                response->set_loadresult(loadresult);
            } else {
                response->set_loadresult("request library is null.");
            }        
            if (done) {
                done->Run();
            }
    }
public:
    RpcServer* _rpc_server;
public:
    BasicServiceImpl(RpcServer* rpc_server) {
        _rpc_server = rpc_server;
    }
};

int main(int argc, char *argv[])
{
    RpcServer server("BasicServer");
    RPCREGI_BASIC(server, BasicServiceImpl, &server);
    if (server.InitData(argc, argv) < 0) {
        return -1;
    }
    server.Start();
    return 0;
}
