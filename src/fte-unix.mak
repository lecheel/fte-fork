# versions of FTE to build

TARGETS = nfte
PRIMARY = nfte
USE_LOCALE = -DUSE_LOCALE

I18NOPTIONS = $(USE_LOCALE)

APPOPTIONS = -DDEFAULT_INTERNAL_CONFIG

#gcc/g++
COPTIONS = -Wall -Wpointer-arith -Wconversion -Wwrite-strings \
           -Winline -Wno-narrowing

CC       = g++ -fno-rtti -fno-exceptions
LD       = g++ -fno-rtti -fno-exceptions
UOS      = -DLINUX

LIBDIR   =
INCDIR   =

OPTIMIZE = -g # -O -g

CCFLAGS  = $(OPTIMIZE) $(I18NOPTIONS) $(APPOPTIONS) $(COPTIONS) -DUNIX $(UOS) $(INCDIR)
LDFLAGS  = $(OPTIMIZE) $(LIBDIR)

OEXT     = o

.SUFFIXES: .cpp .o

include objs.inc
SRCS = $(OBJS:.o=.cpp) $(NOBJS:.o=.cpp) $(CFTE_OBJS:.o=.cpp)

OBJS := $(addprefix obj/,$(OBJS))
NOBJS := $(addprefix obj/,$(NOBJS))
CFTE_OBJS := $(addprefix obj/,$(CFTE_OBJS))

NLIBS    = -lncurses

.PHONY: all cfte nfte clean

all:    cfte $(TARGETS)

obj:
	@mkdir -p obj

obj/%.o: %.cpp | obj
	$(CC) $(CCFLAGS) -c $< -o $@

obj/%.o: %.c | obj
	$(CC) $(CCFLAGS) -c $< -o $@

cfte: $(CFTE_OBJS)
	$(LD) $(LDFLAGS) $(CFTE_OBJS) -o cfte

obj/c_config.o: defcfg.h

defcfg.h: defcfg.cnf
	perl mkdefcfg.pl <defcfg.cnf >defcfg.h

DEFAULT_FTE_CONFIG = defcfg.fte

defcfg.cnf: $(DEFAULT_FTE_CONFIG) cfte
	./cfte $(DEFAULT_FTE_CONFIG) defcfg.cnf

nfte: $(OBJS) $(NOBJS)
	$(LD) -o $@ $(LDFLAGS) $(OBJS) $(NOBJS) $(NLIBS)

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
