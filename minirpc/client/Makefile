include ../Makefile.comm
ostype=$(shell uname -s)
PROJECT = libclient.a

INC += -I../

CC += $(SHARED_FLAG)

OBJ = cclientsession.o
OBJ += cmsghandler.o

MODULE = ../common/libcommon.a
MODULE += -lpthread -lstdc++

all : $(PROJECT)

$(PROJECT) : $(OBJ)
	ar -rus $@ $^
	@echo ""
	@echo "+--------------------------------------------+"
	@echo "|            Finish compilation              |"
	@echo "+--------------------------------------------+"
	@echo "|         Thanks using libclient.a           |"
	@echo "|   copyright(c)Wang Yaofu  voipman@qq.com   |"
	@echo "+--------------------------------------------+"

clean:
	rm -rf *.o *.a *.so

