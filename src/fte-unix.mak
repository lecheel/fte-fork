# versions of FTE to build

TARGETS = nfte
PRIMARY = nfte
XMBFLAG = #-DUSE_XMB
USE_LOCALE = -DUSE_LOCALE

I18NOPTIONS = $(XMBFLAG) $(REMAPFLAG) $(SYSTEM_X_LOCALE) $(USE_LOCALE)

APPOPTIONS = -DDEFAULT_INTERNAL_CONFIG

#gcc/g++
COPTIONS = -Wall -Wpointer-arith -Wconversion -Wwrite-strings \
           -Winline -Wno-narrowing

CC       = g++ -fno-rtti -fno-exceptions
LD       = g++ -fno-rtti -fno-exceptions
UOS      = -DLINUX
XINCDIR  = -I/usr/X11R6/include
XLIBDIR  = -L/usr/X11R6/lib -lstdc++
MOC      = moc

LIBDIR   =
INCDIR   =

OPTIMIZE = -g # -O -g

CCFLAGS  = $(OPTIMIZE) $(I18NOPTIONS) $(APPOPTIONS) $(COPTIONS) -DUNIX $(UOS) $(INCDIR) $(XINCDIR) $(QINCDIR) $(MINCDIR)
LDFLAGS  = $(OPTIMIZE) $(LIBDIR) $(XLIBDIR) $(QLIBDIR) $(MLIBDIR)

OEXT     = o

.SUFFIXES: .cpp .o .moc

include objs.inc
SRCS = $(OBJS:.o=.cpp) $(UNIXOBJS:.o=.cpp) $(CFTE_OBJS:.o=.cpp)

OBJS := $(addprefix obj/,$(OBJS))
NOBJS := $(addprefix obj/,$(NOBJS))
CFTE_OBJS := $(addprefix obj/,$(CFTE_OBJS))

XLIBS    = -lX11 $(SOCKETLIB)
NLIBS    = -lncurses
QLIBS    = -lqt

.PHONY: all cfte nfte xfte clean

all:    cfte $(TARGETS)

obj:
	@mkdir -p obj

obj/%.o: %.cpp | obj
	$(CC) $(CCFLAGS) -c $< -o $@

obj/%.o: %.c | obj
	$(CC) $(CCFLAGS) -c $< -o $@

.cpp.moc:
	$(MOC) $< -o $@
#rm -f fte ; ln -s $(PRIMARY) fte

cfte: $(CFTE_OBJS)
	$(LD) $(LDFLAGS) $(CFTE_OBJS) -o cfte

obj/c_config.o: defcfg.h

defcfg.h: defcfg.cnf
	perl mkdefcfg.pl <defcfg.cnf >defcfg.h

#DEFAULT_FTE_CONFIG = simple.fte
DEFAULT_FTE_CONFIG = defcfg.fte
#DEFAULT_FTE_CONFIG = defcfg2.fte
#DEFAULT_FTE_CONFIG = ../config/main.fte

defcfg.cnf: $(DEFAULT_FTE_CONFIG) cfte
	./cfte $(DEFAULT_FTE_CONFIG) defcfg.cnf

xfte: .depend $(OBJS) $(XOBJS)
	$(LD) -o $@ $(LDFLAGS) $(OBJS) $(XOBJS) $(XLIBS)

nfte: $(OBJS) $(NOBJS)
	$(LD) -o $@ $(LDFLAGS) $(OBJS) $(NOBJS) $(NLIBS)

g_qt.obj: g_qt.moc

g_qt_dlg.obj: g_qt_dlg.moc

.depend: defcfg.h
	$(CC) -MM $(CCFLAGS) $(SRCS) | sed 's,^\([^ ]*\)\.o:,obj/\1.o:,' > .depend

# purposefully not part of "all".
tags: $(SRCS) $(wildcard *.h)
	ctags *.h $(SRCS)

clean:
	rm -f core *.o .depend $(TARGETS) defcfg.h defcfg.cnf cfte fte tags
	rm -rf obj

#
# include dependency files if they exist
#
ifneq ($(wildcard .depend),)
include .depend
endif
