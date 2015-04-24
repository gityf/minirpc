#include "rpc_serverobserver.h"
#include "echo.pb.h"
#include "common/clogwriter.h"
#include <unistd.h>
extern "C" {
    int echo_Init(CRpcSerObserver* aRpcServer);
}
class LibEchoServiceImpl : public echo::EchoService {
    virtual void Echo(::google::protobuf::RpcController* controller,
                      const ::echo::EchoRequest* request,
                      ::echo::EchoResponse* response,
                      ::google::protobuf::Closure* done) {

        response->set_response(request->message()+" lib_rpc_server_echo_hello");
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
};
int echo_Init(CRpcSerObserver* aRpcServer) {
    DEBUG(LL_WARN, "echo_Init Begin.");
    RPCREGI(aRpcServer, LibEchoServiceImpl);
    return 0;
}
