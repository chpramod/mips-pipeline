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


FILES:=rfib
OFILES:=$(FILES:%=%.o)

GENFILES=boot.image boot.o
TARGETS:=rfib.image

INCLUDES += -I$(SIMDIR)/Bench/lib -I$(SIMDIR)/cpus/common
MORECFLAGS += $(INCLUDES)
SUPPORTLIB := $(SIMDIR)/Bench/lib/supportlib.a
LIBS      += $(SUPPORTLIB) 

include $(SIMDIR)/Tools/mk/Makefile.std

CC:=$(SIMDIR)/Tools/bin/mips-cc.pl -O2 -k

rfib.image: $(OFILES)
	$(ECHO) "Linking rfib..."
	$(CC) -o rfib rfib.o $(LIBS)

%.o: %.s
	$(CC) -c $*.s 

endif
