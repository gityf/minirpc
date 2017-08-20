# Minirpc
  This is mini rpc depend on google protobuf.
## Data flow
![auth flow icon](https://github.com/gityf/minirpc/blob/master/minirpc/doc/minirpc-flow.png)
## Agent display service stat list
![auth flow icon](https://github.com/gityf/minirpc/blob/master/minirpc/doc/agent_service_display_page.png)
## Agent display service list.
![auth flow icon](https://github.com/gityf/minirpc/blob/master/minirpc/doc/minirpc-agent.png)
## Intalling
  build gtest/protobuf source code for unittest.
```bash
cd minirpc/tools
unzip gtest-1.7.0.zip
cd gtest-1.7.0
./configure
make
```
  to build protobuf
```bash
tar -xvf protobuf-2.6.1.tar.gz
cd protobuf-2.6.1
./configure --prefix=`pwd`/inbin
make ; make install
```

  build and install common , client, and agent directory code.
```bash
cd common
make clean; make
cd ../client
make clean ; make
cd ../agent
make clean; make
```
## example
### create a echo library.
```c++

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

```
