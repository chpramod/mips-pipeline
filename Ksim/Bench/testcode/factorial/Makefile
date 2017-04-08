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


FILES:=factorial
OFILES:=$(FILES:%=%.o)

GENFILES=boot.image boot.o
TARGETS:=factorial.image

INCLUDES += -I$(SIMDIR)/Bench/lib -I$(SIMDIR)/cpus/common
MORECFLAGS += $(INCLUDES)
SUPPORTLIB := $(SIMDIR)/Bench/lib/supportlib.a
LIBS      += $(SUPPORTLIB) 

include $(SIMDIR)/Tools/mk/Makefile.std

CC:=$(SIMDIR)/Tools/bin/mips-cc.pl -O2 -k

factorial.image: $(OFILES)
	$(ECHO) "Linking factorial..."
	$(CC) -o factorial factorial.o $(LIBS)

%.o: %.s
	$(CC) -c $*.s 

endif
