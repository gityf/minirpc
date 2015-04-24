#include "rpc_serverobserver.h"
#include "echo2.pb.h"
#include "common/clogwriter.h"
#include <unistd.h>
extern "C" {
    int echo2_Init(CRpcSerObserver* aRpcServer);
}
class LibEcho2ServiceImpl : public echo2::EchoService {
    virtual void Echo(::google::protobuf::RpcController* controller,
                      const ::echo2::EchoRequest* request,
                      ::echo2::EchoResponse* response,
                      ::google::protobuf::Closure* done) {

        response->set_response(request->message()+" lib_rpc_server_echo2_hello");
        if (done) {
            done->Run();
        }
    }
    virtual void Dummy(::google::protobuf::RpcController* controller,
                       const ::echo2::DummyRequest* request,
                       ::echo2::DummyResponse* response,
                       ::google::protobuf::Closure* done) {
        if (done) {
            done->Run();
        }
    }
};
int echo2_Init(CRpcSerObserver* aRpcServer) {
    DEBUG(LL_WARN, "echo2_Init Begin.");
    RPCREGI(aRpcServer, LibEcho2ServiceImpl);
    return 0;
}
