#
#  $Id: Makefile,v 1.1.1.1 2006/05/23 13:53:41 mainakc Exp $
#
ifndef SIMDIR
all clean clobber depend realclean:
	@echo "***********************************************************"
	@echo "*                                                         *"
	@echo "*  Set SIMDIR to the root of the main Sim repository      *"
	@echo "*                                                         *"
	@echo "***********************************************************"
else

# extra flags used for simulation stuff... synchronous simulation env


FILES:=vadd
OFILES:=$(FILES:%=%.o)

GENFILES=boot.image boot.o
TARGETS:=vadd.image

INCLUDES += -I$(SIMDIR)/Bench/lib -I$(SIMDIR)/cpus/common
MORECFLAGS += $(INCLUDES)
SUPPORTLIB := $(SIMDIR)/Bench/lib/supportlib.a
LIBS      += $(SUPPORTLIB) 

include $(SIMDIR)/Tools/mk/Makefile.std

CC:=$(SIMDIR)/Tools/bin/mips-cc.pl -O2 -k

vadd.image: $(OFILES)
	$(ECHO) "Linking vadd..."
	$(CC) -o vadd vadd.o $(LIBS)

%.o: %.s
	$(CC) -c $*.s 

endif
