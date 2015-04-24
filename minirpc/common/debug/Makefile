include ../../Makefile.comm

INC += -I../..

OBJ = cstack_trace.o
CC += $(SHARED_FLAG)
all : libdebug.a
libdebug.a : $(OBJ)
	ar -rus $@ $^
	@echo ""
	@echo "+--------------------------------------------+"
	@echo "|            Finish compilation              |"
	@echo "+--------------------------------------------+"
	@echo "|         Thanks using libdebug.a            |"
	@echo "|   copyright(c)Wang Yaofu  voipman@qq.com   |"
	@echo "+--------------------------------------------+"

clean:
	rm -rf *.o *.a

