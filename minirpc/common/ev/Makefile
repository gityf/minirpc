include ../../Makefile.comm

INC = -I../../
CC += $(SHARED_FLAG)
OBJ = ceventloop.o
OBJ += ceventpoll.o
OBJ += ceventhandler.o
libsockpoll.a : $(OBJ)
	ar -rus $@ $^
	@echo ""
	@echo "+--------------------------------------------+"
	@echo "|            Finish compilation              |"
	@echo "+--------------------------------------------+"
	@echo "|         Thanks using libsockpoll.a         |"
	@echo "|   copyright(c)Wang Yaofu  voipman@qq.com   |"
	@echo "+--------------------------------------------+"
clean:
	rm -rf *.o *.a

