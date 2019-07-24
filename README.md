# Minirpc
  This is mini rpc depend on google protobuf.
```
  minirpc 是基于 protobuf 的分布式 RPC 系统。

1. rpc server端提供的service可以是so库文件方式存在，可以使用libloader工具增加到server上。
2. rpc server会将自己所提供的服务注册到agent上。
3. agent服务中心采用去中心化的方式运行，通过加入组播实现rpc server的服务列表管理和运行状态的准实时展现。
4. rpc client可以使用需要调用的server名称（字符串服务名），向agent中心获取提供此类服务的rpc server，之后连接rpc server实现数据传输和业务处理。
```
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

### Where is example
    location: minirpc/common/rpc/protobuf
    echo.proto is proto IDL file.
    to run 
```bash
 protoc --cpp_out=. ./echo.proto
``` 
    then

    echo.pn.h and echo.pb.cc will be created by protoc command.

#### rpc_server_basic.cpp is rpc-server source code demo file.
#### rpc_libloader.cpp is tool for loading so file for rpc_server_basic.
#### rpc_client_demo.cpp is rpc-client source code demo file.

