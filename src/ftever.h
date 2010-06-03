#ifndef __FTEVER_H
#define __FTEVER_H

#define MAKE_VERSION(major,minor,release) ((major<<24L) | (minor << 16L) | release)

#define PROG_FTE      "fte"
#define PROG_CFTE     "cfte"
#define PROGRAM       PROG_FTE
#define EXTRA_VERSION "(lechee)"
#define VERSION       "0.49.14" EXTRA_VERSION
#define VERNUM        MAKE_VERSION(0x00, 0x49, 0x0402)
#define COPYRIGHT     "Copyright(c) 1994-1998-2005 Marko Macek & Others"
#endif
