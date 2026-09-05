/*    sysdep.h
 *
 *    Copyright (c) 1994-1996, Marko Macek
 *
 *    You may distribute under the terms of either the GNU General Public
 *    License or the Artistic License, as specified in the README file.
 *
 */

#ifndef __SYSDEP_H
#define __SYSDEP_H

#ifndef _GNU_SOURCE
#define _GNU_SOURCE 1
#endif

#include <assert.h>
#include <errno.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#if defined(AIX) || defined(SCO) || defined(NCR)
#include <strings.h>
#endif
#include <stdio.h>
#include <fcntl.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>

#ifdef DBMALLOC
#include <malloc.h>
#endif

#define USE_DIRENT
#include <dirent.h>
#include <unistd.h>
#include <pwd.h>
#include <fnmatch.h>

#define strnicmp strncasecmp
#define stricmp strcasecmp
#define filecmp strcmp
extern "C" int memicmp(const void *s1, const void *s2, size_t n);

#ifndef MAXPATH
#    define MAXPATH 1024
#endif

#ifndef O_BINARY
#    define O_BINARY 0
#endif

#define _LNK_CONV
#define PT_UNIXISH   0
#define PT_DOSISH    1
#define PATHTYPE     PT_UNIXISH

#ifndef S_ISDIR
#    define S_ISDIR(mode)  (((mode) & S_IFMT) == S_IFDIR)
#endif

#ifndef S_IWGRP
#define S_IWGRP 0
#define S_IWOTH 0
#endif

#if defined __cplusplus && __cplusplus >= 199707L
#define HAVE_BOOL
#endif

#if defined __GNUC__ && (__GNUC__ > 2 || (__GNUC__ == 2 && __GNUC_MINOR__ >= 7))
#define HAVE_BOOL
#endif

#if defined _G_HAVE_BOOL
#define HAVE_BOOL
#endif

#if defined __BORLANDC__ && __BORLANDC__ >= 0x0500
#define HAVE_BOOL
#endif



#if defined(__GLIBC__) && defined(__GLIBC_PREREQ)
# if __GLIBC_PREREQ(2, 38)
#  define HAVE_STRLCPY 1
#  define HAVE_STRLCAT 1
# endif
#endif

#ifndef HAVE_BOOL
#define bool  int
#define true  1
#define false 0
#endif


#endif
