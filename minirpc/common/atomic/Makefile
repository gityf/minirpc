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
LIB += ../libcommon.a
LIB += -lpthread

INC  = -I../..
INC += -I$(GTEST_HOME)/include

ALL = atomic_performance_unittest\
	atomic_unittest\
	this_thread_unittest

all : $(ALL)
	@echo ""
	@echo "+--------------------------------------------+"
	@echo "|      Finish compilation unittest           |"
	@echo "+--------------------------------------------+"
	@echo "|   copyright(c)Wang Yaofu  voipman@qq.com   |"
	@echo "+--------------------------------------------+"

atomic_performance_unittest : atomic_performance_unittest.o
	$(CC) -o $@ $< $(LIB)
atomic_unittest : atomic_unittest.o
	$(CC) -o $@ $< $(LIB)
this_thread_unittest : this_thread_unittest.o
	$(CC) -o $@ $< $(LIB)

clean:
	rm -rf *.o *.a *_unittest

install :
	test -d ../test || mkdir -p ../test
	cp *_unittest ../test
