include ../Makefile.comm
ostype=$(shell uname -s)
PROJECT = miniagent
#MONITOR_EMAIL=1
LIB = -lstdc++ -lpthread
ifeq (${ostype}, SunOS)
LIB +=-lsocket -lnsl -lintl
endif

ifeq (${ostype}, AIX)
endif

ifeq (${ostype}, Linux)
endif

INC  = -I..

MODULE = ../common/libcommon.a ../common/ev/libsockpoll.a

OBJ = csessionrs.o
OBJ += csessionsync.o
OBJ += chttphandler.o
OBJ += cagentservice.o
ifdef MONITOR_EMAIL
CC += -DMONITOR_EMAIL
OBJ += cagentmonitor.o
endif
OBJ += ../common/ev/ceventhandler.o

$(PROJECT) : agent.o $(OBJ) $(MODULE)
	$(CC) -o $@ $< $(OBJ) $(MODULE) $(LIB)
	@echo ""
	@echo "+--------------------------------------------+"
	@echo "|            Finish compilation              |"
	@echo "+--------------------------------------------+"
	@echo "|         Thanks using MiniAgent             |"
	@echo "|   copyright(c)Wang Yaofu  voipman@qq.com   |"
	@echo "+--------------------------------------------+"

clean:
	rm -rf *.o *.a $(PROJECT)

install : $(PROJECT)
	test -d ../test || mkdir -p ../test
	cp $(PROJECT) ../test
