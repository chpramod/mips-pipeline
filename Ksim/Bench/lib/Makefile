#
#  $Id: Makefile,v 1.1.1.1 2006/05/23 13:53:41 mainakc Exp $
#
#
#
#  Tweak your compiler here
#
# Works for most architectures
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
OFILES=asm_support.o c_support.o

HOST_CC=$(SIMDIR)/Tools/bin/mips-cc.pl -mips2 -I$(SIMDIR)/cpus/common -O2
HOST_AS=/usr/local/bin/mips-sgi-irix5-gcc -mips2 -non_shared -x assembler-with-cpp -EB -O -DX86 -D__MIPSEB -I. 
AR=/usr/local/bin/mips-sgi-irix5-ld -noinhibit-exec

TARGETS=supportlib.a

include $(SIMDIR)/Tools/mk/Makefile.std

CC=$(HOST_CC)

$(TARGETS): $(OFILES)
	-@$(ECHO) "Generating $(TARGETS)..."
	-@$(AR) -r -o $(TARGETS) $(OFILES)

.s.o:
	-@$(ECHO) "Cross-assembling $<..."
	-@$(HOST_AS) -c $<

endif
