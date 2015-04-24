include ../Makefile.comm

INC += -I../
INC += -I./

OBJ = cconfig.o
OBJ += ini_parser.o
OBJ += csmtpmail.o
OBJ += clogwriter.o
OBJ += cthread.o
OBJ += tinythread.o
OBJ += csocket.o
OBJ += cprocess.o
OBJ += chttputils.o
OBJ += chttpclient.o
OBJ += cstringutils.o
OBJ += cfilesys.o
OBJ += cdatetime.o
OBJ += csetproctitle.o
OBJ += cprocmaintain.o
OBJ += cstatisticinfo.o
OBJ += base64.o
#OBJ += cpassfd.o
CC += $(SHARED_FLAG)
all : \
	libcommon.a \
	./ev/libsockpoll.a \
	./debug/libdebug.a \
	./crc/libcrc.a \
	./ipc/libipc.a \
	./cipher/libcipher.a \
	./dl/libdlload.a \
	./rpc/librpcserver.a \
	./rpc/protobuf/demo \

libcommon.a : $(OBJ)
	ar -rus $@ $^
	@echo ""
	@echo "+--------------------------------------------+"
	@echo "|            Finish compilation              |"
	@echo "+--------------------------------------------+"
	@echo "|         Thanks using libcommon.a           |"
	@echo "|   copyright(c)Wang Yaofu  voipman@qq.com   |"
	@echo "+--------------------------------------------+"
./ev/libsockpoll.a :
	gmake -C ev || make -C ev

./debug/libdebug.a :
	gmake -C debug || make -C debug

./crc/libcrc.a :
	gmake -C crc || make -C crc

./ipc/libipc.a :
	gmake -C ipc || make -C ipc

./cipher/libcipher.a :
	gmake -C cipher || make -C cipher

./dl/libdlload.a :
	gmake -C dl || make -C dl

./rpc/librpcserver.a :
	gmake -C rpc || make -C rpc

./rpc/protobuf/demo :
	gmake -C rpc/protobuf || make -C rpc/protobuf

clean:
	rm -rf *.o *.a
	rm -rf debug/*.o debug/*.a
	rm -rf crc/*.o crc/*.a
	rm -rf ipc/*.o ipc/*.a
	rm -rf cipher/*.o cipher/*.a
	rm -rf dl/*.o dl/*.a

