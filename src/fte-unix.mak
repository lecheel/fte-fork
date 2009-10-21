# versions of FTE to build

# Versions:
#  xfte - using XLib (the most stable)

#  vfte - for Linux console directly (with limitations, see con_linux.cpp)

TARGETS = nfte xfte sfte vfte
#TARGETS = xfte vfte sfte nfte
#TARGETS = xfte

PRIMARY = xfte

# Comment or uncoment this two flags below if
# you want to use:

# Keyboard remaping by XFTE
#REMAPFLAG = -DUSE_HARD_REMAP

# Drawing fonts with locale support
#XMBFLAG = -DUSE_XMB

# System X11R6 is compiled with X_LOCALE
#SYSTEM_X_LOCALE = -DX_LOCALE

# Enable normal locale support
#USE_LOCALE = -DUSE_LOCALE

#I18NOPTIONS = $(XMBFLAG) $(REMAPFLAG) $(SYSTEM_X_LOCALE) $(USE_LOCALE)

# Optionally, you can define:
# -DDEFAULT_INTERNAL_CONFIG to use internal config by default
# -DUSE_XTINIT to use XtInitialize on init
# -DFTE_NO_LOGGING to completely disable trace logging
ifeq "$(FTECONFIG)" "minimal"
DEFAULT_FTE_CONFIG = minimal.fte
APPOPTIONS = -DCONFIG_MINIMAL
endif
ifeq "$(FTECONFIG)" "dosedit"
DEFAULT_FTE_CONFIG = dosedit.fte
APPOPTIONS = -DCONFIG_MINIMAL -DCONFIG_DOSEDIT
endif
ifeq "$(DEFAULT_FTE_CONFIG)" ""
DEFAULT_FTE_CONFIG = simple.fte
endif


PREFIX=/usr/local
SYSTEM_FTERC=$(PREFIX)/lib/fte/system.fterc

#gcc/g++
#COPTIONS = -Wall -Wpointer-arith -Wconversion -Wwrite-strings \
#           -Winline

#CROSS = /export/tools/bin/i386-linux-
#CC       = g++
#LD       = g++
# try this for smaller/faster code and less dependencies
CCNATIVE = g++ -fno-rtti -fno-exceptions 
LDNATIVE = g++ -fno-rtti -fno-exceptions 
CC       = $(CROSS)$(CCNATIVE)
LD       = $(CROSS)$(LDNATIVE)


# choose your os here

#######################################################################
# Linux
UOS      = -DLINUX
XINCDIR  = -I/usr/X11R6/include
XLIBDIR  = -L/usr/X11R6/lib -lstdc++

#######################################################################
# HP/UX
#UOS      = -DHPUX -D_HPUX_SOURCE -DCAST_FD_SET_INT
#UOS      = -DHPUX -D_HPUX_SOURCE

#CC   = CC +a1
#LD   = CC
#CC = aCC
#LD = aCC

#XLIBDIR  = -L/usr/lib/X11R6

#XINCDIR  = -I/usr/include/X11R5
#XLIBDIR  = -L/usr/lib/X11R5

#MINCDIR  = -I/usr/include/Motif1.2
#MLIBDIR  = -L/usr/lib/Motif1.2

#SINCDIR   = -I/usr/include/slang
#SINCDIR   = -I/usr/local/include

#######################################################################
# AIX
#UOS      = -DAIX -D_BSD_INCLUDES

#CC   = xlC
#LD   = xlC
#COPTIONS = -DNO_NEW_CPP_FEATURES
#TARGETS = xfte

#######################################################################
# Irix
# missing fnmatch, but otherwise ok (tested only on 64bit)
# 6.x has fnmatch now ;-)
# uncomment below to use SGI CC compiler
#UOS  = -DIRIX
#CC   = CC
#LD   = CC
#COPTIONS = -DNO_NEW_CPP_FEATURES -OPT:Olimit=3000 # -xc++

#######################################################################
# SunOS (Solaris)
#UOS      = -DSUNOS
#CC = CC -noex
#LD = CC -noex
#COPTIONS = -DNO_NEW_CPP_FEATURES
#XINCDIR  = -I/usr/openwin/include
#XLIBDIR  = -L/usr/openwin/lib

#######################################################################
# for SCO CC
#
# !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
# If you have problems with the program "cfte"
# try to compile this program without optimalization -O2
# use just -O
#
#UOS = -DSCO
#CC  = CC  -b elf
#LD  = $(CC)
#XLIBDIR  = -L/usr/X11R6/lib
#SOCKETLIB = -lsocket
#COPTIONS = +.cpp

#######################################################################
# NCR
#CC = cc -Hnocopyr
#LD = cc -Hnocopyr
#COPTIONS = -w3
#UOS = -DNCR
#XINCDIR  = -I../../include
#SOCKETLIB = -lsocket -lnsl -lc -lucb

#######################################################################

#QTDIR   = /users/markom/qt
#QLIBDIR  = -L$(QTDIR)/lib
#QINCDIR  = -I$(QTDIR)/include
#QINCDIR  = -I/usr/include/qt

MOC      = moc

LIBDIR   =
INCDIR   =

ifneq "$(FTECONFIG)" ""
OPTIMIZE = -O2 -s
else
OPTIMIZE = -g # -O -g
#OPTIMIZE = -O2
#OPTIMIZE = -O2 -s
endif

CCFLAGS  = $(OPTIMIZE) $(I18NOPTIONS) $(APPOPTIONS) -DSYSTEM_FTERC='"$(SYSTEM_FTERC)"' $(COPTIONS) -DUNIX $(UOS) $(INCDIR) $(XINCDIR) $(QINCDIR) $(MINCDIR) $(SINCDIR)
LDFLAGS  = $(OPTIMIZE) $(LIBDIR) $(XLIBDIR) $(QLIBDIR) $(MLIBDIR)

OEXT     = o

.SUFFIXES: .cpp .o .moc

include objs.inc
SRCS = $(OBJS:.o=.cpp)

# Need -lXt below if USE_XTINIT is defined
XLIBS    = -lX11 $(SOCKETLIB)
#-lmpatrol -lelf
VLIBS    = -lgpm -lncurses
# -ltermcap outdated by ncurses
NLIBS    = -lncurses
SLIBS    = /usr/lib/libslang.a
QLIBS    = -lqt
#MLIBS    = -lXm -lXp -lXt -lXpm -lXext

.cpp.o:
	$(CC) $(CCFLAGS) -c $<

.c.o:
	$(CC) $(CCFLAGS) -c $<

native-%.o: %.c
	$(CCNATIVE) $(CCFLAGS) -c $< -o $@

native-%.o: %.cpp
	$(CCNATIVE) $(CCFLAGS) -c $< -o $@

.cpp.moc:
	$(MOC) $< -o $@

all:    $(CFTE_TARGET) $(TARGETS)
#rm -f fte ; ln -s $(PRIMARY) fte

cfte: cfte.o s_files.o s_string.o
	$(LD) $(LDFLAGS) $+ -o $@

native-cfte: $(patsubst %,native-%,cfte.o s_files.o s_string.o)
	$(LDNATIVE) $(LDFLAGS) $+ -o $@

c_config.o: defcfg.h

defcfg.h: defcfg.cnf
	perl mkdefcfg.pl <defcfg.cnf >defcfg.h

#DEFAULT_FTE_CONFIG = simple.fte
#DEFAULT_FTE_CONFIG = defcfg.fte
#DEFAULT_FTE_CONFIG = defcfg2.fte
#DEFAULT_FTE_CONFIG = ../config/main.fte

defcfg.cnf: $(DEFAULT_FTE_CONFIG) $(NATIVE_PREFIX)cfte
	./$(NATIVE_PREFIX)cfte $(DEFAULT_FTE_CONFIG) defcfg.cnf

xfte: .depend $(OBJS) $(XOBJS)
	$(LD) -o $@ $(LDFLAGS) $(OBJS) $(XOBJS) $(XLIBS)

#qfte: g_qt.moc g_qt_dlg.moc $(OBJS) $(QOBJS)
#	$(LD) -o $@ $(LDFLAGS) $(OBJS) $(QOBJS) $(QLIBS) $(XLIBS)

vfte: $(OBJS) $(VOBJS)
	$(LD) -o $@ $(LDFLAGS) $(OBJS) $(VOBJS) $(VLIBS)

sfte: $(OBJS) $(SOBJS) compkeys
	$(LD) -o $@ $(LDFLAGS) $(OBJS) $(SOBJS) $(SLIBS)

nfte: $(OBJS) $(NOBJS) compkeys
	$(LD) -o $@ $(LDFLAGS) $(OBJS) $(NOBJS) $(NLIBS)

compkeys: compkeys.o
	$(LD) $(LDFLAGS) compkeys.o -o compkeys

#mfte: $(OBJS) $(MOBJS)
#	$(LD) $(LDFLAGS) $(OBJS) $(MOBJS) $(MLIBS) $(XLIBS) -o mfte

g_qt.obj: g_qt.moc

g_qt_dlg.obj: g_qt_dlg.moc

.depend: defcfg.h
	$(CC) -MM $(CCFLAGS) $(SRCS) 1>.depend

# purposefully not part of "all".
tags: $(SRCS) $(wildcard *.h)
	ctags *.h $(SRCS)

clean:
	rm -f core *.o .depend $(TARGETS) defcfg.h defcfg.cnf $(NATIVE_PREFIX)cfte cfte fte vfte compkeys tags

#
# include dependency files if they exist
#
ifneq ($(wildcard .depend),)
include .depend
endif
