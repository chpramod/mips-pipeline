#!/usr/local/bin/gmake
#
#  $Id: Makefile,v 1.1.1.1 2006/05/23 13:53:59 mainakc Exp $
#
#
#
ifndef SIMDIR
all clean clobber depend realclean:
	@echo "***********************************************************"
	@echo "*                                                         *"
	@echo "*  Set SIMDIR to the root of the main Sim repository      *"
	@echo "*                                                         *"
	@echo "***********************************************************"
else

#
# standard stuff to link against
#
VPATH= .:../../common:

SUBDIRS=$(SIMDIR)/lib

# extra flags used for simulation stuff... synchronous simulation env
MORECFLAGS+=-DSYNCHRONOUS -DMIPS_FAST -I../../common $(GTK_FLAGS) -Wno-deprecated -include /usr/include/errno.h

CORE:=mips.o exec_helper.o syscall.o decode.o executor.o memory.o wb.o
MIPC:=main.o

MIPC_OFILES=$(MIPC) $(CORE)
OFILES:=$(CORE) $(MIPC)
TARGETS=mipc

include $(SIMDIR)/Tools/mk/Makefile.std

mipc: $(MIPC_OFILES) $(LDEP) $(LIBS)
	$(ECHO) "Linking $@..."
	$(CXX) -o $@ $(MIPC_OFILES) $(LFLAGS) $(LIBS) -lsim
endif
