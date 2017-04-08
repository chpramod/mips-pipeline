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
SHARED=time.o misc.o config.o lex.o hash.o inthash.o heap.o pp.o \
	tcp.o bool.o

LIBSIM=sim.o queue.o contexts_uf.o tasking.o log.o checkpoint.o mem.o wakeupcall.o

LIBASIM=prs.o prs_proc.o contexts.o thread.o mutex.o count.o channel.o \
	asim.o alog.o amem.o

# extra flags used to compile fair context library
CASYNC= -DFAIR -DASYNCHRONOUS -DDEBUG_MODE -DCLASS_HACKERY -DCLASS_HACKERY_NONDET

# define -DCLASS_HACKERY if you want all the stuff for
# simulating CHP with the chp2c++ hacks...

CSYNC= -DSYNCHRONOUS

LIBGSIM=view.o

OFILES=$(SHARED) $(LIBASIM) $(LIBSIM) $(LIBGSIM)
GENFILES=guess guess_jmpbuf.o

TARGETS=libsim.a #libasim.a libgsim.a

include $(SIMDIR)/Tools/mk/Makefile.gr

CFLAGS += -Wno-deprecated

libgsim.a: $(LIBGSIM)
	$(ECHO) "Linking graphics library..."
	-ar ruv libgsim.a $(LIBGSIM)
	$(RANLIB) libgsim.a

libsim.a: $(SHARED) $(LIBSIM)
	$(ECHO) "Linking sync simulation library..."
	-ar ruv libsim.a $(SHARED) $(LIBSIM)
	$(RANLIB) libsim.a

libasim.a: $(SHARED) $(LIBASIM)
	$(ECHO) "Linking async simulation library..."
	-ar ruv libasim.a $(SHARED) $(LIBASIM)
	$(RANLIB) libasim.a

#
#
# Graphics stuff
#
#
view.o: view.c deps/view.d
	@$(ECHO) "Compiling $<"
	@$(CC) -c $(CFLAGS) $(GINC) $<

#
#
#  Asynchronous part of the tree
#
#
guess: guess_jmpbuf.o
	@$(CC) $(CFLAGS) -o guess guess_jmpbuf.o $(EXTRALIBS)

contexts.o: guess deps/contexts.d
	@echo Compiling contexts.c
	@make_context $(CC) $(CFLAGS) $(CASYNC) -c

prs.o: prs.c deps/prs.d
	@$(ECHO) "Compiling $<"
	@$(CC) -c $(CFLAGS) $(CASYNC) $<

prs_proc.o: prs_proc.cc deps/prs_proc.d
	@$(ECHO) "Compiling $<"
	@$(CXX) -c $(CFLAGS) $(CASYNC) $<

thread.o: thread.c deps/thread.d
	@$(ECHO) "Compiling $<"
	@$(CC) -c $(CFLAGS) $(CASYNC) $<

mutex.o: mutex.c deps/mutex.d
	@$(ECHO) "Compiling $<"
	@$(CC) -c $(CFLAGS) $(CASYNC) $<

count.o: count.c deps/count.d
	@$(ECHO) "Compiling $<"
	@$(CC) -c $(CFLAGS) $(CASYNC) $<

channel.o: channel.c deps/channel.d
	@$(ECHO) "Compiling $<"
	@$(CC) -c $(CFLAGS) $(CASYNC) $<

asim.o: sim.cc deps/asim.d
	@$(ECHO) "Compiling $<"
	@$(CXX) -c $(CFLAGS) $(CASYNC) $< -o asim.o

alog.o: log.cc deps/alog.d
	@$(ECHO) "Compiling $<"
	@$(CXX) -c $(CFLAGS) $(CASYNC) $< -o alog.o

amem.o: mem.cc deps/amem.d
	@$(ECHO) "Compiling $<"
	@$(CXX) -c $(CFLAGS) $(CASYNC) $< -o amem.o

deps/view.d: view.c
	@$(ECHO) "Deps for $<"
	@$(CC) $(DEPFLAGS) $(CFLAGS) $(GINC) $< > $(basename $<).d
	@$(CP) $(basename $<).d deps
	@$(SED) -e 's,^\(.*\)\.o:,deps/\1.d:,' $(basename $<).d >> deps/$(basename $<).d
	@$(RM) $(basename $<).d

deps/prs.d: prs.c
	@$(ECHO) "Deps for $<"
	@$(CC) $(DEPFLAGS) $(CFLAGS) $(CASYNC) $< > $(basename $<).d
	@$(CP) $(basename $<).d deps
	@$(SED) -e 's,^\(.*\)\.o:,deps/\1.d:,' $(basename $<).d >> deps/$(basename $<).d
	@$(RM) $(basename $<).d

deps/prs_proc.d: prs_proc.cc
	@$(ECHO) "Deps for $<"
	@$(CXX) $(DEPFLAGS) $(CFLAGS) $(CASYNC) $< > $(basename $<).d
	@$(CP) $(basename $<).d deps
	@$(SED) -e 's,^\(.*\)\.o:,deps/\1.d:,' $(basename $<).d >> deps/$(basename $<).d
	@$(RM) $(basename $<).d


deps/thread.d: thread.c
	@$(ECHO) "Deps for $<"
	@$(CC) $(DEPFLAGS) $(CFLAGS) $(CASYNC) $< > $(basename $<).d
	@$(CP) $(basename $<).d deps
	@$(SED) -e 's,^\(.*\)\.o:,deps/\1.d:,' $(basename $<).d >> deps/$(basename $<).d
	@$(RM) $(basename $<).d


deps/mutex.d: mutex.c
	@$(ECHO) "Deps for $<"
	@$(CC) $(DEPFLAGS) $(CFLAGS) $(CASYNC) $< > $(basename $<).d
	@$(CP) $(basename $<).d deps
	@$(SED) -e 's,^\(.*\)\.o:,deps/\1.d:,' $(basename $<).d >> deps/$(basename $<).d
	@$(RM) $(basename $<).d


deps/count.d: count.c
	@$(ECHO) "Deps for $<"
	@$(CC) $(DEPFLAGS) $(CFLAGS) $(CASYNC) $< > $(basename $<).d
	@$(CP) $(basename $<).d deps
	@$(SED) -e 's,^\(.*\)\.o:,deps/\1.d:,' $(basename $<).d >> deps/$(basename $<).d
	@$(RM) $(basename $<).d


deps/channel.d: channel.c
	@$(ECHO) "Deps for $<"
	@$(CC) $(DEPFLAGS) $(CFLAGS) $(CASYNC) $< > $(basename $<).d
	@$(CP) $(basename $<).d deps
	@$(SED) -e 's,^\(.*\)\.o:,deps/\1.d:,' $(basename $<).d >> deps/$(basename $<).d
	@$(RM) $(basename $<).d

deps/asim.d: sim.cc
	@$(ECHO) "Deps for $<"
	@$(CXX) $(CFLAGS) $(CASYNC) $(DEPFLAGS) $< > $(basename $<).d
	@$(SED) -e 's/sim\.o/asim\.o/' sim.d > asim.d
	@$(RM) sim.d
	@$(CP) asim.d deps
	@$(SED) -e 's,^\(.*\)\.o:,deps/asim.d.d:,' asim.d >> deps/asim.d
	@$(RM) asim.d

deps/alog.d: log.cc
	@$(ECHO) "Deps for $<"
	@$(CXX) $(CFLAGS) $(CASYNC) $(DEPFLAGS) $< > $(basename $<).d
	@$(SED) -e 's/log\.o/alog\.o/' log.d > alog.d
	@$(RM) log.d
	@$(CP) alog.d deps
	@$(SED) -e 's,^\(.*\)\.o:,deps/alog.d:,' alog.d >> deps/alog.d
	@$(RM) alog.d

deps/amem.d: mem.cc
	@$(ECHO) "Deps for $<"
	@$(CXX) $(CFLAGS) $(CASYNC) $(DEPFLAGS) $< > $(basename $<).d
	@$(SED) -e 's/log\.o/alog\.o/' mem.d > amem.d
	@$(RM) mem.d
	@$(CP) amem.d deps
	@$(SED) -e 's,^\(.*\)\.o:,deps/amem.d:,' amem.d >> deps/amem.d
	@$(RM) amem.d

#
#
# Synchronous part
#
#
sim.o: sim.cc deps/sim.d
	@$(ECHO) "Compiling $<"
	@$(CXX) -c $(CFLAGS) $(CSYNC) $<

log.o: log.cc deps/log.d
	@$(ECHO) "Compiling $<"
	@$(CXX) -c $(CFLAGS) $(CSYNC) $<

mem.o: mem.cc deps/mem.d
	@$(ECHO) "Compiling $<"
	@$(CXX) -c $(CFLAGS) $(CSYNC) $<

mem2.o: mem2.cc deps/mem2.d
	@$(ECHO) "Compiling $<"
	@$(CXXNOOPT) -c $(CFLAGS) $(CSYNC) $<

queue.o: queue.cc deps/queue.d
	@$(ECHO) "Compiling $<"
	@$(CXX) -c $(CFLAGS) $(CSYNC) $<

tasking.o: tasking.cc deps/tasking.d
	@$(ECHO) "Compiling $<"
	@$(CXX) -c $(CFLAGS) $(CSYNC) $<

wakeupcall.o: wakeupcall.cc deps/wakeupcall.d
	@$(ECHO) "Compiling $<"
	@$(CXX) -c $(CFLAGS) $(CSYNC) $<

checkpoint.o: checkpoint.cc deps/checkpoint.d
	@$(ECHO) "Compiling $<"
	@$(CXX) -c $(CFLAGS) $(CSYNC) $<

contexts_uf.o: contexts.c deps/contexts_uf.d
	@echo Compiling contexts_uf
	@$(CC) -c $(CFLAGS) contexts.c -o contexts_uf.o

deps/contexts_uf.d: contexts.c
	@$(ECHO) "Deps for $<"
	@$(CC) $(CFLAGS) $(CSYNC) $(DEPFLAGS) contexts.c > $(basename $<).d
	@$(MV) contexts.d contexts_uf.d
	@$(CP) contexts_uf.d deps
	@$(SED) -e 's,^\(.*\)\.o:,deps/contexts_uf.d:,' contexts_uf.d >> deps/contexts_uf.d
	@$(RM) contexts_uf.d

deps/sim.d: sim.cc
	@$(ECHO) "Deps for $<"
	@$(CXX) $(CFLAGS) $(CSYNC) $(DEPFLAGS) $< > $(basename $<).d
	@$(CP) $(basename $<).d deps
	@$(SED) -e 's,^\(.*\)\.o:,deps/\1.d:,' $(basename $<).d >> deps/$(basename $<).d
	@$(RM) $(basename $<).d

deps/log.d: log.cc
	@$(ECHO) "Deps for $<"
	@$(CXX) $(CFLAGS) $(CSYNC) $(DEPFLAGS) $< > $(basename $<).d
	@$(CP) $(basename $<).d deps
	@$(SED) -e 's,^\(.*\)\.o:,deps/\1.d:,' $(basename $<).d >> deps/$(basename $<).d
	@$(RM) $(basename $<).d

deps/mem.d: mem.cc
	@$(ECHO) "Deps for $<"
	@$(CXX) $(CFLAGS) $(CSYNC) $(DEPFLAGS) $< > $(basename $<).d
	@$(CP) $(basename $<).d deps
	@$(SED) -e 's,^\(.*\)\.o:,deps/\1.d:,' $(basename $<).d >> deps/$(basename $<).d
	@$(RM) $(basename $<).d

deps/mem2.d: mem2.cc
	@$(ECHO) "Deps for $<"
	@$(CXX) $(CFLAGS) $(CSYNC) $(DEPFLAGS) $< > $(basename $<).d
	@$(CP) $(basename $<).d deps
	@$(SED) -e 's,^\(.*\)\.o:,deps/\1.d:,' $(basename $<).d >> deps/$(basename $<).d
	@$(RM) $(basename $<).d

deps/queue.d: queue.cc
	@$(ECHO) "Deps for $<"
	@$(CXX) $(CFLAGS) $(CSYNC) $(DEPFLAGS) $< > $(basename $<).d
	@$(CP) $(basename $<).d deps
	@$(SED) -e 's,^\(.*\)\.o:,deps/\1.d:,' $(basename $<).d >> deps/$(basename $<).d
	@$(RM) $(basename $<).d

deps/tasking.d: tasking.cc
	@$(ECHO) "Deps for $<"
	@$(CXX) $(CFLAGS) $(CSYNC) $(DEPFLAGS) $< > $(basename $<).d
	@$(CP) $(basename $<).d deps
	@$(SED) -e 's,^\(.*\)\.o:,deps/\1.d:,' $(basename $<).d >> deps/$(basename $<).d
	@$(RM) $(basename $<).d

deps/checkpoint.d: checkpoint.cc
	@$(ECHO) "Deps for $<"
	@$(CXX) $(CFLAGS) $(CSYNC) $(DEPFLAGS) $< > $(basename $<).d
	@$(CP) $(basename $<).d deps
	@$(SED) -e 's,^\(.*\)\.o:,deps/\1.d:,' $(basename $<).d >> deps/$(basename $<).d
	@$(RM) $(basename $<).d

endif
