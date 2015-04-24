include ../../../Makefile.comm
PB_HOME = ../../../tools/protobuf-2.6.1/inbin
INC += -I../../..
INC += -I..
INC += -I.
INC += -I$(PB_HOME)/include

OBJ = echo.pb.o
OBJ += basic.pb.o
OBJ += echo2.pb.o

LIB_OBJ = echo.pb.o
CC += $(SHARED_FLAG)
CCC = gcc -g
LIB = -L$(PB_HOME)/lib -lprotobuf
LIB += -lpthread -lstdc++ -ldl

CLN_MODULE = ../librpclient.a\
             ../../../client/libclient.a\
             ../../libcommon.a

SER_MODULE = ../librpcserver.a\
             ../../libcommon.a\
             ../../ev/libsockpoll.a\
             ../../dl/libdlload.a\
             ../../ipc/libipc.a

all : basic.pb.h echo.pb.h echo2.pb.h rpc_server_basic rpc_client_echo\
      rpc_client_echo2 rpc_client_demo rpc_server_demo\
      libecho.so libecho2.so rpc_libloader
basic.pb.h :
	$(PB_HOME)/bin/protoc --cpp_out=. ./basic.proto
echo.pb.h :
	$(PB_HOME)/bin/protoc --cpp_out=. ./echo.proto
echo2.pb.h :
	$(PB_HOME)/bin/protoc --cpp_out=. ./echo2.proto
rpc_server_basic : rpc_server_basic.o  basic.pb.o $(SER_MODULE)
	$(CCC) -o $@ $< basic.pb.o $(SER_MODULE) $(LIB)
rpc_client_echo2 : rpc_client_echo2.o  $(OBJ) $(CLN_MODULE)
	$(CCC) -o $@ $< $(OBJ) $(CLN_MODULE) $(LIB)
rpc_client_demo : rpc_client_demo.o echo.pb.o $(CLN_MODULE)
	$(CCC) -o $@ $< echo.pb.o $(CLN_MODULE) $(LIB)
rpc_client_echo : rpc_client_echo.o  $(OBJ) $(CLN_MODULE)
	$(CCC) -o $@ $< $(OBJ) $(CLN_MODULE) $(LIB)
rpc_libloader : rpc_libloader.o  basic.pb.o $(CLN_MODULE)
	$(CCC) -o $@ $< basic.pb.o $(CLN_MODULE) $(LIB)
rpc_server_demo : rpc_server_demo.o  $(OBJ) $(SER_MODULE)
	$(CCC) -o $@ $< $(OBJ) $(SER_MODULE) $(LIB)
libecho.so : echo.pb.o
	$(CC) lib_rpc_server_echo.cpp -o libecho.so $(INC) echo.pb.o ../../libcommon.a $(LIB)
libecho2.so : echo2.pb.o
	$(CC) lib_rpc_server_echo2.cpp -o libecho2.so $(INC) echo2.pb.o ../../libcommon.a $(LIB)
clean:
	rm -rf *.o *.a *_demo *.so

install :
	test -d ../test || mkdir -p ../test
	cp rpc_server_demo ../test
	cp rpc_client_demo ../test
