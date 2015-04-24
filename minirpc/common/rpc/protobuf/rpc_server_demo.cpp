#include "rpc_server.h"
#include "echo.pb.h"

class EchoServiceImpl : public echo::EchoService {
    virtual void Echo(::google::protobuf::RpcController* controller,
                      const ::echo::EchoRequest* request,
                      ::echo::EchoResponse* response,
                      ::google::protobuf::Closure* done) {

        response->set_response(request->message()+" server_hello");
        if (done) {
            done->Run();
        }
    }
    virtual void Dummy(::google::protobuf::RpcController* controller,
                       const ::echo::DummyRequest* request,
                       ::echo::DummyResponse* response,
                       ::google::protobuf::Closure* done) {
        if (done) {
            done->Run();
        }
    }
public:
    RpcServer* _rpc_server;
public:
    EchoServiceImpl(RpcServer* rpc_server) {
        _rpc_server = rpc_server;
    }
};

int main(int argc, char *argv[])
{
    RpcServer server("BasicServer");
    RPCREGI_BASIC(server, EchoServiceImpl, &server);
    if (server.InitData(argc, argv) < 0) {
        return -1;
    }
    server.Start();
    return 0;
}
