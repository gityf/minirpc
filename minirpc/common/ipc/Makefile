include ../../Makefile.comm

INC += -I../..

OBJ = csharemem.o
OBJ += cmsgqueue.o
OBJ += csemaphore.o
CC += $(SHARED_FLAG)
all : libipc.a
libipc.a : $(OBJ)
	ar -rus $@ $^
	@echo ""
	@echo "+--------------------------------------------+"
	@echo "|            Finish compilation              |"
	@echo "+--------------------------------------------+"
	@echo "|         Thanks using libipc.a              |"
	@echo "|   copyright(c)Wang Yaofu  voipman@qq.com   |"
	@echo "+--------------------------------------------+"

clean:
	rm -rf *.o *.a

