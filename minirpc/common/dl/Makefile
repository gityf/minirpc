include ../../Makefile.comm

INC += -I../../
INC += -I./

OBJ = library.o
OBJ += libraryimpl.o
CC += $(SHARED_FLAG)
all : libdlload.a
libdlload.a : $(OBJ)
	ar -rus $@ $^
	@echo ""
	@echo "+--------------------------------------------+"
	@echo "|            Finish compilation              |"
	@echo "+--------------------------------------------+"
	@echo "|         Thanks using libdlload.a           |"
	@echo "|   copyright(c)Wang Yaofu  voipman@qq.com   |"
	@echo "+--------------------------------------------+"

dlloader : dlloader.o $(OBJ)
	gcc -g -o $@ $< $(OBJ) -ldl -lstdc++
	@echo ""
	@echo "+--------------------------------------------+"
	@echo "|            Finish compilation              |"
	@echo "+--------------------------------------------+"
	@echo "|         Thanks using dlloader              |"
	@echo "|   copyright(c)Wang Yaofu  voipman@qq.com   |"
	@echo "+--------------------------------------------+"

clean:
	rm -rf *.o *.a

