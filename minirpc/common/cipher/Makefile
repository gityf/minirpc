include ../../Makefile.comm

INC += -I../
INC += -I./

OBJ = aes.o
OBJ = tdes.o
OBJ += md5.o
OBJ += md5key.o
OBJ += sha1.o
OBJ += sha.o
OBJ += sha256.o
OBJ += sha512.o
CC += $(SHARED_FLAG)
all : libcipher.a
libcipher.a : $(OBJ)
	ar -rus $@ $^
	@echo ""
	@echo "+--------------------------------------------+"
	@echo "|            Finish compilation              |"
	@echo "+--------------------------------------------+"
	@echo "|         Thanks using libcipher.a           |"
	@echo "|   copyright(c)Wang Yaofu  voipman@qq.com   |"
	@echo "+--------------------------------------------+"

clean:
	rm -rf *.o *.a

