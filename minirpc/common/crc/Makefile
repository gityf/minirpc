include ../../Makefile.comm

INC += -I.

OBJ = crc16.o
OBJ += crc32.o
OBJ += crc64.o
OBJ += crchash.o
CC += $(SHARED_FLAG)
all : libcrc.a
libcrc.a : $(OBJ)
	ar -rus $@ $^
	@echo ""
	@echo "+--------------------------------------------+"
	@echo "|            Finish compilation              |"
	@echo "+--------------------------------------------+"
	@echo "|         Thanks using libcrc.a              |"
	@echo "|   copyright(c)Wang Yaofu  voipman@qq.com   |"
	@echo "+--------------------------------------------+"

clean:
	rm -rf *.o *.a

