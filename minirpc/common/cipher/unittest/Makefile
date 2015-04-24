include ../../../Makefile.comm

ostype=$(shell uname -s)
LIB = -lpthread -lstdc++
ifeq (${ostype}, SunOS)
	LIB += -lintl
endif

ifeq (${ostype}, AIX)
	LIB += -brtl
endif
GTEST_HOME=../../../tools/gtest-1.7.0
LIB += $(GTEST_HOME)/lib/.libs/libgtest.a
LIB += $(GTEST_HOME)/lib/.libs/libgtest_main.a
LIB += ../libcipher.a ../../libcommon.a ../../crc/libcrc.a

INC  = -I.
INC  += -I../
INC += -I$(GTEST_HOME)/include

ALL = cipher_unittest

all : $(ALL)
	@echo ""
	@echo "+--------------------------------------------+"
	@echo "|      Finish compilation unittest           |"
	@echo "+--------------------------------------------+"
	@echo "|   copyright(c)Wang Yaofu  voipman@qq.com   |"
	@echo "+--------------------------------------------+"

cipher_unittest : cipher_unittest.o
	$(CC) -o $@ $< $(LIB)

clean:
	rm -rf *.o *.a *_unittest

install :
	test -d ../test || mkdir -p ../test
	cp *_unittest ../test
