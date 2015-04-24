include Makefile.comm

PROJECT = ./tools/desp
PROJECT0 = ./common/ev/libsockpoll.a
PROJECT1 = ./common/libcommon.a
PROJECT2 = ./agent/agent
PROJECT3 = ./client/libagent.a

all: $(PROJECT) $(PROJECT0) $(PROJECT1) $(PROJECT2) $(PROJECT3)

$(PROJECT):
	gmake -C tools || make -C tools

$(PROJECT0):
	gmake -C common/ev || make -C common/ev

$(PROJECT1):
	gmake -C common || make -C common

$(PROJECT2):
	gmake -C agent || make -C agent

$(PROJECT3):
	gmake -C client || make -C client

clean :
	gmake -C common/ev clean || make -C common/ev clean
	gmake -C common clean || make -C common clean
	gmake -C agent  clean || make -C agent  clean
	gmake -C client clean || make -C client clean
	gmake -C interface clean || make -C interface clean
	gmake -C scs clean || make -C scs clean

install :
	test -d ./test || mkdir ./test
	gmake -C agent  install || make -C agent  install
	gmake -C client install || make -C client install
