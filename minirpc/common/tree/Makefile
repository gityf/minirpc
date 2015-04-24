include ../../Makefile.comm

INC += -I../
INC += -I./

OBJ = rbtree.o
OBJ += tree234.o

CC += $(SHARED_FLAG)
all : libtree.a
libtree.a : $(OBJ)
	ar -rus $@ $^
	@echo ""
	@echo "+--------------------------------------------+"
	@echo "|            Finish compilation              |"
	@echo "+--------------------------------------------+"
	@echo "|         Thanks using libtree.a             |"
	@echo "|   copyright(c)Wang Yaofu  voipman@qq.com   |"
	@echo "+--------------------------------------------+"

clean:
	rm -rf *.o *.a

