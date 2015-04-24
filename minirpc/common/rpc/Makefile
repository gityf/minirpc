include ../../Makefile.comm
PB_HOME = ../../tools/protobuf-2.6.1/inbin
INC += -I../..
#INC += -I.
INC += -I$(PB_HOME)/include

OBJ = rpc_client.o

OBJ_SER = rpc_ipserver.o
OBJ_SER += rpc_msghandler.o
OBJ_SER += rpc_server.o
CC += $(SHARED_FLAG)
all : librpclient.a\
	librpcserver.a

librpclient.a : $(OBJ)
	ar -rus $@ $^
	@echo ""
	@echo "+--------------------------------------------+"
	@echo "|            Finish compilation              |"
	@echo "+--------------------------------------------+"
	@echo "|         Thanks using librpclient.a         |"
	@echo "|   copyright(c)Wang Yaofu  voipman@qq.com   |"
	@echo "+--------------------------------------------+"
librpcserver.a : $(OBJ_SER)
	ar -rus $@ $^
	@echo ""
	@echo "+--------------------------------------------+"
	@echo "|            Finish compilation              |"
	@echo "+--------------------------------------------+"
	@echo "|         Thanks using librpcserver.a        |"
	@echo "|   copyright(c)Wang Yaofu  voipman@qq.com   |"
	@echo "+--------------------------------------------+"

clean:
	rm -rf *.o *.a

