include ../../Makefile.comm

ostype=$(shell uname -s)
LIB = -lpthread -lstdc++
ifeq (${ostype}, SunOS)
	LIB += -lintl
endif

ifeq (${ostype}, AIX)
	LIB += -brtl
endif
GTEST_HOME=../../tools/gtest-1.7.0
LIB += $(GTEST_HOME)/lib/.libs/libgtest.a
LIB += $(GTEST_HOME)/lib/.libs/libgtest_main.a
LIB += ../../common/libcommon.a\
       ../../common/cipher/libcipher.a\
       ../../common/ipc/libipc.a
LIB += -lpthread

INC  = -I../..
INC += -I$(GTEST_HOME)/include

ALL = common_unittest\
	cmsgqueue_unittest\
	csingetone_unittest\
	chttp_unittest\
	csocket_unittest\
	ini_parser_unittest\
	closure_unittest\
	tinythread_unittest\
	serialize_unittest

all : $(ALL)
	@echo ""
	@echo "+--------------------------------------------+"
	@echo "|      Finish compilation unittest           |"
	@echo "+--------------------------------------------+"
	@echo "|   copyright(c)Wang Yaofu  voipman@qq.com   |"
	@echo "+--------------------------------------------+"

cmsgqueue_unittest : cmsgqueue_unittest.o
	$(CC) -o $@ $< $(LIB)
common_unittest : common_unittest.o
	$(CC) -o $@ $< $(LIB)
csingetone_unittest : csingetone_unittest.o
	$(CC) -o $@ $< $(LIB)
chttp_unittest : chttpclient_unittest.o
	$(CC) -o $@ $< $(LIB) 
csocket_unittest : csocket_unittest.o
	$(CC) -o $@ $< $(LIB)
ini_parser_unittest : ini_parser_unittest.o
	$(CC) -o $@ $< $(LIB) 
closure_unittest : closure_unittest.o
	$(CC) -o $@ $< $(LIB) 
tinythread_unittest : tinythread_unittest.o
	$(CC) -o $@ $< $(LIB) 
serialize_unittest : serialize_unittest.o
	$(CC) -o $@ $< $(LIB) 


clean:
	rm -rf *.o *.a *_unittest

install :
	test -d ../test || mkdir -p ../test
	cp *_unittest ../test
